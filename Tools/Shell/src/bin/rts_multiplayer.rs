//! Packaged multiplayer acceptance: two-process listen (MP-1), dedicated 1v1 (MP-3,
//! `--dedicated`) and dedicated 3-4 client layouts (MP-2, `--dedicated-layouts`).
//!
//! cargo run --locked --manifest-path Tools/Shell/Cargo.toml --bin rts_multiplayer -- \
//!     ARCHIVE/Linux/Politika/Binaries/Linux/Politika [--layout 1v1] [--evidence DIR] [--port N]
//!
//! Launches an independent listen-server host and an independent client from the same
//! packaged Linux Development build, each on its own Xvfb display, user directory and log,
//! connected over 127.0.0.1. Real X11 mouse/keyboard input drives both processes. Unreal
//! remains authoritative; this harness only drives input, records screenshots/MP4 and
//! correlates the server's structured events with each client's observations.
use std::{
    env, fs,
    path::{Path, PathBuf},
    process::{Child, Command, Stdio},
    thread,
    time::{Duration, Instant, SystemTime, UNIX_EPOCH},
};

const WIDTH: u32 = 1280;
const HEIGHT: u32 = 720;
/// Order hotkeys from Config/DefaultInput.ini; W/A/S/D are camera-only.
const STOP_KEY: &str = "x";
const ATTACK_MOVE_KEY: &str = "q";

fn run_output(command: &mut Command) -> Result<String, String> {
    let result = command
        .output()
        .map_err(|e| format!("{}: {e}", command.get_program().to_string_lossy()))?;
    if !result.status.success() {
        return Err(format!(
            "{} exited {}: {}",
            command.get_program().to_string_lossy(),
            result.status,
            String::from_utf8_lossy(&result.stderr).trim()
        ));
    }
    Ok(String::from_utf8_lossy(&result.stdout).trim().to_owned())
}

fn json_escape(value: &str) -> String {
    let mut out = String::with_capacity(value.len() + 2);
    for c in value.chars() {
        match c {
            '"' => out.push_str("\\\""),
            '\\' => out.push_str("\\\\"),
            '\n' => out.push_str("\\n"),
            '\r' => out.push_str("\\r"),
            '\t' => out.push_str("\\t"),
            c if (c as u32) < 0x20 => out.push_str(&format!("\\u{:04x}", c as u32)),
            c => out.push(c),
        }
    }
    out
}

fn sha256(path: &Path) -> Result<String, String> {
    let digest = run_output(Command::new("sha256sum").arg(path))?;
    digest
        .split_whitespace()
        .next()
        .map(str::to_owned)
        .ok_or_else(|| format!("sha256sum produced no digest for {}", path.display()))
}

/// Value of ` key=value` in a structured log line.
fn field<'a>(line: &'a str, key: &str) -> Option<&'a str> {
    let pattern = format!(" {key}=");
    let start = line.find(&pattern)? + pattern.len();
    line[start..].split_whitespace().next()
}

fn number(line: &str, key: &str) -> Option<f64> {
    field(line, key)?.parse().ok()
}

/// An isolated X server for one game process.
struct Display {
    number: u32,
    child: Child,
}

impl Display {
    fn start(evidence: &Path, label: &str) -> Result<Self, String> {
        for number in 90..160 {
            if Path::new(&format!("/tmp/.X11-unix/X{number}")).exists()
                || Path::new(&format!("/tmp/.X{number}-lock")).exists()
            {
                continue;
            }
            let log = fs::File::create(evidence.join(format!("xvfb-{label}.log")))
                .map_err(|e| e.to_string())?;
            let child = Command::new("Xvfb")
                .arg(format!(":{number}"))
                .args([
                    "-screen",
                    "0",
                    &format!("{WIDTH}x{HEIGHT}x24"),
                    "-nolisten",
                    "tcp",
                ])
                .stdin(Stdio::null())
                .stdout(Stdio::null())
                .stderr(Stdio::from(log))
                .spawn()
                .map_err(|e| format!("start Xvfb (install xorg-server-xvfb): {e}"))?;
            let mut display = Display { number, child };
            let start = Instant::now();
            while start.elapsed() < Duration::from_secs(10) {
                if Path::new(&format!("/tmp/.X11-unix/X{number}")).exists() {
                    return Ok(display);
                }
                if let Ok(Some(_)) = display.child.try_wait() {
                    break;
                }
                thread::sleep(Duration::from_millis(100));
            }
            let _ = display.child.kill();
        }
        Err("BLOCKED: no free Xvfb display could be started".into())
    }

    fn name(&self) -> String {
        format!(":{}", self.number)
    }
}

impl Drop for Display {
    fn drop(&mut self) {
        // SIGTERM lets Xvfb remove its /tmp/.X<n>-lock and socket; SIGKILL leaks both and
        // eventually exhausts the display range for later runs.
        let _ = Command::new("kill")
            .args(["-TERM", &self.child.id().to_string()])
            .status();
        let start = Instant::now();
        while start.elapsed() < Duration::from_secs(5) {
            if let Ok(Some(_)) = self.child.try_wait() {
                return;
            }
            thread::sleep(Duration::from_millis(50));
        }
        let _ = self.child.kill();
        let _ = self.child.wait();
        let _ = fs::remove_file(format!("/tmp/.X{}-lock", self.number));
        let _ = fs::remove_file(format!("/tmp/.X11-unix/X{}", self.number));
    }
}

fn xdo(display: &str, args: &[&str]) -> Result<String, String> {
    run_output(Command::new("xdotool").env("DISPLAY", display).args(args))
}

/// One packaged game process with its own user directory and absolute log.
struct Game {
    label: String,
    child: Child,
    log: PathBuf,
    display: Option<String>,
    window: Option<String>,
    command: Vec<String>,
}

impl Game {
    fn launch(
        binary: &Path,
        label: &str,
        evidence: &Path,
        args: &[String],
        display: Option<&str>,
    ) -> Result<Self, String> {
        let user_dir = evidence.join(format!("user-{label}"));
        fs::create_dir_all(&user_dir).map_err(|e| e.to_string())?;
        let log = evidence.join(format!("{label}.log"));
        let mut all: Vec<String> = args.to_vec();
        all.push(format!("-UserDir={}/", user_dir.display()));
        all.push(format!("-ABSLOG={}", log.display()));
        all.push("-unattended".into());
        all.push("-nosplash".into());
        let mut command = Command::new(binary);
        command
            .args(&all)
            .stdin(Stdio::null())
            .stdout(Stdio::null())
            .stderr(Stdio::null());
        // Never inherit the operator's desktop session: SDL prefers Wayland when it is
        // available, which would open the game on the real desktop instead of Xvfb.
        command.env_remove("WAYLAND_DISPLAY");
        match display {
            Some(name) => {
                command.env("DISPLAY", name).env("SDL_VIDEODRIVER", "x11");
            }
            None => {
                command
                    .env_remove("DISPLAY")
                    .env("SDL_VIDEODRIVER", "offscreen");
            }
        }
        let child = command
            .spawn()
            .map_err(|e| format!("launch {label}: {e}"))?;
        let mut recorded = vec![binary.display().to_string()];
        recorded.extend(all);
        Ok(Game {
            label: label.into(),
            child,
            log,
            display: display.map(str::to_owned),
            window: None,
            command: recorded,
        })
    }

    fn pid(&self) -> u32 {
        self.child.id()
    }

    fn text(&self) -> String {
        fs::read(&self.log)
            .map(|b| String::from_utf8_lossy(&b).into_owned())
            .unwrap_or_default()
    }

    fn offset(&self) -> usize {
        fs::metadata(&self.log)
            .map(|m| m.len() as usize)
            .unwrap_or(0)
    }

    /// Waits for a log line containing every fragment, after `offset` bytes.
    fn wait_line(
        &mut self,
        fragments: &[&str],
        timeout: Duration,
        offset: usize,
    ) -> Result<String, String> {
        let start = Instant::now();
        loop {
            let text = self.text();
            // Only complete lines: a line still being written can match its fragments before
            // its remaining fields are flushed.
            let complete = text.rfind('\n').map_or("", |end| &text[..=end]);
            if let Some(tail) = complete.get(offset..) {
                if let Some(line) = tail
                    .lines()
                    .find(|line| fragments.iter().all(|f| line.contains(f)))
                {
                    return Ok(line.to_owned());
                }
            }
            if let Some(status) = self.child.try_wait().map_err(|e| e.to_string())? {
                return Err(format!(
                    "{} exited {status} before {:?}; log={}",
                    self.label,
                    fragments,
                    self.log.display()
                ));
            }
            if start.elapsed() >= timeout {
                return Err(format!(
                    "timeout waiting {timeout:?} for {:?} in {}; not a pass",
                    fragments,
                    self.log.display()
                ));
            }
            thread::sleep(Duration::from_millis(100));
        }
    }

    fn wait(&mut self, fragments: &[&str], timeout: Duration) -> Result<String, String> {
        self.wait_line(fragments, timeout, 0)
    }

    fn attach_window(&mut self) -> Result<String, String> {
        let display = self
            .display
            .clone()
            .ok_or("headless process has no window")?;
        for _ in 0..150 {
            if let Ok(found) = xdo(&display, &["search", "--onlyvisible", "--name", "Politika"]) {
                if let Some(id) = found.lines().next() {
                    let geometry = xdo(&display, &["getwindowgeometry", "--shell", id])?;
                    if !geometry.contains(&format!("WIDTH={WIDTH}"))
                        || !geometry.contains(&format!("HEIGHT={HEIGHT}"))
                    {
                        return Err(format!(
                            "{}: unexpected window geometry {geometry}",
                            self.label
                        ));
                    }
                    self.window = Some(id.to_owned());
                    return Ok(geometry);
                }
            }
            thread::sleep(Duration::from_millis(100));
        }
        Err(format!("{} produced no visible window", self.label))
    }

    fn click(&self, x: f64, y: f64, button: &str) -> Result<(), String> {
        let display = self.display.as_deref().ok_or("no display")?;
        let window = self.window.as_deref().ok_or("no window")?;
        let (x, y) = (
            x.clamp(4.0, f64::from(WIDTH) - 4.0).round(),
            y.clamp(4.0, f64::from(HEIGHT) - 4.0).round(),
        );
        xdo(
            display,
            &[
                "mousemove",
                "--window",
                window,
                "--sync",
                &format!("{x}"),
                &format!("{y}"),
            ],
        )?;
        thread::sleep(Duration::from_millis(120));
        xdo(display, &["click", button]).map(|_| ())
    }

    fn hover(&self, x: f64, y: f64) -> Result<(), String> {
        let display = self.display.as_deref().ok_or("no display")?;
        let window = self.window.as_deref().ok_or("no window")?;
        xdo(
            display,
            &[
                "mousemove",
                "--window",
                window,
                "--sync",
                &format!("{}", x.round()),
                &format!("{}", y.round()),
            ],
        )
        .map(|_| ())
    }

    fn key(&self, key: &str) -> Result<(), String> {
        let display = self.display.as_deref().ok_or("no display")?;
        xdo(display, &["key", key]).map(|_| ())
    }

    fn screenshot(&self, evidence: &Path, name: &str) -> Result<PathBuf, String> {
        let display = self.display.as_deref().ok_or("no display")?;
        let window = self.window.as_deref().ok_or("no window")?;
        let path = evidence.join(format!("{}-{name}.png", self.label));
        run_output(
            Command::new("import")
                .env("DISPLAY", display)
                .args(["-window", window])
                .arg(&path),
        )
        .map_err(|e| format!("screenshot {name}: {e}"))?;
        let size = fs::metadata(&path).map_err(|e| e.to_string())?.len();
        if size < 4096 {
            return Err(format!(
                "screenshot {} is unexpectedly small ({size} bytes)",
                path.display()
            ));
        }
        Ok(path)
    }

    fn kill_hard(&mut self) {
        let _ = Command::new("kill")
            .args(["-KILL", &self.child.id().to_string()])
            .status();
        let _ = self.child.wait();
    }

    fn alive(&mut self) -> bool {
        matches!(self.child.try_wait(), Ok(None))
    }

    fn stop(&mut self) {
        if self.alive() {
            let _ = Command::new("kill")
                .args(["-TERM", &self.child.id().to_string()])
                .status();
            let start = Instant::now();
            while self.alive() && start.elapsed() < Duration::from_secs(15) {
                thread::sleep(Duration::from_millis(200));
            }
            if self.alive() {
                self.kill_hard();
            }
        }
    }
}

impl Drop for Game {
    fn drop(&mut self) {
        if self.alive() {
            self.kill_hard();
        }
    }
}

struct Recorder {
    child: Child,
    path: PathBuf,
}

impl Recorder {
    fn start(display: &str, evidence: &Path, label: &str) -> Result<Self, String> {
        let path = evidence.join(format!("{label}.mp4"));
        let log = fs::File::create(evidence.join(format!("{label}-recorder.log")))
            .map_err(|e| e.to_string())?;
        let child = Command::new("ffmpeg")
            .args([
                "-hide_banner",
                "-loglevel",
                "warning",
                "-y",
                "-f",
                "x11grab",
                "-framerate",
                "25",
                "-video_size",
            ])
            .arg(format!("{WIDTH}x{HEIGHT}"))
            .args(["-i", &format!("{display}+0,0")])
            .args([
                "-an",
                "-c:v",
                "libx264",
                "-preset",
                "veryfast",
                "-crf",
                "24",
                "-pix_fmt",
                "yuv420p",
                "-movflags",
                "+faststart",
            ])
            .arg(&path)
            .stdin(Stdio::null())
            .stdout(Stdio::null())
            .stderr(Stdio::from(log))
            .spawn()
            .map_err(|e| format!("start ffmpeg x11grab recording: {e}"))?;
        Ok(Recorder { child, path })
    }

    fn finish(mut self) -> Result<PathBuf, String> {
        let _ = Command::new("kill")
            .args(["-INT", &self.child.id().to_string()])
            .status();
        let status = self.child.wait().map_err(|e| e.to_string())?;
        if !status.success() && status.code() != Some(255) {
            return Err(format!("ffmpeg recorder exited {status}"));
        }
        let probe = run_output(
            Command::new("ffprobe")
                .args([
                    "-v",
                    "error",
                    "-select_streams",
                    "v:0",
                    "-show_entries",
                    "stream=codec_name,width,height",
                    "-of",
                    "csv=p=0",
                ])
                .arg(&self.path),
        )?;
        if probe != format!("h264,{WIDTH},{HEIGHT}") {
            return Err(format!("recorded MP4 failed stream validation ({probe})"));
        }
        Ok(self.path.clone())
    }
}

/// Collected verdicts using only the acceptance vocabulary.
#[derive(Default)]
struct Report {
    schema: Option<&'static str>,
    tests: Vec<(String, &'static str, String)>,
    artifacts: Vec<(String, String)>,
    facts: Vec<(String, String)>,
}

impl Report {
    fn verdict(&mut self, id: &str, pass: bool, detail: impl Into<String>) {
        let detail = detail.into();
        println!("{} {id}: {detail}", if pass { "PASS" } else { "FAIL" });
        self.tests
            .push((id.into(), if pass { "PASS" } else { "FAIL" }, detail));
    }
    fn mark(&mut self, id: &str, verdict: &'static str, detail: impl Into<String>) {
        let detail = detail.into();
        println!("{verdict} {id}: {detail}");
        self.tests.push((id.into(), verdict, detail));
    }
    fn artifact(&mut self, name: &str, path: &Path) {
        self.artifacts
            .push((name.into(), path.display().to_string()));
    }
    fn fact(&mut self, name: &str, value: impl Into<String>) {
        self.facts.push((name.into(), value.into()));
    }
    fn failed(&self) -> bool {
        self.tests.iter().any(|(_, verdict, _)| *verdict != "PASS")
    }
    fn write(&self, path: &Path) -> Result<(), String> {
        let mut json = format!(
            "{{\n  \"schema\": \"{}\",\n  \"facts\": {{\n",
            self.schema.unwrap_or("politika.mp1.packaged-evidence.v1")
        );
        for (index, (key, value)) in self.facts.iter().enumerate() {
            json.push_str(&format!(
                "    \"{}\": \"{}\"{}\n",
                json_escape(key),
                json_escape(value),
                if index + 1 < self.facts.len() {
                    ","
                } else {
                    ""
                }
            ));
        }
        json.push_str("  },\n  \"tests\": [\n");
        for (index, (id, verdict, detail)) in self.tests.iter().enumerate() {
            json.push_str(&format!(
                "    {{\"id\": \"{}\", \"verdict\": \"{verdict}\", \"detail\": \"{}\"}}{}\n",
                json_escape(id),
                json_escape(detail),
                if index + 1 < self.tests.len() {
                    ","
                } else {
                    ""
                }
            ));
        }
        json.push_str("  ],\n  \"artifacts\": {\n");
        for (index, (key, value)) in self.artifacts.iter().enumerate() {
            json.push_str(&format!(
                "    \"{}\": \"{}\"{}\n",
                json_escape(key),
                json_escape(value),
                if index + 1 < self.artifacts.len() {
                    ","
                } else {
                    ""
                }
            ));
        }
        json.push_str("  }\n}\n");
        fs::write(path, json).map_err(|e| e.to_string())
    }
}

/// Latest screen position the given process logged for an entity.
fn entity_screen(text: &str, marker: &str, entity: u64) -> Option<(f64, f64)> {
    text.lines()
        .rev()
        .filter(|line| line.contains(marker))
        .find(|line| field(line, "entity").and_then(|v| v.parse::<u64>().ok()) == Some(entity))
        .and_then(|line| Some((number(line, "screen_x")?, number(line, "screen_y")?)))
        .filter(|(x, y)| {
            *x > 8.0 && *y > 8.0 && *x < f64::from(WIDTH) - 8.0 && *y < f64::from(HEIGHT) - 8.0
        })
}

fn objective_screen(text: &str, marker: &str) -> Option<(f64, f64)> {
    text.lines()
        .rev()
        .find(|line| line.contains(marker))
        .and_then(|line| Some((number(line, "screen_x")?, number(line, "screen_y")?)))
        .filter(|(x, y)| {
            *x > 8.0 && *y > 8.0 && *x < f64::from(WIDTH) - 8.0 && *y < f64::from(HEIGHT) - 8.0
        })
}

/// Left-clicks an entity at its latest logged screen position until the process logs its
/// selection; a zoomed-out unit is small, so nearby points on its body are also tried.
fn select_entity(game: &mut Game, marker: &str, entity: u64) -> Result<(), String> {
    for (dx, dy) in [
        (0.0, 0.0),
        (0.0, -8.0),
        (0.0, -16.0),
        (4.0, -4.0),
        (-4.0, -4.0),
        (0.0, 6.0),
    ] {
        let (x, y) = wait_screen(
            game,
            |t| entity_screen(t, marker, entity),
            &format!("entity {entity}"),
        )?;
        let mark = game.offset();
        game.click(x + dx, y + dy, "1")?;
        if game
            .wait_line(
                &["RTS_SELECTION player=", &format!("unit={entity}")],
                Duration::from_secs(2),
                mark,
            )
            .is_ok()
        {
            return Ok(());
        }
    }
    Err(format!(
        "{}: could not select entity {entity} with real input",
        game.label
    ))
}

fn zoom_out(game: &Game, notches: u32) -> Result<(), String> {
    game.hover(640.0, 360.0)?;
    let display = game.display.as_deref().ok_or("no display")?;
    for _ in 0..notches {
        xdo(display, &["click", "5"])?;
        thread::sleep(Duration::from_millis(80));
    }
    Ok(())
}

fn wait_screen<F: Fn(&str) -> Option<(f64, f64)>>(
    game: &mut Game,
    read: F,
    what: &str,
) -> Result<(f64, f64), String> {
    let start = Instant::now();
    while start.elapsed() < Duration::from_secs(12) {
        if let Some(found) = read(&game.text()) {
            return Ok(found);
        }
        if !game.alive() {
            break;
        }
        thread::sleep(Duration::from_millis(200));
    }
    Err(format!("{}: no on-screen position for {what}", game.label))
}

/// Owner of each authoritative entity, from the host's spawn records.
fn entity_owners(host_text: &str) -> Vec<(u64, u32)> {
    let mut owners: Vec<(u64, u32)> = host_text
        .lines()
        .filter(|line| line.contains("MP_SERVER_SAMPLE reason=activated"))
        .filter_map(|line| {
            Some((
                field(line, "entity")?.parse().ok()?,
                field(line, "owner")?.parse().ok()?,
            ))
        })
        .collect();
    owners.sort_unstable();
    owners.dedup();
    owners
}

struct Options {
    binary: PathBuf,
    layout: String,
    evidence: PathBuf,
    port: u16,
}

fn parse_options() -> Result<Options, String> {
    let mut args = env::args().skip(1);
    let binary = PathBuf::from(args.next().ok_or(
        "usage: rts_multiplayer PACKAGED_BINARY [--layout 1v1] [--evidence DIR] [--port N]",
    )?)
    .canonicalize()
    .map_err(|e| format!("packaged binary: {e}"))?;
    let mut layout = "1v1".to_owned();
    let mut evidence = None;
    let mut port = 17777u16;
    while let Some(flag) = args.next() {
        let value = args.next().ok_or_else(|| format!("{flag} needs a value"))?;
        match flag.as_str() {
            "--layout" => layout = value,
            "--evidence" => evidence = Some(PathBuf::from(value)),
            "--port" => port = value.parse().map_err(|e| format!("--port: {e}"))?,
            other => return Err(format!("unknown option {other}")),
        }
    }
    if layout != "1v1" {
        return Err("BLOCKED: the two-process harness qualifies the 1v1 layout; 2v1/3v1/2v2 need 3-4 processes (MP-2)".into());
    }
    let epoch = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .map_err(|e| e.to_string())?
        .as_secs();
    let evidence = evidence.unwrap_or_else(|| {
        PathBuf::from(format!(
            "build/diagnostics/mp1-listen-server/packaged-{epoch}"
        ))
    });
    Ok(Options {
        binary,
        layout,
        evidence,
        port,
    })
}

fn host_args(options: &Options, rematch: bool) -> Vec<String> {
    let url = format!(
        "/Game/Maps/RTSVerticalSlice?listen?PolitikaLayout={}",
        options.layout
    );
    let mut args = vec![
        url.clone(),
        format!("-port={}", options.port),
        "-PolitikaTestFixture=MultiplayerSkirmish".into(),
        "-RTSAcceptance=multiplayer".into(),
        "-windowed".into(),
        format!("-ResX={WIDTH}"),
        format!("-ResY={HEIGHT}"),
    ];
    args.extend(pinned_forces());
    if rematch {
        args.push(format!("-PolitikaMPRematch={url}"));
        args.push("-PolitikaMPRematchDelay=3".into());
        args.push("-PolitikaMPRematchAttempts=1".into());
    }
    args
}

fn client_args(options: &Options, rendered: bool, probe: bool, rematch: bool) -> Vec<String> {
    let address = format!("127.0.0.1:{}", options.port);
    let mut args = vec![address.clone()];
    if rendered {
        args.extend([
            "-windowed".into(),
            format!("-ResX={WIDTH}"),
            format!("-ResY={HEIGHT}"),
        ]);
    } else {
        args.extend(["-nullrhi".into(), "-nosound".into()]);
    }
    if probe {
        args.push("-PolitikaMPProbe".into());
    }
    if rematch {
        args.push(format!("-PolitikaMPRematch={address}"));
        args.push("-PolitikaMPRematchDelay=10".into());
        args.push("-PolitikaMPRematchAttempts=2".into());
    }
    args
}

/// Development-only multiplayer switches must be recognized by the bootstrap launch-option
/// parser: none may surface as an InvalidLaunchOptions MainMenu recovery issue.
fn launch_options_verdict(report: &mut Report, id: &str, games: &[&Game]) {
    let mut detail = Vec::new();
    // Every session passes at least the test fixture switch; none means nothing was checked.
    let mut pass = games
        .iter()
        .any(|g| g.command.iter().any(|a| a.starts_with("-Politika")));
    for game in games {
        let text = game.text();
        let booted = text.contains("\"event\":\"bootstrap\"");
        let flagged = text.contains("\"code\":\"InvalidLaunchOptions\"");
        let switches: Vec<&str> = game
            .command
            .iter()
            .map(String::as_str)
            .filter(|a| a.starts_with("-Politika"))
            .collect();
        pass &= booted && !flagged;
        detail.push(format!(
            "{}: bootstrap={booted} InvalidLaunchOptions={flagged} switches={switches:?}",
            game.label
        ));
    }
    report.verdict(id, pass, detail.join("; "));
}

/// Expected server verdict for each hostile probe the client sends (rules::OrderRejection).
fn expected_probe_rejection(name: &str) -> Option<&'static [u32]> {
    Some(match name {
        "foreign_issuer" => &[1],
        "hidden_target" => &[18],
        "system_issuer" => &[1],
        "unknown_issuer" => &[9],
        "invalid_kind" | "nonfinite_destination" | "shape_mismatch" | "protocol_mismatch" => &[15],
        "out_of_bounds" => &[5],
        "stale_generation" => &[13],
        "friendly_target" | "allied_target" => &[8],
        "allied_issuer" => &[1],
        "duplicate_replay" | "reordered_older" => &[16],
        "after_termination" => &[12],
        name if name.starts_with("hidden_issuer_") => &[1, 9],
        _ => return None,
    })
}

fn check_probes(report: &mut Report, id: &str, client: &Game, host: &Game) {
    check_probes_for(report, id, client, host, &[1, 2]);
}

/// Probe verdicts for one probing client; `remote` lists every participant a remote
/// command may legitimately be attributed to on the server.
fn check_probes_for(report: &mut Report, id: &str, client: &Game, host: &Game, remote: &[u32]) {
    let client_text = client.text();
    let host_text = host.text();
    let mut checked = 0;
    let mut problems = Vec::new();
    for line in client_text
        .lines()
        .filter(|line| line.contains("MP_PROBE_SENT"))
    {
        let (Some(name), Some(correlation)) = (field(line, "name"), field(line, "correlation"))
        else {
            continue;
        };
        let outcome = client_text.lines().find(|l| {
            l.contains("MP_COMMAND_OUTCOME") && field(l, "correlation") == Some(correlation)
        });
        let Some(outcome) = outcome else {
            problems.push(format!("{name}: no correlated outcome"));
            continue;
        };
        let accepted = field(outcome, "accepted") == Some("1");
        let rejection: u32 = field(outcome, "rejection")
            .and_then(|v| v.parse().ok())
            .unwrap_or(999);
        checked += 1;
        if name == "valid_once" {
            if !accepted {
                problems.push(format!("valid_once rejected ({rejection})"));
            }
            continue;
        }
        match expected_probe_rejection(name) {
            Some(expected) if !accepted && expected.contains(&rejection) => {}
            Some(expected) => problems.push(format!(
                "{name}: accepted={accepted} rejection={rejection} expected {expected:?}"
            )),
            None => problems.push(format!("{name}: unclassified probe")),
        }
    }
    // The server attributes every remote command to a connection-bound participant, never
    // to another (in the two-process listen run the host itself is participant 1).
    let server_attributed = host_text
        .lines()
        .filter(|l| {
            (l.contains("MP_COMMAND_RECEIVED") || l.contains("MP_COMMAND_REJECTED_AT_INTAKE"))
                && field(l, "remote") != Some("0")
        })
        .all(|l| {
            field(l, "participant")
                .and_then(|v| v.parse::<u32>().ok())
                .is_some_and(|p| remote.contains(&p))
        });
    if !server_attributed {
        problems.push(format!(
            "server attributed a remote command to a participant outside {remote:?}"
        ));
    }
    report.verdict(id, checked >= 14 && problems.is_empty(),
        format!("{checked} correlated hostile/edge probes over the real ServerSubmitCommandIntent RPC; problems={problems:?}"));
}

fn health_consistency(report: &mut Report, id: &str, host: &Game, client: &Game) {
    // Every health value a client observed was produced by server damage application, and
    // every unit the client saw destroyed was destroyed by the server.
    let host_text = host.text();
    let client_text = client.text();
    let mut problems = Vec::new();
    let mut observed = 0;
    for line in client_text
        .lines()
        .filter(|l| l.contains("MP_CLIENT_UNIT_HEALTH"))
    {
        let (Some(entity), Some(health)) = (field(line, "entity"), field(line, "health")) else {
            continue;
        };
        if field(line, "max").is_some_and(|max| max == health) {
            continue;
        }
        observed += 1;
        let server = host_text.lines().any(|l| {
            (l.contains("RTS_HEALTH_CHANGED")
                && field(l, "unit") == Some(entity)
                && field(l, "after") == Some(health))
                || (l.contains("MP_SERVER_SAMPLE")
                    && field(l, "entity") == Some(entity)
                    && field(l, "health") == Some(health))
        });
        if !server {
            problems.push(format!(
                "client saw entity {entity} at health {health} with no server record"
            ));
        }
    }
    let mut deaths = 0;
    for line in client_text
        .lines()
        .filter(|l| l.contains("MP_CLIENT_UNIT_DESTROYED"))
    {
        let Some(entity) = field(line, "entity") else {
            continue;
        };
        deaths += 1;
        if !host_text
            .lines()
            .any(|l| l.contains("RTS_UNIT_DESTROYED") && field(l, "unit") == Some(entity))
        {
            problems.push(format!(
                "client saw entity {entity} destroyed; server did not"
            ));
        }
    }
    report.verdict(id, observed > 0 && problems.is_empty(),
        format!("client observed {observed} damaged-health states and {deaths} deaths, all server-originated; problems={problems:?}"));
}

fn session_gameplay(options: &Options, report: &mut Report) -> Result<(), String> {
    let evidence = &options.evidence;
    let host_display = Display::start(evidence, "host")?;
    let client_display = Display::start(evidence, "client")?;
    report.fact("host_display", host_display.name());
    report.fact("client_display", client_display.name());
    let mut host = Game::launch(
        &options.binary,
        "host",
        evidence,
        &host_args(options, true),
        Some(&host_display.name()),
    )?;
    report.fact("host_pid", host.pid().to_string());
    report.fact("host_command", host.command.join(" "));
    let ready = host.wait(&["MP_LISTEN_SERVER_READY"], Duration::from_secs(90))?;
    let awaiting = host.wait(&["MP_MATCH_AWAITING_PARTICIPANTS"], Duration::from_secs(30))?;
    let generation1 = field(&awaiting, "generation").unwrap_or("0").to_owned();
    report.verdict(
        "MP1-F-LISTEN",
        field(&ready, "port") == Some(&options.port.to_string())
            && field(&ready, "net_driver") == Some("1"),
        format!("host listening: {}", ready.trim()),
    );
    let layout = host.wait(&["MP_SESSION_LAYOUT"], Duration::from_secs(5))?;
    report.verdict(
        "MP1-LAYOUT-1V1-UNALIGNED",
        field(&layout, "layout") == Some("1v1")
            && field(&layout, "seats") == Some("2")
            && field(&layout, "fog") == Some("1")
            && host.text().contains("MATCH_UNALIGNED_SIDE units=2"),
        "1v1 session = two player seats + the Unaligned NPC side (2 Unaligned units)",
    );

    let mut client = Game::launch(
        &options.binary,
        "client",
        evidence,
        &client_args(options, true, true, true),
        Some(&client_display.name()),
    )?;
    report.fact("client_pid", client.pid().to_string());
    report.fact("client_command", client.command.join(" "));
    report.fact("address", format!("127.0.0.1:{}", options.port));
    let prelogin = host.wait(&["MP_PRELOGIN ACCEPTED"], Duration::from_secs(60))?;
    let bound = host.wait(
        &["MP_PARTICIPANT_BOUND participant=2", "remote=1"],
        Duration::from_secs(30),
    )?;
    let host_bound = host.wait(
        &["MP_PARTICIPANT_BOUND participant=1", "remote=0"],
        Duration::from_secs(5),
    )?;
    let active = host.wait(
        &["MP_MATCH_ACTIVE", &format!("generation={generation1}")],
        Duration::from_secs(10),
    )?;
    let assignment = client.wait(
        &["MP_CLIENT_ASSIGNMENT participant=2"],
        Duration::from_secs(30),
    )?;
    client.wait(
        &[
            "MP_CLIENT_MATCH_OBSERVED",
            "map=map.test.multiplayer_skirmish",
        ],
        Duration::from_secs(30),
    )?;
    report.verdict(
        "MP1-01",
        field(&host_bound, "team") == Some("1"),
        format!("host binding: {}", host_bound.trim()),
    );
    report.verdict(
        "MP1-02",
        field(&bound, "team") == Some("2")
            && field(&assignment, "generation") == Some(generation1.as_str()),
        format!(
            "remote binding: {} / client: {}",
            bound.trim(),
            assignment.trim()
        ),
    );
    report.verdict(
        "MP1-G4-CONNECTED",
        prelogin.contains(&format!("protocol={}", network_protocol()))
            && client.pid() != host.pid(),
        format!(
            "two independent packaged processes (pids {} / {}) connected; {}",
            host.pid(),
            client.pid(),
            active.trim()
        ),
    );

    let host_geometry = host.attach_window()?;
    let client_geometry = client.attach_window()?;
    fs::write(evidence.join("host-window.txt"), &host_geometry).map_err(|e| e.to_string())?;
    fs::write(evidence.join("client-window.txt"), &client_geometry).map_err(|e| e.to_string())?;
    let host_recorder = Recorder::start(&host_display.name(), evidence, "host")?;
    let client_recorder = Recorder::start(&client_display.name(), evidence, "client")?;
    let result = (|| -> Result<(), String> {
        thread::sleep(Duration::from_secs(2));
        report.artifact("host_active_png", &host.screenshot(evidence, "01-active")?);
        report.artifact(
            "client_active_png",
            &client.screenshot(evidence, "01-active")?,
        );

        // Fog: at start the client receives only its own units; the host renders only its team's view.
        let owners = entity_owners(&host.text());
        let client_text = client.text();
        let relevant: Vec<(u64, u32)> = client_text
            .lines()
            .filter(|l| l.contains("MP_CLIENT_UNIT_RELEVANT"))
            .filter_map(|l| {
                Some((
                    field(l, "entity")?.parse().ok()?,
                    field(l, "owner")?.parse().ok()?,
                ))
            })
            .collect();
        let host_hidden_enemies = host
            .text()
            .lines()
            .filter(|l| l.contains("MP_SERVER_SAMPLE reason=activated"))
            .filter(|l| field(l, "owner") != Some("1"))
            .all(|l| field(l, "host_hidden") == Some("1"));
        report.verdict("MP1-FOG-INITIAL", !relevant.is_empty() && relevant.iter().all(|(_, owner)| *owner == 2)
            && owners.iter().filter(|(_, owner)| *owner == 2).count() == relevant.len() && host_hidden_enemies,
            format!("authoritative entities {owners:?}; client received only {relevant:?}; host hides unobserved enemy/Unaligned units={host_hidden_enemies}"));

        // Hostile probe (client) runs 3 s after activation over the real RPC.
        client.wait(&["MP_PROBE_COMPLETE"], Duration::from_secs(20))?;
        thread::sleep(Duration::from_secs(2));

        // Remote client real input: select, Sprint (ability), Stop, Guard, Move toward the objective.
        let client_units: Vec<u64> = owners
            .iter()
            .filter(|(_, o)| *o == 2)
            .map(|(e, _)| *e)
            .collect();
        let host_units: Vec<u64> = owners
            .iter()
            .filter(|(_, o)| *o == 1)
            .map(|(e, _)| *e)
            .collect();
        let (client_unit, host_unit_a, host_unit_b) =
            match (client_units.first(), host_units.first(), host_units.get(1)) {
                (Some(c), Some(a), Some(b)) => (*c, *a, *b),
                _ => return Err(format!("unexpected authoritative roster {owners:?}")),
            };
        let since_host = host.offset();
        select_entity(&mut client, "MP_CLIENT_SAMPLE", client_unit)?;
        let mut client_orders = Vec::new();
        for (key, order) in [("f", 7u32), (STOP_KEY, 3), ("g", 4)] {
            let mark = client.offset();
            client.key(key)?;
            let outcome = client.wait_line(
                &[
                    "MP_COMMAND_OUTCOME",
                    &format!("order={order}"),
                    "accepted=1",
                    "authority=0",
                ],
                Duration::from_secs(5),
                mark,
            )?;
            client_orders.push(format!("{key}->{}", outcome.trim()));
            thread::sleep(Duration::from_millis(400));
        }
        // Zoom out so the client's start and the shared objective are both on screen.
        zoom_out(&client, 5)?;
        thread::sleep(Duration::from_millis(2500));
        let objective = wait_screen(
            &mut client,
            |t| objective_screen(t, "MP_CLIENT_OBJECTIVE"),
            "objective on client",
        )?;
        let unit_screen = wait_screen(
            &mut client,
            |t| entity_screen(t, "MP_CLIENT_SAMPLE", client_unit),
            "client unit",
        )?;
        // A ground point about 45% of the way from the objective to the unit: inside sight of
        // the objective, outside its capture radius.
        let target = (
            objective.0 + (unit_screen.0 - objective.0) * 0.45,
            objective.1 + (unit_screen.1 - objective.1) * 0.45,
        );
        let mark = client.offset();
        client.click(target.0, target.1, "3")?;
        let moved = client.wait_line(
            &["MP_COMMAND_OUTCOME", "order=0", "accepted=1"],
            Duration::from_secs(5),
            mark,
        )?;
        let admitted_remote = host.wait_line(
            &[
                "RTS_ORDER_ADMITTED order=Move",
                &format!("unit={client_unit}"),
                "participant=2",
            ],
            Duration::from_secs(5),
            since_host,
        )?;
        report.verdict("MP1-POS-REMOTE-ORDERS", client_orders.len() == 3,
            format!("remote real input Sprint/Stop/Guard/Move admitted by the server: {client_orders:?}; {}; {}", moved.trim(), admitted_remote.trim()));
        report.artifact(
            "client_orders_png",
            &client.screenshot(evidence, "02-orders")?,
        );

        // Host real input: unit B Stop and Guard at default zoom, unit A moves onto the
        // objective after zooming out, then unit B attack-moves to the objective.
        let since_host = host.offset();
        select_entity(&mut host, "MP_SERVER_SAMPLE", host_unit_b)?;
        for (key, order) in [(STOP_KEY, "Stop"), ("g", "Guard")] {
            let mark = host.offset();
            host.key(key)?;
            host.wait_line(
                &[
                    &format!("RTS_ORDER_ADMITTED order={order}"),
                    &format!("unit={host_unit_b}"),
                    "participant=1",
                ],
                Duration::from_secs(5),
                mark,
            )?;
        }
        select_entity(&mut host, "MP_SERVER_SAMPLE", host_unit_a)?;
        zoom_out(&host, 5)?;
        thread::sleep(Duration::from_millis(2500));
        let objective = wait_screen(
            &mut host,
            |t| objective_screen(t, "MP_SERVER_OBJECTIVE"),
            "objective on host",
        )?;
        host.click(objective.0, objective.1, "3")?;
        let host_move = host.wait_line(
            &[
                "RTS_ORDER_ADMITTED order=Move",
                &format!("unit={host_unit_a}"),
                "participant=1",
            ],
            Duration::from_secs(5),
            since_host,
        )?;
        select_entity(&mut host, "MP_SERVER_SAMPLE", host_unit_b)?;
        host.hover(objective.0 + 18.0, objective.1 + 10.0)?;
        thread::sleep(Duration::from_millis(150));
        let mark = host.offset();
        host.key(ATTACK_MOVE_KEY)?;
        let host_attack_move = host.wait_line(
            &[
                "RTS_ORDER_ADMITTED order=AttackMove",
                &format!("unit={host_unit_b}"),
                "participant=1",
            ],
            Duration::from_secs(5),
            mark,
        )?;
        let same_intake = host
            .text()
            .lines()
            .any(|l| l.contains("MP_COMMAND_RECEIVED participant=1 remote=0"));
        report.verdict(
            "MP1-POS-HOST-ORDERS",
            same_intake,
            format!(
                "host real input through the shared intake: {}; {}",
                host_move.trim(),
                host_attack_move.trim()
            ),
        );
        report.artifact("host_orders_png", &host.screenshot(evidence, "02-orders")?);

        // Fog reveal, combat and objective under server authority.
        let reveal = client.wait_line(
            &["MP_CLIENT_UNIT_RELEVANT", "owner=1"],
            Duration::from_secs(40),
            0,
        );
        report.verdict(
            "MP1-FOG-REVEAL",
            reveal.is_ok(),
            format!("host unit replicated to the client once observed: {reveal:?}"),
        );
        report.artifact(
            "client_contact_png",
            &client.screenshot(evidence, "03-contact")?,
        );
        let results = host.wait(
            &[
                "MATCH_RESULTS outcome=objective_secured",
                &format!("generation={generation1}"),
            ],
            Duration::from_secs(90),
        )?;
        let client_results = client.wait(
            &[
                "MP_CLIENT_LIFECYCLE",
                "result=1",
                "reason=objective_secured",
            ],
            Duration::from_secs(15),
        )?;
        thread::sleep(Duration::from_millis(1500));
        report.artifact(
            "host_results_png",
            &host.screenshot(evidence, "04-results")?,
        );
        report.artifact(
            "client_results_png",
            &client.screenshot(evidence, "04-results")?,
        );
        let commits = host
            .text()
            .lines()
            .filter(|l| {
                l.contains("MATCH_RESULTS") && l.contains(&format!("generation={generation1}"))
            })
            .count();
        report.verdict(
            "MP1-11",
            commits == 1
                && field(&results, "team") == Some("1")
                && field(&client_results, "team") == Some("1")
                && field(&client_results, "viewer_team") == Some("2"),
            format!(
                "server committed exactly {commits} result; {}; client observed {}",
                results.trim(),
                client_results.trim()
            ),
        );
        health_consistency(report, "MP1-POS-DAMAGE-CONSISTENCY", &host, &client);
        client.wait(
            &["MP_PROBE_SENT name=after_termination"],
            Duration::from_secs(10),
        )?;
        thread::sleep(Duration::from_secs(1));
        check_probes(report, "MP1-NEG-PROBES", &client, &host);

        // Normal completion: host returns to MainMenu; the client recovers with the result reason.
        let host_mark = host.offset();
        let client_mark = client.offset();
        host.hover(640.0, 360.0)?;
        host.key("Escape")?;
        host.wait_line(
            &["LoadMap: /Game/Maps/MainMenu"],
            Duration::from_secs(20),
            host_mark,
        )?;
        let failure = client.wait_line(
            &["MP_CLIENT_NETWORK_FAILURE"],
            Duration::from_secs(25),
            client_mark,
        )?;
        client.wait_line(
            &["LoadMap: /Game/Maps/MainMenu"],
            Duration::from_secs(25),
            client_mark,
        )?;
        thread::sleep(Duration::from_secs(1));
        report.artifact("host_menu_png", &host.screenshot(evidence, "05-menu")?);
        report.artifact("client_menu_png", &client.screenshot(evidence, "05-menu")?);
        report.verdict(
            "MP1-G6-NORMAL-RETURN",
            field(&failure, "result_observed") == Some("1"),
            format!(
                "both processes returned to MainMenu; client explained the ended match: {}",
                failure.trim()
            ),
        );

        // MP1-12: rematch in the same processes uses a fresh generation and fresh bindings.
        let awaiting2 = host.wait_line(
            &["MP_MATCH_AWAITING_PARTICIPANTS"],
            Duration::from_secs(40),
            host_mark,
        )?;
        let generation2 = field(&awaiting2, "generation").unwrap_or("0").to_owned();
        let active2 = host.wait_line(
            &["MP_MATCH_ACTIVE", &format!("generation={generation2}")],
            Duration::from_secs(60),
            host_mark,
        )?;
        let rebound = host.wait_line(
            &[
                "MP_PARTICIPANT_BOUND participant=2",
                &format!("generation={generation2}"),
            ],
            Duration::from_secs(5),
            host_mark,
        )?;
        let reassigned = client.wait_line(
            &[
                "MP_CLIENT_ASSIGNMENT participant=2",
                &format!("generation={generation2}"),
            ],
            Duration::from_secs(30),
            client_mark,
        )?;
        let fresh = generation2.parse::<u64>().unwrap_or(0)
            > generation1.parse::<u64>().unwrap_or(u64::MAX);
        report.verdict(
            "MP1-12",
            fresh,
            format!(
                "rematch generation {generation1} -> {generation2}; {}; {}; {}",
                active2.trim(),
                rebound.trim(),
                reassigned.trim()
            ),
        );
        thread::sleep(Duration::from_secs(2));
        report.artifact(
            "host_rematch_png",
            &host.screenshot(evidence, "06-rematch")?,
        );
        report.artifact(
            "client_rematch_png",
            &client.screenshot(evidence, "06-rematch")?,
        );

        // MP1-03: a third connection is refused before any controller/seat exists.
        let mut third = Game::launch(
            &options.binary,
            "third",
            evidence,
            &client_args(options, false, false, false),
            None,
        )?;
        report.fact("third_pid", third.pid().to_string());
        let host_mark3 = host.offset();
        let refused = host.wait_line(
            &["MP_PRELOGIN REJECTED"],
            Duration::from_secs(60),
            host_mark3,
        )?;
        let third_failure = third.wait(&["MP_CLIENT_NETWORK_FAILURE"], Duration::from_secs(30))?;
        third.wait(&["LoadMap: /Game/Maps/MainMenu"], Duration::from_secs(30))?;
        let no_seat = !host.text()[host_mark3..].contains("MP_PARTICIPANT_BOUND");
        report.verdict(
            "MP1-03",
            field(&refused, "reason") == Some("MP.SESSION.IN_PROGRESS") && no_seat,
            format!(
                "{}; third process: {}",
                refused.trim(),
                third_failure.trim()
            ),
        );
        third.stop();

        // Remote-client process failure: the server revokes the seat and ends the match once.
        let host_mark4 = host.offset();
        let killed_at = Instant::now();
        client.kill_hard();
        let unbound = host.wait_line(
            &["MP_PARTICIPANT_UNBOUND participant=2", "remote=1"],
            Duration::from_secs(30),
            host_mark4,
        )?;
        let detected = killed_at.elapsed();
        let ended = host.wait_line(
            &[
                "MATCH_RESULTS outcome=participant_disconnected",
                &format!("generation={generation2}"),
            ],
            Duration::from_secs(5),
            host_mark4,
        )?;
        host.wait_line(
            &["MP_HOST_RETURNING reason=participant_disconnected"],
            Duration::from_secs(10),
            host_mark4,
        )?;
        host.wait_line(
            &["LoadMap: /Game/Maps/MainMenu"],
            Duration::from_secs(20),
            host_mark4,
        )?;
        thread::sleep(Duration::from_secs(1));
        report.artifact(
            "host_after_client_loss_png",
            &host.screenshot(evidence, "07-client-lost-menu")?,
        );
        let commits2 = host
            .text()
            .lines()
            .filter(|l| {
                l.contains("MATCH_RESULTS") && l.contains(&format!("generation={generation2}"))
            })
            .count();
        report.verdict(
            "MP1-G6-CLIENT-LOSS",
            commits2 == 1,
            format!(
                "client SIGKILL detected in {:.1}s: {}; {}; host returned to MainMenu",
                detected.as_secs_f64(),
                unbound.trim(),
                ended.trim()
            ),
        );
        Ok(())
    })();
    let host_video = host_recorder.finish();
    let client_video = client_recorder.finish();
    match &host_video {
        Ok(path) => report.artifact("host_mp4", path),
        Err(error) => report.mark("EVIDENCE-HOST-MP4", "INFRA_ERROR", error.clone()),
    }
    match &client_video {
        Ok(path) => report.artifact("client_mp4", path),
        Err(error) => report.mark("EVIDENCE-CLIENT-MP4", "INFRA_ERROR", error.clone()),
    }
    launch_options_verdict(report, "MP1-DEV-LAUNCH-OPTIONS", &[&host, &client]);
    report.artifact("host_log", &host.log);
    report.artifact("client_log", &client.log);
    host.stop();
    client.stop();
    result
}

fn session_host_loss(options: &Options, report: &mut Report) -> Result<(), String> {
    let evidence = &options.evidence;
    let host_display = Display::start(evidence, "host-loss-host")?;
    let client_display = Display::start(evidence, "host-loss-client")?;
    let mut host = Game::launch(
        &options.binary,
        "hostloss-host",
        evidence,
        &host_args(options, false),
        Some(&host_display.name()),
    )?;
    host.wait(&["MP_MATCH_AWAITING_PARTICIPANTS"], Duration::from_secs(90))?;
    let mut client = Game::launch(
        &options.binary,
        "hostloss-client",
        evidence,
        &client_args(options, true, false, false),
        Some(&client_display.name()),
    )?;
    host.wait(&["MP_MATCH_ACTIVE"], Duration::from_secs(60))?;
    client.wait(&["MP_CLIENT_MATCH_OBSERVED"], Duration::from_secs(30))?;
    client.attach_window()?;
    thread::sleep(Duration::from_secs(2));
    let mark = client.offset();
    let killed_at = Instant::now();
    host.kill_hard();
    let failure = client.wait_line(
        &["MP_CLIENT_NETWORK_FAILURE"],
        Duration::from_secs(30),
        mark,
    )?;
    let detected = killed_at.elapsed();
    client.wait_line(
        &["LoadMap: /Game/Maps/MainMenu"],
        Duration::from_secs(30),
        mark,
    )?;
    thread::sleep(Duration::from_secs(2));
    report.artifact(
        "client_after_host_loss_png",
        &client.screenshot(evidence, "08-host-lost-menu")?,
    );
    let explained = client.text()[mark..].contains("NetworkConnectionLost");
    report.verdict("MP1-G6-HOST-LOSS", field(&failure, "result_observed") == Some("0") && explained && detected < Duration::from_secs(20),
        format!("host SIGKILL detected by the client in {:.1}s (ConnectionTimeout=10s): {}; recovery reason NetworkConnectionLost={explained}",
            detected.as_secs_f64(), failure.trim()));
    report.artifact("hostloss_client_log", &client.log);
    client.stop();
    Ok(())
}

/// Shipping regression: default offline boot with no network namespace, rendered to Xvfb,
/// and absence of Development-only multiplayer fixture/probe entry points in the binary.
fn shipping_boot(binary: &Path, evidence: &Path) -> Result<Report, String> {
    let mut report = Report::default();
    fs::create_dir_all(evidence).map_err(|e| e.to_string())?;
    report.fact("binary", binary.display().to_string());
    report.fact("executable_sha256", sha256(binary)?);
    let strings = run_output(Command::new("strings").args(["-el"]).arg(binary))?;
    let forbidden = [
        "MultiplayerSkirmish",
        "map.test.multiplayer_skirmish",
        "PolitikaMPProbe",
        "MP_PROBE_SENT",
        "PolitikaMPRematch",
        "RTSAcceptance",
    ];
    let present: Vec<&str> = forbidden
        .iter()
        .copied()
        .filter(|s| strings.contains(s))
        .collect();
    report.verdict(
        "SHIP-NO-DEV-ENTRY-POINTS",
        present.is_empty(),
        format!("Development-only strings present: {present:?}"),
    );
    let display = Display::start(evidence, "shipping")?;
    report.fact("display", display.name());
    let user_dir = evidence.join("user-shipping");
    fs::create_dir_all(&user_dir).map_err(|e| e.to_string())?;
    // No network namespace: proves the default route needs no network or online service.
    let mut child = Command::new("unshare")
        .args(["--user", "--map-current-user", "--net", "--"])
        .arg(binary)
        .args([
            "-windowed",
            &format!("-ResX={WIDTH}"),
            &format!("-ResY={HEIGHT}"),
            "-unattended",
            "-nosplash",
        ])
        .arg(format!("-UserDir={}/", user_dir.display()))
        .env_remove("WAYLAND_DISPLAY")
        .env("DISPLAY", display.name())
        .env("SDL_VIDEODRIVER", "x11")
        .stdin(Stdio::null())
        .stdout(Stdio::null())
        .stderr(Stdio::null())
        .spawn()
        .map_err(|e| format!("launch Shipping in a network-less namespace: {e}"))?;
    let mut window = None;
    for _ in 0..300 {
        if let Ok(found) = xdo(
            &display.name(),
            &["search", "--onlyvisible", "--name", "Politika"],
        ) {
            window = found.lines().next().map(str::to_owned);
            if window.is_some() {
                break;
            }
        }
        thread::sleep(Duration::from_millis(100));
    }
    thread::sleep(Duration::from_secs(10));
    let alive = matches!(child.try_wait(), Ok(None));
    let shot = evidence.join("shipping-main-menu.png");
    let captured = window.as_deref().is_some_and(|id| {
        run_output(
            Command::new("import")
                .env("DISPLAY", display.name())
                .args(["-window", id])
                .arg(&shot),
        )
        .is_ok()
    });
    // The configured default route is the offline RTS test map; Escape must reach MainMenu.
    let menu = evidence.join("shipping-main-menu-after-escape.png");
    let menu_captured = window.as_deref().is_some_and(|id| {
        xdo(
            &display.name(),
            &["mousemove", "--window", id, "--sync", "640", "360"],
        )
        .is_ok()
            && xdo(&display.name(), &["key", "Escape"]).is_ok()
            && {
                thread::sleep(Duration::from_secs(6));
                run_output(
                    Command::new("import")
                        .env("DISPLAY", display.name())
                        .args(["-window", id])
                        .arg(&menu),
                )
                .is_ok()
            }
    });
    let alive_after_menu = matches!(child.try_wait(), Ok(None));
    let _ = child.kill();
    let _ = child.wait();
    report.verdict("SHIP-OFFLINE-BOOT", alive && captured,
        format!("Shipping default route rendered in a network-less namespace: alive={alive}; window captured={captured}"));
    report.verdict("SHIP-MAIN-MENU", menu_captured && alive_after_menu,
        format!("Escape returned to MainMenu in Shipping: captured={menu_captured}; alive={alive_after_menu} (inspect screenshot)"));
    if captured {
        report.artifact("shipping_default_route_png", &shot);
    }
    if menu_captured {
        report.artifact("shipping_main_menu_png", &menu);
    }
    Ok(report)
}

// ---------------------------------------------------------------------------------------
// MP-3 dedicated server: a headless PolitikaServer process hosts; every player is a remote
// client of the packaged Development game.
// ---------------------------------------------------------------------------------------

struct DedicatedOptions {
    server: PathBuf,
    client: Options,
}

fn parse_dedicated_options() -> Result<DedicatedOptions, String> {
    let usage = "usage: rts_multiplayer --dedicated SERVER_BINARY CLIENT_BINARY [--evidence DIR] [--port N]";
    let mut args = env::args().skip(2);
    let server = PathBuf::from(args.next().ok_or(usage)?)
        .canonicalize()
        .map_err(|e| format!("server binary: {e}"))?;
    let binary = PathBuf::from(args.next().ok_or(usage)?)
        .canonicalize()
        .map_err(|e| format!("client binary: {e}"))?;
    let mut evidence = None;
    let mut port = 17790u16;
    while let Some(flag) = args.next() {
        let value = args.next().ok_or_else(|| format!("{flag} needs a value"))?;
        match flag.as_str() {
            "--evidence" => evidence = Some(PathBuf::from(value)),
            "--port" => port = value.parse().map_err(|e| format!("--port: {e}"))?,
            other => return Err(format!("unknown option {other}")),
        }
    }
    let epoch = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .map_err(|e| e.to_string())?
        .as_secs();
    let evidence = evidence.unwrap_or_else(|| {
        PathBuf::from(format!(
            "build/diagnostics/mp3-dedicated-server/packaged-{epoch}"
        ))
    });
    Ok(DedicatedOptions {
        server,
        client: Options {
            binary,
            layout: "1v1".into(),
            evidence,
            port,
        },
    })
}

fn server_args(port: u16, layout: &str) -> Vec<String> {
    let mut args = vec![
        format!("/Game/Maps/RTSVerticalSlice?PolitikaLayout={layout}"),
        format!("-port={port}"),
        "-PolitikaTestFixture=MultiplayerSkirmish".into(),
        "-RTSAcceptance=multiplayer".into(),
    ];
    args.extend(pinned_forces());
    args
}

const NO_STARTING_HARVESTERS: &str = "-PolitikaMPStartingHarvesters=0";
const STARTING_HARVESTERS: &str = "-PolitikaMPStartingHarvesters=1";

/// Sessions pin exact forces, so they turn the free starting harvester off (ECON-E5), unless the
/// matrix is running them with the normal default to see that they still hold.
fn pinned_forces() -> Vec<String> {
    if env::var_os("POLITIKA_HARNESS_DEFAULT_FORCES").is_some() {
        Vec::new()
    } else {
        vec![NO_STARTING_HARVESTERS.into()]
    }
}

/// Variables that would let a process reach a display server, audio server or the
/// operator's desktop session. The dedicated server is launched with none of them.
const DESKTOP_ENV: [&str; 8] = [
    "DISPLAY",
    "WAYLAND_DISPLAY",
    "XDG_RUNTIME_DIR",
    "XDG_SESSION_TYPE",
    "DBUS_SESSION_BUS_ADDRESS",
    "PULSE_SERVER",
    "PIPEWIRE_REMOTE",
    "SDL_VIDEODRIVER",
];

impl Game {
    /// Launches the headless server with a minimal environment: an isolated HOME and user
    /// directory, PATH and locale only. No display, audio or session-bus variables exist.
    fn launch_server(
        binary: &Path,
        label: &str,
        evidence: &Path,
        args: &[String],
    ) -> Result<Self, String> {
        let user_dir = evidence.join(format!("user-{label}"));
        let home = evidence.join(format!("home-{label}"));
        fs::create_dir_all(&user_dir).map_err(|e| e.to_string())?;
        fs::create_dir_all(&home).map_err(|e| e.to_string())?;
        let log = evidence.join(format!("{label}.log"));
        let mut all: Vec<String> = args.to_vec();
        all.push(format!("-UserDir={}/", user_dir.display()));
        all.push(format!("-ABSLOG={}", log.display()));
        all.push("-unattended".into());
        let mut command = Command::new(binary);
        command
            .args(&all)
            .env_clear()
            .env("PATH", env::var("PATH").unwrap_or_default())
            .env("HOME", &home)
            .env("LANG", "C.UTF-8")
            .stdin(Stdio::null())
            .stdout(Stdio::null())
            .stderr(Stdio::null());
        let child = command
            .spawn()
            .map_err(|e| format!("launch {label}: {e}"))?;
        let mut recorded = vec![binary.display().to_string()];
        recorded.extend(all);
        Ok(Game {
            label: label.into(),
            child,
            log,
            display: None,
            window: None,
            command: recorded,
        })
    }

    /// Environment of the running process, as the kernel reports it.
    fn environment(&self) -> Vec<String> {
        fs::read(format!("/proc/{}/environ", self.pid()))
            .map(|bytes| {
                bytes
                    .split(|b| *b == 0)
                    .filter(|v| !v.is_empty())
                    .map(|v| String::from_utf8_lossy(v).into_owned())
                    .collect()
            })
            .unwrap_or_default()
    }
}

fn files_under(dir: &Path) -> Vec<PathBuf> {
    let mut out = Vec::new();
    let mut stack = vec![dir.to_path_buf()];
    while let Some(next) = stack.pop() {
        let Ok(entries) = fs::read_dir(&next) else {
            continue;
        };
        for entry in entries.flatten() {
            let path = entry.path();
            if path.is_dir() {
                stack.push(path);
            } else {
                out.push(path);
            }
        }
    }
    out.sort();
    out
}

/// Authoritative setup lines whose content must be identical on a listen host and a
/// dedicated server of the same build, fixture and layout.
fn setup_signature(text: &str) -> Vec<String> {
    let markers = [
        "MATCH_AUTHORITATIVE_UNIT ",
        "MATCH_AUTHORITATIVE_OBJECTIVE ",
        "MATCH_UNALIGNED_SIDE ",
        "MP_ACCEPTANCE_UNALIGNED ",
        "CONTENT_VALIDATION_PASSED ",
    ];
    let mut lines: Vec<String> = Vec::new();
    for line in text.lines() {
        if let Some(marker) = markers.iter().find(|m| line.contains(*m)) {
            let start = line.find(marker).unwrap_or(0);
            lines.push(line[start..].trim().to_owned());
        }
        if line.contains("MP_ACCEPTANCE_FIXTURE ") {
            break; // first match generation only
        }
    }
    if let Some(layout) = text.lines().find(|l| l.contains("MP_SESSION_LAYOUT ")) {
        lines.push(format!(
            "layout={} seats={} fog={} roster={}",
            field(layout, "layout").unwrap_or("?"),
            field(layout, "seats").unwrap_or("?"),
            field(layout, "fog").unwrap_or("?"),
            field(layout, "roster").unwrap_or("?")
        ));
    }
    lines
}

fn count_lines(text: &str, fragments: &[&str]) -> usize {
    text.lines()
        .filter(|line| fragments.iter().all(|f| line.contains(f)))
        .count()
}

fn dedicated_gameplay(options: &DedicatedOptions, report: &mut Report) -> Result<(), String> {
    let client_options = &options.client;
    let evidence = &client_options.evidence;
    let port = client_options.port;

    // Headless boot: no display, audio, desktop session or renderer flag.
    let mut server = Game::launch_server(
        &options.server,
        "server",
        evidence,
        &server_args(port, "1v1"),
    )?;
    report.fact("server_pid", server.pid().to_string());
    report.fact("server_command", server.command.join(" "));
    let server_pid = server.pid();
    let ready = server.wait(&["MP_DEDICATED_SERVER_READY"], Duration::from_secs(120))?;
    let server_env = server.environment();
    let leaked: Vec<&str> = DESKTOP_ENV
        .iter()
        .copied()
        .filter(|key| server_env.iter().any(|v| v.starts_with(&format!("{key}="))))
        .collect();
    report.fact("server_environment", server_env.join(" "));
    let no_renderer_flag = !server
        .command
        .iter()
        .any(|a| a.eq_ignore_ascii_case("-nullrhi"));
    report.verdict(
        "MP3-BOOT-HEADLESS",
        field(&ready, "port") == Some(&port.to_string())
            && field(&ready, "net_driver") == Some("1")
            && field(&ready, "local_players") == Some("0")
            && field(&ready, "can_render") == Some("0")
            && field(&ready, "can_render_audio") == Some("0")
            && leaked.is_empty()
            && no_renderer_flag,
        format!(
            "{}; desktop/audio variables present={leaked:?}; -nullrhi passed={}",
            ready.trim(),
            !no_renderer_flag
        ),
    );
    let awaiting = server.wait(&["MP_MATCH_AWAITING_PARTICIPANTS"], Duration::from_secs(30))?;
    let generation1 = field(&awaiting, "generation").unwrap_or("0").to_owned();
    let layout = server.wait(&["MP_SESSION_LAYOUT"], Duration::from_secs(5))?;
    report.verdict(
        "MP3-LAYOUT-1V1-UNALIGNED",
        field(&layout, "layout") == Some("1v1")
            && field(&layout, "seats") == Some("2")
            && field(&layout, "fog") == Some("1")
            && field(&layout, "hosting") == Some("dedicated")
            && field(&layout, "remote_seats") == Some("2")
            && server.text().contains("MATCH_UNALIGNED_SIDE units=2"),
        format!("{}; both seats are remote", layout.trim()),
    );

    // Client A joins first (seat 0 / participant 1 / team A), then client B (participant 2).
    let display_a = Display::start(evidence, "client-a")?;
    let display_b = Display::start(evidence, "client-b")?;
    let mut args_a = client_args(client_options, true, false, false);
    args_a.extend([
        format!("-PolitikaMPRematch=127.0.0.1:{port}"),
        "-PolitikaMPRematchDelay=3".into(),
        "-PolitikaMPRematchAttempts=1".into(),
    ]);
    let mut client_a = Game::launch(
        &client_options.binary,
        "client-a",
        evidence,
        &args_a,
        Some(&display_a.name()),
    )?;
    report.fact("client_a_pid", client_a.pid().to_string());
    report.fact("client_a_command", client_a.command.join(" "));
    let bound_a = server.wait(
        &["MP_PARTICIPANT_BOUND participant=1", "remote=1"],
        Duration::from_secs(60),
    )?;
    let mut client_b = Game::launch(
        &client_options.binary,
        "client-b",
        evidence,
        &client_args(client_options, true, true, true),
        Some(&display_b.name()),
    )?;
    report.fact("client_b_pid", client_b.pid().to_string());
    report.fact("client_b_command", client_b.command.join(" "));
    let bound_b = server.wait(
        &["MP_PARTICIPANT_BOUND participant=2", "remote=1"],
        Duration::from_secs(60),
    )?;
    let active = server.wait(
        &["MP_MATCH_ACTIVE", &format!("generation={generation1}")],
        Duration::from_secs(10),
    )?;
    let assignment_a = client_a.wait(
        &["MP_CLIENT_ASSIGNMENT participant=1"],
        Duration::from_secs(30),
    )?;
    let assignment_b = client_b.wait(
        &["MP_CLIENT_ASSIGNMENT participant=2"],
        Duration::from_secs(30),
    )?;
    for client in [&mut client_a, &mut client_b] {
        client.wait(
            &[
                "MP_CLIENT_MATCH_OBSERVED",
                "map=map.test.multiplayer_skirmish",
            ],
            Duration::from_secs(30),
        )?;
    }
    report.verdict(
        "MP3-01",
        field(&bound_a, "team") == Some("1")
            && field(&assignment_a, "generation") == Some(generation1.as_str()),
        format!(
            "first remote takes seat 0: {} / client: {}",
            bound_a.trim(),
            assignment_a.trim()
        ),
    );
    report.verdict(
        "MP3-02",
        field(&bound_b, "team") == Some("2")
            && field(&assignment_b, "generation") == Some(generation1.as_str()),
        format!(
            "second remote takes seat 1: {} / client: {}",
            bound_b.trim(),
            assignment_b.trim()
        ),
    );
    report.verdict(
        "MP3-ACTIVE-ALL-REMOTE",
        field(&active, "hosting") == Some("dedicated")
            && field(&active, "local") == Some("0")
            && field(&active, "remote") == Some("2")
            && server_pid != client_a.pid()
            && client_a.pid() != client_b.pid(),
        format!(
            "three independent processes (server {server_pid}, clients {} / {}): {}",
            client_a.pid(),
            client_b.pid(),
            active.trim()
        ),
    );

    client_a.attach_window()?;
    client_b.attach_window()?;
    let recorder_a = Recorder::start(&display_a.name(), evidence, "client-a")?;
    let recorder_b = Recorder::start(&display_b.name(), evidence, "client-b")?;
    let result = (|| -> Result<(), String> {
        thread::sleep(Duration::from_secs(2));
        report.artifact(
            "client_a_active_png",
            &client_a.screenshot(evidence, "01-active")?,
        );
        report.artifact(
            "client_b_active_png",
            &client_b.screenshot(evidence, "01-active")?,
        );

        // Fog: each client initially receives only its own units.
        let owners = entity_owners(&server.text());
        let mut fog_ok = !owners.is_empty();
        let mut fog_detail = Vec::new();
        for (client, participant) in [(&client_a, 1u32), (&client_b, 2u32)] {
            let relevant: Vec<(u64, u32)> = client
                .text()
                .lines()
                .filter(|l| l.contains("MP_CLIENT_UNIT_RELEVANT"))
                .filter_map(|l| {
                    Some((
                        field(l, "entity")?.parse().ok()?,
                        field(l, "owner")?.parse().ok()?,
                    ))
                })
                .collect();
            let own = owners.iter().filter(|(_, o)| *o == participant).count();
            fog_ok &= !relevant.is_empty()
                && relevant.iter().all(|(_, owner)| *owner == participant)
                && relevant.len() == own;
            fog_detail.push(format!("participant {participant} received {relevant:?}"));
        }
        report.verdict(
            "MP3-FOG-INITIAL",
            fog_ok,
            format!("authoritative entities {owners:?}; {fog_detail:?}"),
        );

        client_b.wait(&["MP_PROBE_COMPLETE"], Duration::from_secs(20))?;
        thread::sleep(Duration::from_secs(2));
        let units_of = |participant: u32| -> Vec<u64> {
            owners
                .iter()
                .filter(|(_, o)| *o == participant)
                .map(|(e, _)| *e)
                .collect()
        };
        let (a_units, b_units) = (units_of(1), units_of(2));
        let (unit_a1, unit_a2, unit_b1) = match (a_units.first(), a_units.get(1), b_units.first()) {
            (Some(a1), Some(a2), Some(b1)) => (*a1, *a2, *b1),
            _ => return Err(format!("unexpected authoritative roster {owners:?}")),
        };

        // Client B real input: Sprint, Stop, Guard, then Move toward the objective.
        let since = server.offset();
        select_entity(&mut client_b, "MP_CLIENT_SAMPLE", unit_b1)?;
        let mut b_orders = Vec::new();
        for (key, order) in [("f", 7u32), (STOP_KEY, 3), ("g", 4)] {
            let mark = client_b.offset();
            client_b.key(key)?;
            let outcome = client_b.wait_line(
                &[
                    "MP_COMMAND_OUTCOME",
                    &format!("order={order}"),
                    "accepted=1",
                    "authority=0",
                ],
                Duration::from_secs(5),
                mark,
            )?;
            b_orders.push(format!("{key}->{}", outcome.trim()));
            thread::sleep(Duration::from_millis(400));
        }
        zoom_out(&client_b, 5)?;
        thread::sleep(Duration::from_millis(2500));
        let objective = wait_screen(
            &mut client_b,
            |t| objective_screen(t, "MP_CLIENT_OBJECTIVE"),
            "objective on client B",
        )?;
        let unit_screen = wait_screen(
            &mut client_b,
            |t| entity_screen(t, "MP_CLIENT_SAMPLE", unit_b1),
            "client B unit",
        )?;
        let target = (
            objective.0 + (unit_screen.0 - objective.0) * 0.45,
            objective.1 + (unit_screen.1 - objective.1) * 0.45,
        );
        let mark = client_b.offset();
        client_b.click(target.0, target.1, "3")?;
        let b_move = client_b.wait_line(
            &["MP_COMMAND_OUTCOME", "order=0", "accepted=1"],
            Duration::from_secs(5),
            mark,
        )?;
        let b_admitted = server.wait_line(
            &[
                "RTS_ORDER_ADMITTED order=Move",
                &format!("unit={unit_b1}"),
                "participant=2",
            ],
            Duration::from_secs(5),
            since,
        )?;
        report.verdict(
            "MP3-POS-CLIENT-B-ORDERS",
            b_orders.len() == 3,
            format!(
                "remote real input Sprint/Stop/Guard/Move: {b_orders:?}; {}; {}",
                b_move.trim(),
                b_admitted.trim()
            ),
        );
        report.artifact(
            "client_b_orders_png",
            &client_b.screenshot(evidence, "02-orders")?,
        );

        // Client A (seat 0, remote) real input: unit 2 Stop/Guard, unit 1 Move onto the
        // objective, unit 2 Attack-move to the objective.
        let since = server.offset();
        select_entity(&mut client_a, "MP_CLIENT_SAMPLE", unit_a2)?;
        for (key, order) in [(STOP_KEY, "Stop"), ("g", "Guard")] {
            let mark = server.offset();
            client_a.key(key)?;
            server.wait_line(
                &[
                    &format!("RTS_ORDER_ADMITTED order={order}"),
                    &format!("unit={unit_a2}"),
                    "participant=1",
                ],
                Duration::from_secs(5),
                mark,
            )?;
        }
        select_entity(&mut client_a, "MP_CLIENT_SAMPLE", unit_a1)?;
        zoom_out(&client_a, 5)?;
        thread::sleep(Duration::from_millis(2500));
        let objective_a = wait_screen(
            &mut client_a,
            |t| objective_screen(t, "MP_CLIENT_OBJECTIVE"),
            "objective on client A",
        )?;
        client_a.click(objective_a.0, objective_a.1, "3")?;
        let a_move = server.wait_line(
            &[
                "RTS_ORDER_ADMITTED order=Move",
                &format!("unit={unit_a1}"),
                "participant=1",
            ],
            Duration::from_secs(5),
            since,
        )?;
        select_entity(&mut client_a, "MP_CLIENT_SAMPLE", unit_a2)?;
        client_a.hover(objective_a.0 + 18.0, objective_a.1 + 10.0)?;
        thread::sleep(Duration::from_millis(150));
        let mark = server.offset();
        client_a.key(ATTACK_MOVE_KEY)?;
        let a_attack_move = server.wait_line(
            &[
                "RTS_ORDER_ADMITTED order=AttackMove",
                &format!("unit={unit_a2}"),
                "participant=1",
            ],
            Duration::from_secs(5),
            mark,
        )?;
        let server_text = server.text();
        let all_remote = count_lines(
            &server_text,
            &["MP_COMMAND_RECEIVED participant=1 remote=1"],
        ) > 0
            && count_lines(&server_text, &["MP_COMMAND_RECEIVED", "remote=0"]) == 0;
        report.verdict(
            "MP3-POS-CLIENT-A-ORDERS",
            all_remote,
            format!("seat-0 orders arrive remotely through the single intake (no remote=0 command exists): {}; {}", a_move.trim(), a_attack_move.trim()),
        );
        report.artifact(
            "client_a_orders_png",
            &client_a.screenshot(evidence, "02-orders")?,
        );

        let reveal = client_b.wait_line(
            &["MP_CLIENT_UNIT_RELEVANT", "owner=1"],
            Duration::from_secs(40),
            0,
        );
        report.verdict(
            "MP3-FOG-REVEAL",
            reveal.is_ok(),
            format!("participant-1 unit replicated to client B once observed: {reveal:?}"),
        );
        report.artifact(
            "client_b_contact_png",
            &client_b.screenshot(evidence, "03-contact")?,
        );

        let results = server.wait(
            &[
                "MATCH_RESULTS outcome=objective_secured",
                &format!("generation={generation1}"),
            ],
            Duration::from_secs(90),
        )?;
        let observed_a = client_a.wait(
            &[
                "MP_CLIENT_LIFECYCLE",
                "result=1",
                "reason=objective_secured",
            ],
            Duration::from_secs(15),
        )?;
        let observed_b = client_b.wait(
            &[
                "MP_CLIENT_LIFECYCLE",
                "result=1",
                "reason=objective_secured",
            ],
            Duration::from_secs(15),
        )?;
        let linger = server.wait(
            &[
                "MP_DEDICATED_SESSION_CLOSE_SCHEDULED reason=results_linger_elapsed",
                &format!("generation={generation1}"),
            ],
            Duration::from_secs(5),
        )?;
        thread::sleep(Duration::from_millis(1500));
        report.artifact(
            "client_a_results_png",
            &client_a.screenshot(evidence, "04-results")?,
        );
        report.artifact(
            "client_b_results_png",
            &client_b.screenshot(evidence, "04-results")?,
        );
        let commits = count_lines(
            &server.text(),
            &["MATCH_RESULTS", &format!("generation={generation1}")],
        );
        report.verdict(
            "MP3-11",
            commits == 1
                && field(&results, "team") == Some("1")
                && field(&observed_a, "team") == Some("1")
                && field(&observed_a, "viewer_team") == Some("1")
                && field(&observed_b, "team") == Some("1")
                && field(&observed_b, "viewer_team") == Some("2"),
            format!(
                "server committed exactly {commits} result: {}; A observed {}; B observed {}; {}",
                results.trim(),
                observed_a.trim(),
                observed_b.trim(),
                linger.trim()
            ),
        );
        health_consistency(report, "MP3-POS-DAMAGE-CONSISTENCY-B", &server, &client_b);
        health_consistency(report, "MP3-POS-DAMAGE-CONSISTENCY-A", &server, &client_a);
        client_b.wait(
            &["MP_PROBE_SENT name=after_termination"],
            Duration::from_secs(10),
        )?;
        thread::sleep(Duration::from_secs(1));
        check_probes_for(report, "MP3-NEG-PROBES", &client_b, &server, &[1, 2]);

        // Both clients leave the finished match; the server resets for a new generation.
        let server_mark = server.offset();
        let (mark_a, mark_b) = (client_a.offset(), client_b.offset());
        for client in [&client_a, &client_b] {
            client.hover(640.0, 360.0)?;
            client.key("Escape")?;
        }
        client_a.wait_line(
            &["LoadMap: /Game/Maps/MainMenu"],
            Duration::from_secs(25),
            mark_a,
        )?;
        client_b.wait_line(
            &["LoadMap: /Game/Maps/MainMenu"],
            Duration::from_secs(25),
            mark_b,
        )?;
        // Capture the menu now: each client schedules its rematch travel 3 s after reaching
        // MainMenu, which is about when the server finishes its reset, so a capture taken after
        // the reset can land on the rejoin's map load and record a blank frame.
        thread::sleep(Duration::from_secs(1));
        report.artifact(
            "client_a_menu_png",
            &client_a.screenshot(evidence, "05-menu")?,
        );
        report.artifact(
            "client_b_menu_png",
            &client_b.screenshot(evidence, "05-menu")?,
        );
        let closing = server.wait_line(
            &[
                "MP_DEDICATED_SESSION_CLOSING reason=results_complete",
                &format!("generation={generation1}"),
            ],
            Duration::from_secs(30),
            server_mark,
        )?;
        let reset = server.wait_line(
            &["MP_DEDICATED_SESSION_RESET", "travel=1"],
            Duration::from_secs(10),
            server_mark,
        )?;
        let ready2 = server.wait_line(
            &["MP_DEDICATED_SERVER_READY"],
            Duration::from_secs(60),
            server_mark,
        )?;
        let awaiting2 = server.wait_line(
            &["MP_MATCH_AWAITING_PARTICIPANTS"],
            Duration::from_secs(10),
            server_mark,
        )?;
        let generation2 = field(&awaiting2, "generation").unwrap_or("0").to_owned();
        report.verdict(
            "MP3-RESET-AFTER-RESULTS",
            server.alive()
                && server.pid() == server_pid
                && field(&closing, "remaining") == Some("0")
                && field(&ready2, "port") == Some(&port.to_string())
                && generation2.parse::<u64>().unwrap_or(0)
                    > generation1.parse::<u64>().unwrap_or(u64::MAX),
            format!(
                "both clients left to MainMenu; same server process reset: {}; {}; {}; {}",
                closing.trim(),
                reset.trim(),
                ready2.trim(),
                awaiting2.trim()
            ),
        );

        // Rematch on the same server: fresh generation and fresh bindings.
        let active2 = server.wait_line(
            &["MP_MATCH_ACTIVE", &format!("generation={generation2}")],
            Duration::from_secs(60),
            server_mark,
        )?;
        let rebound_a = server.wait_line(
            &[
                "MP_PARTICIPANT_BOUND participant=1",
                &format!("generation={generation2}"),
            ],
            Duration::from_secs(5),
            server_mark,
        )?;
        let rebound_b = server.wait_line(
            &[
                "MP_PARTICIPANT_BOUND participant=2",
                &format!("generation={generation2}"),
            ],
            Duration::from_secs(5),
            server_mark,
        )?;
        let reassigned_a = client_a.wait_line(
            &[
                "MP_CLIENT_ASSIGNMENT participant=1",
                &format!("generation={generation2}"),
            ],
            Duration::from_secs(30),
            mark_a,
        )?;
        let reassigned_b = client_b.wait_line(
            &[
                "MP_CLIENT_ASSIGNMENT participant=2",
                &format!("generation={generation2}"),
            ],
            Duration::from_secs(30),
            mark_b,
        )?;
        report.verdict(
            "MP3-12-REMATCH",
            true,
            format!(
                "generation {generation1} -> {generation2}: {}; {}; {}; {}; {}",
                active2.trim(),
                rebound_a.trim(),
                rebound_b.trim(),
                reassigned_a.trim(),
                reassigned_b.trim()
            ),
        );
        thread::sleep(Duration::from_secs(2));
        report.artifact(
            "client_a_rematch_png",
            &client_a.screenshot(evidence, "06-rematch")?,
        );
        report.artifact(
            "client_b_rematch_png",
            &client_b.screenshot(evidence, "06-rematch")?,
        );

        // A third connection is refused before any controller or seat exists.
        let mut third = Game::launch(
            &client_options.binary,
            "third",
            evidence,
            &client_args(client_options, false, false, false),
            None,
        )?;
        report.fact("third_pid", third.pid().to_string());
        let mark3 = server.offset();
        let refused =
            server.wait_line(&["MP_PRELOGIN REJECTED"], Duration::from_secs(60), mark3)?;
        let third_failure = third.wait(&["MP_CLIENT_NETWORK_FAILURE"], Duration::from_secs(30))?;
        third.wait(&["LoadMap: /Game/Maps/MainMenu"], Duration::from_secs(30))?;
        let no_seat = !server.text()[mark3..].contains("MP_PARTICIPANT_BOUND");
        report.verdict(
            "MP3-03",
            field(&refused, "reason") == Some("MP.SESSION.IN_PROGRESS") && no_seat,
            format!(
                "{}; third process: {}",
                refused.trim(),
                third_failure.trim()
            ),
        );
        third.stop();

        // Client loss: the server ends the match once, removes the remaining client and resets.
        let mark4 = server.offset();
        let mark_b4 = client_b.offset();
        let killed_at = Instant::now();
        client_a.kill_hard();
        let unbound = server.wait_line(
            &["MP_PARTICIPANT_UNBOUND participant=1", "remote=1"],
            Duration::from_secs(30),
            mark4,
        )?;
        let detected = killed_at.elapsed();
        let ended = server.wait_line(
            &[
                "MATCH_RESULTS outcome=participant_disconnected",
                &format!("generation={generation2}"),
            ],
            Duration::from_secs(5),
            mark4,
        )?;
        let closing2 = server.wait_line(
            &["MP_DEDICATED_SESSION_CLOSING reason=participant_disconnected"],
            Duration::from_secs(10),
            mark4,
        )?;
        let kicked = client_b.wait_line(&["MP_CLIENT_KICKED"], Duration::from_secs(15), mark_b4)?;
        let removed = client_b.wait_line(
            &["MP_CLIENT_NETWORK_FAILURE"],
            Duration::from_secs(20),
            mark_b4,
        )?;
        client_b.wait_line(
            &["LoadMap: /Game/Maps/MainMenu"],
            Duration::from_secs(25),
            mark_b4,
        )?;
        let awaiting3 = server.wait_line(
            &["MP_MATCH_AWAITING_PARTICIPANTS"],
            Duration::from_secs(60),
            mark4,
        )?;
        let generation3 = field(&awaiting3, "generation").unwrap_or("0").to_owned();
        thread::sleep(Duration::from_secs(1));
        report.artifact(
            "client_b_removed_menu_png",
            &client_b.screenshot(evidence, "07-client-a-lost-menu")?,
        );
        let commits2 = count_lines(
            &server.text(),
            &["MATCH_RESULTS", &format!("generation={generation2}")],
        );
        report.verdict(
            "MP3-G6-CLIENT-LOSS",
            commits2 == 1
                && field(&closing2, "remaining") == Some("1")
                && field(&removed, "kicked") == Some("1")
                && server.alive()
                && generation3.parse::<u64>().unwrap_or(0) > generation2.parse::<u64>().unwrap_or(u64::MAX),
            format!("client A SIGKILL detected in {:.1}s: {}; {}; {}; client B: {} / {}; server reset to {}",
                detected.as_secs_f64(), unbound.trim(), ended.trim(), closing2.trim(), kicked.trim(), removed.trim(), awaiting3.trim()),
        );

        // Join order, not process identity, selects seats: client B (rematch attempt 2)
        // now takes seat 0; a fresh client C takes seat 1. Then the server is killed.
        let rebound_b3 = server.wait_line(
            &[
                "MP_PARTICIPANT_BOUND participant=1",
                &format!("generation={generation3}"),
            ],
            Duration::from_secs(60),
            mark4,
        )?;
        let display_c = Display::start(evidence, "client-c")?;
        let mut client_c = Game::launch(
            &client_options.binary,
            "client-c",
            evidence,
            &client_args(client_options, true, false, false),
            Some(&display_c.name()),
        )?;
        report.fact("client_c_pid", client_c.pid().to_string());
        let bound_c = server.wait_line(
            &[
                "MP_PARTICIPANT_BOUND participant=2",
                &format!("generation={generation3}"),
            ],
            Duration::from_secs(60),
            mark4,
        )?;
        server.wait_line(
            &["MP_MATCH_ACTIVE", &format!("generation={generation3}")],
            Duration::from_secs(10),
            mark4,
        )?;
        client_c.wait(&["MP_CLIENT_MATCH_OBSERVED"], Duration::from_secs(30))?;
        let mark_b5 = client_b.offset();
        client_b.wait_line(
            &["MP_CLIENT_MATCH_OBSERVED"],
            Duration::from_secs(30),
            mark_b4,
        )?;
        client_c.attach_window()?;
        thread::sleep(Duration::from_secs(2));
        report.verdict(
            "MP3-SEAT-BY-JOIN-ORDER",
            true,
            format!(
                "generation {generation3}: {}; {}",
                rebound_b3.trim(),
                bound_c.trim()
            ),
        );
        let mark_c = client_c.offset();
        let killed_at = Instant::now();
        server.kill_hard();
        let mut lost = Vec::new();
        for (client, mark) in [(&mut client_b, mark_b5), (&mut client_c, mark_c)] {
            let failure = client.wait_line(
                &["MP_CLIENT_NETWORK_FAILURE"],
                Duration::from_secs(30),
                mark,
            )?;
            client.wait_line(
                &["LoadMap: /Game/Maps/MainMenu"],
                Duration::from_secs(30),
                mark,
            )?;
            let explained = client.text()[mark..].contains("NetworkConnectionLost");
            lost.push((
                field(&failure, "result_observed") == Some("0") && explained,
                failure.trim().to_owned(),
            ));
        }
        let detected = killed_at.elapsed();
        thread::sleep(Duration::from_secs(2));
        report.artifact(
            "client_c_after_server_loss_png",
            &client_c.screenshot(evidence, "08-server-lost-menu")?,
        );
        report.artifact(
            "client_b_after_server_loss_png",
            &client_b.screenshot(evidence, "08-server-lost-menu")?,
        );
        report.verdict(
            "MP3-G6-SERVER-LOSS",
            lost.iter().all(|(ok, _)| *ok) && detected < Duration::from_secs(25),
            format!("server SIGKILL: both clients recovered with NetworkConnectionLost within {:.1}s: {lost:?}", detected.as_secs_f64()),
        );
        report.artifact("client_c_log", &client_c.log);
        client_c.stop();
        Ok(())
    })();
    match recorder_a.finish() {
        Ok(path) => report.artifact("client_a_mp4", &path),
        Err(error) => report.mark("EVIDENCE-CLIENT-A-MP4", "INFRA_ERROR", error),
    }
    match recorder_b.finish() {
        Ok(path) => report.artifact("client_b_mp4", &path),
        Err(error) => report.mark("EVIDENCE-CLIENT-B-MP4", "INFRA_ERROR", error),
    }
    launch_options_verdict(
        report,
        "MP3-DEV-LAUNCH-OPTIONS",
        &[&server, &client_a, &client_b],
    );
    report.artifact("server_log", &server.log);
    report.artifact("client_a_log", &client_a.log);
    report.artifact("client_b_log", &client_b.log);
    server.stop();
    client_a.stop();
    client_b.stop();

    // No local profile, save data or settings were written by the server process.
    let user_files = files_under(&evidence.join("user-server"));
    let home_files = files_under(&evidence.join("home-server"));
    let profile_files: Vec<String> = user_files
        .iter()
        .chain(home_files.iter())
        .map(|p| p.display().to_string())
        .filter(|p| p.contains("SaveGames") || p.ends_with(".sav"))
        .collect();
    let server_text = server.text();
    let forbidden = [
        "{\"event\":\"profile\"",
        "{\"event\":\"ui_ready\"",
        "LoadMap: /Game/Maps/MainMenu",
        "LogAudioMixer",
        "Initializing audio mixer",
    ];
    let present: Vec<&str> = forbidden
        .iter()
        .copied()
        .filter(|m| server_text.contains(m))
        .collect();
    // The engine's own default config files (e.g. GameUserSettings.ini written at exit) are
    // recorded, not treated as a profile: the server booted with an empty HOME/UserDir.
    report.fact("server_user_files", format!("{user_files:?}"));
    report.fact("server_home_files", format!("{home_files:?}"));
    let audio_disabled = server_text.contains("Audio Device Manager not initializing");
    report.verdict(
        "MP3-NO-PROFILE-UI-AUDIO",
        profile_files.is_empty() && present.is_empty() && audio_disabled,
        format!("booted from an empty HOME/UserDir; player profile/save files={profile_files:?}; shell/profile/audio markers in server log={present:?}; audio device manager disabled={audio_disabled}"),
    );
    result
}

fn dedicated_admission_failure(
    options: &DedicatedOptions,
    report: &mut Report,
) -> Result<(), String> {
    // An unsupported layout must fail closed: no units, no listening session, exit code 3.
    let evidence = &options.client.evidence;
    let mut server = Game::launch_server(
        &options.server,
        "server-invalid-layout",
        evidence,
        &server_args(options.client.port + 2, "5v5"),
    )?;
    // The process is expected to exit by itself; inspect its log only after it has.
    let start = Instant::now();
    let mut status = None;
    while start.elapsed() < Duration::from_secs(150) {
        if let Ok(Some(code)) = server.child.try_wait() {
            status = Some(code);
            break;
        }
        thread::sleep(Duration::from_millis(200));
    }
    let text = server.text();
    let failed = text
        .lines()
        .find(|l| l.contains("MP_DEDICATED_ADMISSION_FAILED"))
        .unwrap_or("no MP_DEDICATED_ADMISSION_FAILED line")
        .to_owned();
    let reason = text.contains("reason=MP.LAYOUT.INVALID");
    let no_session =
        !text.contains("MP_MATCH_AWAITING_PARTICIPANTS") && !text.contains("MATCH_CONTENT_READY");
    report.verdict(
        "MP3-ADMISSION-FAIL-CLOSED",
        status.and_then(|s| s.code()) == Some(3)
            && text.contains("MP_DEDICATED_ADMISSION_FAILED")
            && reason
            && no_session,
        format!(
            "{}; exit={status:?}; MP.LAYOUT.INVALID={reason}; no session/units={no_session}",
            failed.trim()
        ),
    );
    report.artifact("invalid_layout_server_log", &server.log);
    server.stop();
    Ok(())
}

fn dedicated_parity(options: &DedicatedOptions, report: &mut Report) -> Result<(), String> {
    // The same build, fixture and layout hosted by a listen server must admit the identical
    // authoritative setup (units, archetypes, owners, factions, slots, locations, weapons,
    // objective, Unaligned side, seat roster and teams) as the dedicated server did.
    let evidence = &options.client.evidence;
    let port = options.client.port + 4;
    let mut listen_args = vec![
        "/Game/Maps/RTSVerticalSlice?listen?PolitikaLayout=1v1".to_string(),
        format!("-port={port}"),
        "-PolitikaTestFixture=MultiplayerSkirmish".into(),
        "-RTSAcceptance=multiplayer".into(),
    ];
    listen_args.extend(["-nullrhi".into(), "-nosound".into()]);
    listen_args.extend(pinned_forces());
    let mut listen = Game::launch(
        &options.client.binary,
        "parity-listen",
        evidence,
        &listen_args,
        None,
    )?;
    listen.wait(&["MATCH_CONTENT_READY"], Duration::from_secs(120))?;
    listen.wait(&["MP_SESSION_LAYOUT"], Duration::from_secs(5))?;
    thread::sleep(Duration::from_secs(1));
    let listen_signature = setup_signature(&listen.text());
    listen.stop();
    let dedicated_text =
        fs::read_to_string(evidence.join("server.log")).map_err(|e| e.to_string())?;
    let dedicated_signature = setup_signature(&dedicated_text);
    let units = listen_signature
        .iter()
        .filter(|l| l.starts_with("MATCH_AUTHORITATIVE_UNIT"))
        .count();
    fs::write(
        evidence.join("parity-listen-signature.txt"),
        listen_signature.join("\n"),
    )
    .map_err(|e| e.to_string())?;
    fs::write(
        evidence.join("parity-dedicated-signature.txt"),
        dedicated_signature.join("\n"),
    )
    .map_err(|e| e.to_string())?;
    let differing: Vec<String> = listen_signature
        .iter()
        .zip(dedicated_signature.iter())
        .filter(|(l, d)| l != d)
        .map(|(l, d)| format!("listen `{l}` != dedicated `{d}`"))
        .collect();
    report.verdict(
        "MP3-PARITY-SETUP",
        units >= 4 && listen_signature.len() == dedicated_signature.len() && differing.is_empty(),
        format!("{} setup lines ({units} units) compared; listen={} dedicated={}; differences={differing:?}",
            listen_signature.len(), listen_signature.len(), dedicated_signature.len()),
    );
    report.artifact("parity_listen_log", &listen.log);
    Ok(())
}

fn run_dedicated() -> ! {
    let mut options = match parse_dedicated_options() {
        Ok(options) => options,
        Err(error) => {
            eprintln!("{error}");
            std::process::exit(2);
        }
    };
    // Game processes change directory at startup; every log path must be absolute.
    match fs::create_dir_all(&options.client.evidence)
        .and_then(|_| options.client.evidence.canonicalize())
    {
        Ok(absolute) => options.client.evidence = absolute,
        Err(error) => {
            eprintln!("evidence directory: {error}");
            std::process::exit(2);
        }
    }
    let evidence = options.client.evidence.clone();
    let mut report = Report {
        schema: Some("politika.mp3.dedicated-evidence.v1"),
        ..Report::default()
    };
    for (name, binary) in [
        ("server", &options.server),
        ("client", &options.client.binary),
    ] {
        report.fact(&format!("{name}_binary"), binary.display().to_string());
        if let Ok(digest) = sha256(binary) {
            report.fact(&format!("{name}_executable_sha256"), digest);
        }
        if let Some(archive) = binary.ancestors().nth(4) {
            if let Ok(identity) = fs::read_to_string(archive.join("BuildIdentity.json")) {
                report.fact(
                    &format!("{name}_build_identity"),
                    identity.trim().to_owned(),
                );
            }
            for file in files_under(&archive.join("Politika/Content/Paks")) {
                if file.extension().is_some_and(|e| e == "utoc" || e == "ucas")
                    && let Ok(digest) = sha256(&file)
                {
                    report.fact(
                        &format!(
                            "{name}:{}",
                            file.file_name().unwrap_or_default().to_string_lossy()
                        ),
                        digest,
                    );
                }
            }
        }
    }
    report.fact("layout", "1v1");
    report.fact("port", options.client.port.to_string());
    for (session, run) in [
        (
            "dedicated-gameplay",
            dedicated_gameplay as fn(&DedicatedOptions, &mut Report) -> Result<(), String>,
        ),
        ("admission-failure", dedicated_admission_failure),
        ("parity", dedicated_parity),
    ] {
        if let Err(error) = run(&options, &mut report) {
            report.mark(
                &format!("SESSION-{session}"),
                if error.starts_with("BLOCKED") {
                    "BLOCKED"
                } else {
                    "FAIL"
                },
                error,
            );
        }
    }
    let path = evidence.join("dedicated-evidence.json");
    if let Err(error) = report.write(&path) {
        eprintln!("write evidence: {error}");
        std::process::exit(2);
    }
    println!("evidence: {}", path.display());
    std::process::exit(if report.failed() { 1 } else { 0 });
}

// ----------------------------------------------------------------------------
// MP-2: packaged 3-4 client dedicated layouts (2v1 / 3v1 / 2v2)
// ----------------------------------------------------------------------------

/// POLITIKA's on-wire compatibility counter from the source tree this harness belongs to.
fn network_protocol() -> String {
    let text = include_str!(concat!(env!("CARGO_MANIFEST_DIR"), "/../../version.json"));
    text.lines()
        .find(|l| l.contains("\"network_protocol\""))
        .and_then(|l| l.split(':').nth(1))
        .map(|v| v.trim().trim_end_matches(',').trim().to_owned())
        .unwrap_or_else(|| "?".into())
}

struct LayoutOptions {
    /// Listen hosting: the rendered `client` binary hosts and plays seat 1 itself, and
    /// `server` names the same binary.
    listen: bool,
    server: PathBuf,
    client: PathBuf,
    legacy_client: Option<PathBuf>,
    layouts: Vec<String>,
    profile: &'static NetProfile,
    evidence: PathBuf,
    port: u16,
}

/// Engine packet emulation (UE `FPacketSimulationSettings`, non-Shipping only) applied to
/// the server and every client, so both directions of each connection are affected.
/// The engine gives `PktOrder` precedence over `PktLag`/`PktDup` and refuses `PktDup` with
/// lag, so reordering, duplication and latency are separate profiles.
struct NetProfile {
    name: &'static str,
    args: &'static [&'static str],
    /// Expected `MP_NET_EMULATION` values: loss, order, dup, lag, lag_variance.
    expected: [i64; 5],
}

const NET_PROFILES: [NetProfile; 4] = [
    NetProfile {
        name: "none",
        args: &[],
        expected: [0, 0, 0, 0, 0],
    },
    NetProfile {
        name: "lag-loss",
        args: &["-PktLag=60", "-PktLagVariance=20", "-PktLoss=3"],
        expected: [3, 0, 0, 60, 20],
    },
    NetProfile {
        name: "reorder",
        args: &["-PktOrder=1", "-PktLoss=3"],
        expected: [3, 1, 0, 0, 0],
    },
    NetProfile {
        name: "duplicate",
        args: &["-PktDup=10", "-PktLoss=3"],
        expected: [3, 0, 10, 0, 0],
    },
];

fn parse_layout_options(listen: bool) -> Result<LayoutOptions, String> {
    let usage = "usage: rts_multiplayer --dedicated-layouts SERVER_BINARY CLIENT_BINARY | \
                 --listen-layouts CLIENT_BINARY \
                 [--layouts 1v1,2v1,3v1,2v2] [--net-profile none|lag-loss|reorder|duplicate] \
                 [--legacy-client OLD_PROTOCOL_CLIENT] [--evidence DIR] [--port N]";
    let mut args = env::args().skip(2);
    let server = PathBuf::from(args.next().ok_or(usage)?)
        .canonicalize()
        .map_err(|e| format!("server binary: {e}"))?;
    let client = if listen {
        server.clone()
    } else {
        PathBuf::from(args.next().ok_or(usage)?)
            .canonicalize()
            .map_err(|e| format!("client binary: {e}"))?
    };
    let mut layouts = vec!["2v1".to_owned(), "3v1".to_owned(), "2v2".to_owned()];
    let mut legacy_client = None;
    let mut evidence = None;
    let mut port = 17860u16;
    let mut profile = &NET_PROFILES[0];
    while let Some(flag) = args.next() {
        let value = args.next().ok_or_else(|| format!("{flag} needs a value"))?;
        match flag.as_str() {
            "--layouts" => layouts = value.split(',').map(str::to_owned).collect(),
            "--net-profile" => {
                profile = NET_PROFILES
                    .iter()
                    .find(|p| p.name == value)
                    .ok_or_else(|| format!("unknown --net-profile {value}"))?
            }
            "--legacy-client" => {
                legacy_client = Some(
                    PathBuf::from(value)
                        .canonicalize()
                        .map_err(|e| format!("legacy client: {e}"))?,
                )
            }
            "--evidence" => evidence = Some(PathBuf::from(value)),
            "--port" => port = value.parse().map_err(|e| format!("--port: {e}"))?,
            other => return Err(format!("unknown option {other}")),
        }
    }
    if let Some(bad) = layouts.iter().find(|l| layout_teams(l).is_none()) {
        return Err(format!(
            "unknown layout {bad}; expected 1v1, 2v1, 3v1 or 2v2"
        ));
    }
    let epoch = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .map_err(|e| e.to_string())?
        .as_secs();
    let evidence = evidence.unwrap_or_else(|| {
        PathBuf::from(format!(
            "build/diagnostics/mp2-{}/packaged-{epoch}",
            if listen {
                "listen-layouts"
            } else {
                "multiclient"
            }
        ))
    });
    Ok(LayoutOptions {
        listen,
        server,
        client,
        legacy_client,
        layouts,
        profile,
        evidence,
        port,
    })
}

/// Team of each seat in join order (politika::rules::TeamForSeat): 1 = team A, 2 = team B.
fn layout_teams(layout: &str) -> Option<&'static [u32]> {
    Some(match layout {
        "1v1" => &[1, 2],
        "2v1" => &[1, 2, 1],
        "3v1" => &[1, 2, 1, 1],
        "2v2" => &[1, 2, 1, 2],
        _ => return None,
    })
}

/// (entity, owner, observed team mask) for every unit the server sampled at activation.
fn activation_samples(server_text: &str, generation: &str) -> Vec<(u64, u32, u32)> {
    let mut samples: Vec<(u64, u32, u32)> = server_text
        .lines()
        .filter(|l| l.contains("MP_SERVER_SAMPLE reason=activated"))
        .filter(|l| field(l, "generation") == Some(generation))
        .filter_map(|l| {
            Some((
                field(l, "entity")?.parse().ok()?,
                field(l, "owner")?.parse().ok()?,
                field(l, "observed")?.parse().ok()?,
            ))
        })
        .collect();
    samples.sort_unstable();
    samples.dedup();
    samples
}

fn relevant_entities(client_text: &str) -> Vec<(u64, u32)> {
    let mut relevant: Vec<(u64, u32)> = client_text
        .lines()
        .filter(|l| l.contains("MP_CLIENT_UNIT_RELEVANT"))
        .filter_map(|l| {
            Some((
                field(l, "entity")?.parse().ok()?,
                field(l, "owner")?.parse().ok()?,
            ))
        })
        .collect();
    relevant.sort_unstable();
    relevant.dedup();
    relevant
}

/// Every damaged health value and death a client observed has a server record. Returns
/// (observed states, deaths, problems).
fn health_records(host: &Game, client: &Game) -> (usize, usize, Vec<String>) {
    let host_text = host.text();
    let client_text = client.text();
    let mut problems = Vec::new();
    let mut observed = 0;
    for line in client_text
        .lines()
        .filter(|l| l.contains("MP_CLIENT_UNIT_HEALTH"))
    {
        let (Some(entity), Some(health)) = (field(line, "entity"), field(line, "health")) else {
            continue;
        };
        if field(line, "max").is_some_and(|max| max == health) {
            continue;
        }
        observed += 1;
        let server = host_text.lines().any(|l| {
            (l.contains("RTS_HEALTH_CHANGED")
                && field(l, "unit") == Some(entity)
                && field(l, "after") == Some(health))
                || (l.contains("MP_SERVER_SAMPLE")
                    && field(l, "entity") == Some(entity)
                    && field(l, "health") == Some(health))
        });
        if !server {
            problems.push(format!(
                "entity {entity} at health {health}: no server record"
            ));
        }
    }
    let mut deaths = 0;
    for line in client_text
        .lines()
        .filter(|l| l.contains("MP_CLIENT_UNIT_DESTROYED"))
    {
        let Some(entity) = field(line, "entity") else {
            continue;
        };
        deaths += 1;
        if !host_text
            .lines()
            .any(|l| l.contains("RTS_UNIT_DESTROYED") && field(l, "unit") == Some(entity))
        {
            problems.push(format!(
                "entity {entity} destroyed on client, not on server"
            ));
        }
    }
    (observed, deaths, problems)
}

/// Audit F27 over the real transport: every cue a client received matches the server's
/// decision for that participant, exact cues carry the true target point, facing cues do
/// not, and no participant received a cue the server decided it must not have.
/// One server shot-cue decision: sequence, participant, cue, target point, sent point.
type ShotDecision = (String, String, String, (f64, f64), (f64, f64));

fn shot_cue_consistency(server: &Game, clients: &[&Game]) -> (usize, [usize; 3], Vec<String>) {
    let server_text = server.text();
    let mut decisions: Vec<ShotDecision> = Vec::new();
    let mut counts = [0usize; 3]; // exact, facing, none
    let mut problems = Vec::new();
    for line in server_text
        .lines()
        .filter(|l| l.contains("MP_SHOT_CUE_SENT"))
    {
        let (Some(sequence), Some(participant), Some(cue)) = (
            field(line, "sequence"),
            field(line, "participant"),
            field(line, "cue"),
        ) else {
            continue;
        };
        let target = (
            number(line, "target_x").unwrap_or(f64::NAN),
            number(line, "target_y").unwrap_or(f64::NAN),
        );
        let point = (
            number(line, "point_x").unwrap_or(f64::NAN),
            number(line, "point_y").unwrap_or(f64::NAN),
        );
        let offset = ((target.0 - point.0).powi(2) + (target.1 - point.1).powi(2)).sqrt();
        match cue {
            "exact" => {
                counts[0] += 1;
                if offset > 1.0 {
                    problems.push(format!(
                        "{sequence}/{participant}: exact cue {offset:.1}cm off target"
                    ));
                }
            }
            "facing" => {
                counts[1] += 1;
                if offset.is_nan() || offset <= 100.0 {
                    problems.push(format!(
                        "{sequence}/{participant}: facing cue lies on the target"
                    ));
                }
            }
            "none" => counts[2] += 1,
            other => problems.push(format!("{sequence}/{participant}: unknown cue {other}")),
        }
        decisions.push((
            sequence.to_owned(),
            participant.to_owned(),
            cue.to_owned(),
            target,
            point,
        ));
    }
    let mut received = 0;
    for client in clients {
        for line in client
            .text()
            .lines()
            .filter(|l| l.contains("MP_CLIENT_SHOT_CUE"))
        {
            let (Some(sequence), Some(participant), Some(exact)) = (
                field(line, "sequence"),
                field(line, "participant"),
                field(line, "exact"),
            ) else {
                continue;
            };
            received += 1;
            let point = (
                number(line, "point_x").unwrap_or(f64::NAN),
                number(line, "point_y").unwrap_or(f64::NAN),
            );
            let Some(decision) = decisions
                .iter()
                .find(|d| d.0 == sequence && d.1 == participant)
            else {
                problems.push(format!(
                    "{}: received cue {sequence} with no server decision for participant {participant}",
                    client.label
                ));
                continue;
            };
            let expected_exact = if decision.2 == "exact" { "1" } else { "0" };
            if decision.2 == "none" {
                problems.push(format!(
                    "{}: received cue {sequence} the server withheld",
                    client.label
                ));
            } else if exact != expected_exact {
                problems.push(format!(
                    "{}: cue {sequence} exact={exact}, server decided {}",
                    client.label, decision.2
                ));
            }
            // FVector_NetQuantize rounds to whole centimetres.
            let drift =
                ((point.0 - decision.4.0).powi(2) + (point.1 - decision.4.1).powi(2)).sqrt();
            if drift > 1.5 {
                problems.push(format!(
                    "{}: cue {sequence} point drifted {drift:.1}cm",
                    client.label
                ));
            }
            if decision.2 != "exact" {
                let leak =
                    ((point.0 - decision.3.0).powi(2) + (point.1 - decision.3.1).powi(2)).sqrt();
                if leak < 100.0 {
                    problems.push(format!(
                        "{}: cue {sequence} reveals the unseen target",
                        client.label
                    ));
                }
            }
        }
    }
    (received, counts, problems)
}

/// One engine traffic sample (`MP_NET_STATS`) for a connection in one match generation.
struct NetSample {
    reason: String,
    participant: u32,
    elapsed: f64,
    counters: Vec<(String, f64)>,
}

impl NetSample {
    fn get(&self, key: &str) -> f64 {
        self.counters
            .iter()
            .find(|(k, _)| k == key)
            .map_or(0.0, |(_, v)| *v)
    }
}

const NET_COUNTERS: [&str; 12] = [
    "in_bytes",
    "out_bytes",
    "in_packets",
    "out_packets",
    "in_lost",
    "out_lost",
    "ooo_recovered",
    "ooo_lost",
    "ooo_duplicate",
    "avg_lag_ms",
    "jitter_ms",
    "visible_units",
];

fn net_samples(text: &str, role: &str, generation: &str) -> Vec<NetSample> {
    text.lines()
        .filter(|l| {
            l.contains("MP_NET_STATS")
                && field(l, "role") == Some(role)
                && field(l, "generation") == Some(generation)
        })
        .filter_map(|l| {
            Some(NetSample {
                reason: field(l, "reason")?.to_owned(),
                participant: field(l, "participant")?.parse().ok()?,
                elapsed: number(l, "elapsed_s")?,
                counters: NET_COUNTERS
                    .iter()
                    .map(|k| ((*k).to_owned(), number(l, k).unwrap_or(0.0)))
                    .collect(),
            })
        })
        .collect()
}

/// F36 and network-condition verdicts for one layout run: the emulation each process
/// applied, exactly-once ordered command intake, per-connection bandwidth from the
/// engine's own counters, observed impairment, and authoritative step cost.
/// `clients` are the remote clients in seat order; the first is participant `first`
/// (1 on a dedicated server, 2 when a listen host plays seat 1).
#[allow(clippy::too_many_arguments)]
fn network_checks(
    report: &mut Report,
    id: &dyn Fn(&str) -> String,
    layout: &str,
    profile: &NetProfile,
    server: &Game,
    clients: &[Game],
    first: u32,
    generation: &str,
) {
    let seats = clients.len() + first as usize - 1;
    let server_text = server.text();
    let key = |name: &str| {
        if profile.name == "none" {
            format!("{layout}_{name}")
        } else {
            format!("{layout}_{}_{name}", profile.name)
        }
    };

    // 1. Every process runs under the requested engine emulation, and only that.
    let mut emulation_ok = true;
    let mut emulation = Vec::new();
    for (label, text) in std::iter::once((server.label.as_str(), server_text.clone()))
        .chain(clients.iter().map(|c| (c.label.as_str(), c.text())))
    {
        let line = text.lines().find(|l| l.contains("MP_NET_EMULATION"));
        let applied: Option<Vec<i64>> = line.and_then(|l| {
            (field(l, "available") == Some("1")).then(|| {
                ["loss", "order", "dup", "lag", "lag_variance"]
                    .iter()
                    .map(|k| field(l, k).and_then(|v| v.parse().ok()).unwrap_or(-1))
                    .collect()
            })
        });
        emulation_ok &= applied.as_deref() == Some(&profile.expected[..]);
        emulation.push(format!("{label}: {}", line.map_or("missing", str::trim)));
    }
    report.verdict(
        &id("NET-EMULATION-APPLIED"),
        emulation_ok,
        format!(
            "profile {} {:?} expects loss/order/dup/lag/variance {:?}; {emulation:?}",
            profile.name, profile.args, profile.expected
        ),
    );

    // 2. Reliable intake: the server sees every command a client sent exactly once, in send
    // order, and never rejects an honest command as a duplicate or for rate.
    let mut intake_problems = Vec::new();
    let mut commands = 0;
    for (index, client) in clients.iter().enumerate() {
        let participant = (index as u32 + first).to_string();
        let text = client.text();
        let sent: Vec<&str> = text
            .lines()
            .filter(|l| l.contains("RTS_COMMAND_INTENT_SENT") || l.contains("MP_PROBE_SENT"))
            .filter_map(|l| field(l, "correlation"))
            .collect();
        let honest: Vec<&str> = text
            .lines()
            .filter(|l| l.contains("RTS_COMMAND_INTENT_SENT"))
            .filter_map(|l| field(l, "correlation"))
            .collect();
        let arrived: Vec<&str> = server_text
            .lines()
            .filter(|l| {
                (l.contains("MP_COMMAND_RECEIVED") || l.contains("MP_COMMAND_REJECTED_AT_INTAKE"))
                    && field(l, "participant") == Some(participant.as_str())
            })
            .filter_map(|l| field(l, "correlation"))
            .collect();
        commands += sent.len();
        if sent != arrived {
            intake_problems.push(format!(
                "{}: sent {} commands, server took {} (sent {sent:?}, arrived {arrived:?})",
                client.label,
                sent.len(),
                arrived.len()
            ));
        }
        for correlation in &sent {
            let outcomes = text
                .lines()
                .filter(|l| {
                    l.contains("MP_COMMAND_OUTCOME")
                        && field(l, "correlation") == Some(*correlation)
                })
                .collect::<Vec<_>>();
            if outcomes.len() != 1 {
                intake_problems.push(format!(
                    "{}: correlation {correlation} has {} outcomes",
                    client.label,
                    outcomes.len()
                ));
            } else if honest.contains(correlation)
                && matches!(field(outcomes[0], "rejection"), Some("16") | Some("17"))
            {
                intake_problems.push(format!(
                    "{}: honest command {correlation} rejected as duplicate/rate: {}",
                    client.label,
                    outcomes[0].trim()
                ));
            }
        }
    }
    report.verdict(
        &id("INTAKE-EXACTLY-ONCE-ORDERED"),
        intake_problems.is_empty() && commands > 0,
        format!(
            "{commands} client commands (real input and probes) over ServerSubmitCommandIntent; problems={intake_problems:?}"
        ),
    );

    // 3. Bandwidth per connection over the active match, from the engine's counters.
    let samples = net_samples(&server_text, "server", generation);
    let mut bandwidth_ok = !samples.is_empty();
    let mut rows = Vec::new();
    let mut impairment = [0.0f64; 3]; // lost, out-of-order (all), duplicate
    for participant in first..=seats as u32 {
        let base = samples
            .iter()
            .find(|s| s.participant == participant && s.reason == "activated");
        let last = samples.iter().rfind(|s| {
            s.participant == participant && s.reason != "periodic" && s.reason != "activated"
        });
        let (Some(base), Some(last)) = (base, last) else {
            bandwidth_ok = false;
            rows.push(format!("p{participant}: missing activated/final sample"));
            continue;
        };
        let seconds = last.elapsed - base.elapsed;
        let rate = |k: &str| (last.get(k) - base.get(k)) / seconds.max(f64::EPSILON);
        let (down, up) = (rate("out_bytes"), rate("in_bytes"));
        bandwidth_ok &= seconds > 5.0 && down > 0.0 && up > 0.0;
        impairment[0] +=
            last.get("out_lost") - base.get("out_lost") + last.get("in_lost") - base.get("in_lost");
        impairment[1] +=
            last.get("ooo_recovered") + last.get("ooo_lost") + last.get("ooo_duplicate")
                - base.get("ooo_recovered")
                - base.get("ooo_lost")
                - base.get("ooo_duplicate");
        impairment[2] += last.get("ooo_duplicate") - base.get("ooo_duplicate");
        let client_lag = clients
            .get((participant - first) as usize)
            .and_then(|c| {
                net_samples(&c.text(), "client", generation)
                    .into_iter()
                    .rev()
                    .find(|s| s.get("avg_lag_ms") > 0.0)
            })
            .map_or(-1.0, |s| s.get("avg_lag_ms"));
        rows.push(format!(
            "p{participant} {seconds:.1}s down={down:.0}B/s ({:.1}pkt/s) up={up:.0}B/s ({:.1}pkt/s) server_rtt={:.1}ms client_rtt={client_lag:.1}ms visible_at_end={}",
            rate("out_packets"),
            rate("in_packets"),
            last.get("avg_lag_ms"),
            last.get("visible_units")
        ));
    }
    // Clients count their own receive-side impairment of server traffic.
    for client in clients {
        let client_samples = net_samples(&client.text(), "client", generation);
        if let (Some(first), Some(last)) = (client_samples.first(), client_samples.last()) {
            impairment[0] += last.get("in_lost") - first.get("in_lost");
            impairment[1] +=
                last.get("ooo_recovered") + last.get("ooo_lost") + last.get("ooo_duplicate")
                    - first.get("ooo_recovered")
                    - first.get("ooo_lost")
                    - first.get("ooo_duplicate");
            impairment[2] += last.get("ooo_duplicate") - first.get("ooo_duplicate");
        }
    }
    report.fact(&key("bandwidth"), rows.join("; "));
    report.verdict(
        &id("NET-BANDWIDTH-MEASURED"),
        bandwidth_ok,
        format!("engine counters, activation to result, per connection: {rows:?}"),
    );

    // 4. The impairment actually happened (the engine counted it), not just configured.
    let observed = format!(
        "packets lost={} out-of-order={} duplicate={}",
        impairment[0], impairment[1], impairment[2]
    );
    report.fact(&key("impairment"), observed.clone());
    let expected_seen = match profile.name {
        "lag-loss" => impairment[0] > 0.0,
        "reorder" => impairment[1] > 0.0,
        "duplicate" => impairment[2] > 0.0,
        _ => true,
    };
    if profile.name != "none" {
        if expected_seen {
            report.verdict(&id("NET-IMPAIRMENT-OBSERVED"), true, observed);
        } else {
            report.mark(
                &id("NET-IMPAIRMENT-OBSERVED"),
                "INCONCLUSIVE",
                format!("configured but not counted by the engine: {observed}"),
            );
        }
    }

    // 5. Authoritative step cost against the fixed-step budget.
    let summary = server_text.lines().find(|l| {
        l.contains("MP_SERVER_STEP_SUMMARY") && field(l, "generation") == Some(generation)
    });
    let step_ok = summary.is_some_and(|l| {
        field(l, "reason") == Some("result")
            && field(l, "seats") == Some(seats.to_string().as_str())
            && number(l, "steps").is_some_and(|s| s > 0.0)
            && match (number(l, "p99_us"), number(l, "budget_us")) {
                (Some(p99), Some(budget)) => p99 < budget,
                _ => false,
            }
    });
    report.fact(&key("server_step"), summary.map_or("missing", str::trim));
    report.verdict(
        &id("SERVER-STEP-WITHIN-BUDGET"),
        step_ok,
        summary.map_or("no MP_SERVER_STEP_SUMMARY".into(), |l| l.trim().to_owned()),
    );
}

/// The process that renders and drives seat `index` (0-based): the listen host plays seat 0
/// itself; every other seat, and every seat on a dedicated server, is a remote client.
fn seat_game<'a>(
    server: &'a mut Game,
    clients: &'a mut [Game],
    listen: bool,
    index: usize,
) -> &'a mut Game {
    match (listen, index) {
        (true, 0) => server,
        (true, _) => &mut clients[index - 1],
        (false, _) => &mut clients[index],
    }
}

/// Log markers carrying a seat's on-screen unit and objective positions.
fn seat_markers(listen: bool, index: usize) -> (&'static str, &'static str) {
    if listen && index == 0 {
        ("MP_SERVER_SAMPLE", "MP_SERVER_OBJECTIVE")
    } else {
        ("MP_CLIENT_SAMPLE", "MP_CLIENT_OBJECTIVE")
    }
}

fn listen_host_args(port: u16, layout: &str) -> Vec<String> {
    let mut args = vec![
        format!("/Game/Maps/RTSVerticalSlice?listen?PolitikaLayout={layout}"),
        format!("-port={port}"),
        "-PolitikaTestFixture=MultiplayerSkirmish".into(),
        "-RTSAcceptance=multiplayer".into(),
        "-windowed".into(),
        format!("-ResX={WIDTH}"),
        format!("-ResY={HEIGHT}"),
    ];
    args.extend(pinned_forces());
    args
}

/// (entity, host_hidden) for every unit the listen host sampled at activation.
fn host_hidden_at_activation(server_text: &str, generation: &str) -> Vec<(u64, bool)> {
    let mut hidden: Vec<(u64, bool)> = server_text
        .lines()
        .filter(|l| l.contains("MP_SERVER_SAMPLE reason=activated"))
        .filter(|l| field(l, "generation") == Some(generation))
        .filter_map(|l| {
            Some((
                field(l, "entity")?.parse().ok()?,
                field(l, "host_hidden")? == "1",
            ))
        })
        .collect();
    hidden.sort_unstable();
    hidden.dedup();
    hidden
}

/// The listen host's own last-known contacts come only from its own fog transitions: each
/// follows a live `MP_HOST_UNIT_FOG hidden=1` of the same entity at the same position after
/// the host had seen it, and never names the host's own side.
/// Returns (recorded contacts, fog transitions, problems).
fn host_contact_audit(text: &str) -> (usize, usize, Vec<String>) {
    let lines: Vec<&str> = text.lines().collect();
    let transitions = lines
        .iter()
        .filter(|l| l.contains("MP_HOST_UNIT_FOG") && field(l, "first") == Some("0"))
        .count();
    let mut problems = Vec::new();
    let mut recorded = 0;
    for (index, line) in lines.iter().enumerate() {
        if !line.contains("MP_LOCAL_CONTACT_LAST_KNOWN") {
            continue;
        }
        recorded += 1;
        let entity = field(line, "entity").unwrap_or("?");
        if field(line, "role") != Some("host") {
            problems.push(format!(
                "contact {entity} not from the host view: {}",
                line.trim()
            ));
        }
        let fog = |l: &&&str| l.contains("MP_HOST_UNIT_FOG") && field(l, "entity") == Some(entity);
        let hidden = lines[..index].iter().rev().find(fog);
        let seen = lines[..index]
            .iter()
            .filter(fog)
            .any(|l| field(l, "hidden") == Some("0"));
        let close = |a: Option<f64>, b: Option<f64>| matches!((a, b), (Some(a), Some(b)) if (a - b).abs() < 0.6);
        match hidden {
            Some(h)
                if seen
                    && field(h, "hidden") == Some("1")
                    && field(h, "alive") == Some("1")
                    && close(number(h, "x"), number(line, "x"))
                    && close(number(h, "y"), number(line, "y")) => {}
            _ => problems.push(format!(
                "contact {entity} not preceded by a matching host fog withdrawal: {}",
                line.trim()
            )),
        }
        if field(line, "owner") == field(line, "viewer")
            || field(line, "team") == field(line, "viewer_team")
        {
            problems.push(format!(
                "contact names the host's own side: {}",
                line.trim()
            ));
        }
    }
    (recorded, transitions, problems)
}

/// The listen host draws each shot from its own team's vision: its cue for a shot equals
/// the server's decision for any allied remote participant (team vision is shared), exact
/// cues sit on the target, and facing cues stay away from it. Returns (host cues,
/// [exact, facing, none], cues compared with an ally, problems).
fn host_shot_cue_audit(
    server_text: &str,
    allies: &[u32],
) -> (usize, [usize; 3], usize, Vec<String>) {
    let mut counts = [0usize; 3];
    let mut compared = 0;
    let mut problems = Vec::new();
    let mut total = 0;
    for line in server_text
        .lines()
        .filter(|l| l.contains("MP_HOST_SHOT_CUE"))
    {
        total += 1;
        let (Some(sequence), Some(cue)) = (field(line, "sequence"), field(line, "cue")) else {
            problems.push(format!("malformed: {}", line.trim()));
            continue;
        };
        let offset = match (
            number(line, "target_x"),
            number(line, "target_y"),
            number(line, "point_x"),
            number(line, "point_y"),
        ) {
            (Some(tx), Some(ty), Some(px), Some(py)) => {
                ((tx - px).powi(2) + (ty - py).powi(2)).sqrt()
            }
            _ => f64::NAN,
        };
        match cue {
            "exact" => {
                counts[0] += 1;
                if !(offset <= 1.0) {
                    problems.push(format!(
                        "{sequence}: host exact cue {offset:.1}cm off target"
                    ));
                }
            }
            "facing" => {
                counts[1] += 1;
                if !(offset > 100.0) {
                    problems.push(format!("{sequence}: host facing cue lies on the target"));
                }
            }
            "none" => counts[2] += 1,
            other => problems.push(format!("{sequence}: unknown host cue {other}")),
        }
        for ally in allies {
            let sent = server_text.lines().find(|l| {
                l.contains("MP_SHOT_CUE_SENT")
                    && field(l, "sequence") == Some(sequence)
                    && field(l, "participant") == Some(ally.to_string().as_str())
            });
            match sent.and_then(|l| field(l, "cue")) {
                Some(ally_cue) if ally_cue == cue => compared += 1,
                Some(ally_cue) => problems.push(format!(
                    "{sequence}: host cue {cue}, allied participant {ally} cue {ally_cue}"
                )),
                None => problems.push(format!(
                    "{sequence}: no decision for allied participant {ally}"
                )),
            }
        }
    }
    (total, counts, compared, problems)
}

/// Every rendered process in seat order: the listen host (if any), then remote clients.
fn rendered_games<'a>(server: &'a Game, clients: &'a [Game], listen: bool) -> Vec<&'a Game> {
    listen
        .then_some(server)
        .into_iter()
        .chain(clients.iter())
        .collect()
}

fn layout_session(
    options: &LayoutOptions,
    layout: &str,
    port: u16,
    report: &mut Report,
) -> Result<(), String> {
    let teams = layout_teams(layout).ok_or("unknown layout")?;
    let seats = teams.len();
    let listen = options.listen;
    // Participant of the first remote client: the listen host is participant 1.
    let first: usize = if listen { 2 } else { 1 };
    let profile = options.profile;
    let suffix = if profile.name == "none" {
        String::new()
    } else {
        format!("-{}", profile.name.to_uppercase())
    };
    let mode = if listen { "LISTEN-" } else { "" };
    let id = |name: &str| format!("MP2-{mode}{}{suffix}-{name}", layout.to_uppercase());
    let evidence = options.evidence.join(format!(
        "{}-{layout}{}",
        if listen { "listen" } else { "layout" },
        suffix.to_lowercase()
    ));
    fs::create_dir_all(&evidence).map_err(|e| e.to_string())?;
    let client_options = Options {
        binary: options.client.clone(),
        layout: layout.into(),
        evidence: evidence.clone(),
        port,
    };

    let emulation: Vec<String> = profile.args.iter().map(|a| (*a).to_owned()).collect();
    let mut displays = Vec::new();
    let mut server = if listen {
        let display = Display::start(&evidence, "host")?;
        let mut args = listen_host_args(port, layout);
        args.extend(emulation.iter().cloned());
        let host = Game::launch(
            &options.server,
            "host",
            &evidence,
            &args,
            Some(&display.name()),
        )?;
        displays.push(display);
        host
    } else {
        let mut args = server_args(port, layout);
        args.extend(emulation.iter().cloned());
        Game::launch_server(&options.server, "server", &evidence, &args)?
    };
    report.fact(
        &format!("{layout}_{}_pid", server.label),
        server.pid().to_string(),
    );
    report.fact(
        &format!("{layout}_{}_command", server.label),
        server.command.join(" "),
    );
    let server_pid = server.pid();
    let ready = server.wait(
        &[if listen {
            "MP_LISTEN_SERVER_READY"
        } else {
            "MP_DEDICATED_SERVER_READY"
        }],
        Duration::from_secs(120),
    )?;
    let awaiting = server.wait(&["MP_MATCH_AWAITING_PARTICIPANTS"], Duration::from_secs(30))?;
    let generation = field(&awaiting, "generation").unwrap_or("0").to_owned();
    let session = server.wait(&["MP_SESSION_LAYOUT"], Duration::from_secs(5))?;
    let remote_seats = seats + 1 - first;
    let ready_ok = if listen {
        field(&ready, "port") == Some(port.to_string().as_str())
            && field(&ready, "net_driver") == Some("1")
    } else {
        field(&ready, "local_players") == Some("0") && field(&ready, "can_render") == Some("0")
    };
    report.verdict(
        &id("LAYOUT"),
        field(&session, "layout") == Some(layout)
            && field(&session, "seats") == Some(seats.to_string().as_str())
            && field(&session, "fog") == Some("1")
            && field(&session, "hosting") == Some(if listen { "listen" } else { "dedicated" })
            && field(&session, "remote_seats") == Some(remote_seats.to_string().as_str())
            && ready_ok,
        format!("{}; {}", ready.trim(), session.trim()),
    );

    // Clients join one at a time so the server binds seats in a known join order.
    let mut clients: Vec<Game> = Vec::new();
    let mut bindings = Vec::new();
    for seat in first - 1..seats {
        let participant = seat + 1;
        let label = format!("client-{participant}");
        let display = Display::start(&evidence, &label)?;
        // One probing client per team: participant 2 (team B) and 3 (team A, with an ally).
        let probe = participant == 2 || participant == 3;
        let mut args = client_args(&client_options, true, probe, false);
        args.extend(emulation.iter().cloned());
        let client = Game::launch(
            &options.client,
            &label,
            &evidence,
            &args,
            Some(&display.name()),
        )?;
        report.fact(&format!("{layout}_{label}_pid"), client.pid().to_string());
        report.fact(
            &format!("{layout}_{label}_command"),
            client.command.join(" "),
        );
        let bound = server.wait(
            &[
                &format!("MP_PARTICIPANT_BOUND participant={participant} "),
                "remote=1",
            ],
            Duration::from_secs(60),
        )?;
        bindings.push(bound);
        displays.push(display);
        clients.push(client);
    }
    let active = server.wait(
        &["MP_MATCH_ACTIVE", &format!("generation={generation}")],
        Duration::from_secs(30),
    )?;
    if listen {
        // The host binds its own (local) seat; it is always participant 1 on team A.
        let host_bound = server.wait(
            &["MP_PARTICIPANT_BOUND participant=1 ", "remote=0"],
            Duration::from_secs(10),
        )?;
        bindings.insert(0, host_bound);
    }
    let mut assignments = Vec::new();
    for (index, client) in clients.iter_mut().enumerate() {
        assignments.push(client.wait(
            &[&format!(
                "MP_CLIENT_ASSIGNMENT participant={} ",
                index + first
            )],
            Duration::from_secs(30),
        )?);
        client.wait(
            &[
                "MP_CLIENT_MATCH_OBSERVED",
                "map=map.test.multiplayer_skirmish",
            ],
            Duration::from_secs(30),
        )?;
    }
    let bind_ok = bindings.len() == seats
        && bindings
            .iter()
            .zip(teams)
            .enumerate()
            .all(|(index, (bound, team))| {
                let remote = if listen && index == 0 { "0" } else { "1" };
                field(bound, "participant") == Some((index + 1).to_string().as_str())
                    && field(bound, "team") == Some(team.to_string().as_str())
                    && field(bound, "remote") == Some(remote)
            })
        && assignments
            .iter()
            .all(|a| field(a, "generation") == Some(generation.as_str()));
    report.verdict(
        &id("SEATS-BY-JOIN-ORDER"),
        bind_ok,
        format!("expected teams {teams:?}; bound {bindings:?}; assigned {assignments:?}"),
    );
    let mut pids: Vec<u32> = clients.iter().map(Game::pid).collect();
    pids.push(server_pid);
    pids.sort_unstable();
    pids.dedup();
    // A dedicated server is one extra process; a listen host is itself seat 1.
    let processes = remote_seats + 1;
    report.verdict(
        &id(if listen {
            "ACTIVE-HOST-AND-REMOTE"
        } else {
            "ACTIVE-ALL-REMOTE"
        }),
        field(&active, "hosting") == Some(if listen { "listen" } else { "dedicated" })
            && field(&active, "local") == Some(if listen { "1" } else { "0" })
            && field(&active, "remote") == Some(remote_seats.to_string().as_str())
            && pids.len() == processes,
        format!(
            "{processes} independent processes {pids:?}: {}",
            active.trim()
        ),
    );

    if listen {
        server.attach_window()?;
    }
    for client in &mut clients {
        client.attach_window()?;
    }
    let mut recorders = Vec::new();
    for (display, game) in displays
        .iter()
        .zip(rendered_games(&server, &clients, listen))
    {
        recorders.push(Recorder::start(&display.name(), &evidence, &game.label)?);
    }
    let result = (|| -> Result<(), String> {
        thread::sleep(Duration::from_secs(3));
        for client in rendered_games(&server, &clients, listen) {
            report.artifact(
                &format!("{layout}_{}_active_png", client.label),
                &client.screenshot(&evidence, "01-active")?,
            );
        }

        // Fog: each client receives exactly the units its team observes at activation,
        // which always includes every allied participant's units (shared team vision).
        let samples = activation_samples(&server.text(), &generation);
        let team_of = |owner: u32| -> u32 {
            if owner == 0 {
                3
            } else {
                teams.get(owner as usize - 1).copied().unwrap_or(0)
            }
        };
        let mut fog_ok = !samples.is_empty();
        let mut allied_ok = true;
        let mut fog_detail = Vec::new();
        let mut cross_contact = Vec::new();
        let host_hidden = host_hidden_at_activation(&server.text(), &generation);
        for index in 0..seats {
            let participant = index as u32 + 1;
            let team = teams[index];
            let expected: Vec<u64> = samples
                .iter()
                .filter(|(_, _, mask)| mask & (1 << team) != 0)
                .map(|(entity, _, _)| *entity)
                .collect();
            // A remote client holds exactly the units replicated to it; the listen host holds
            // every unit and renders only those its fog presentation leaves unhidden.
            let received: Vec<u64> = if listen && index == 0 {
                fog_ok &= host_hidden.len() == samples.len();
                host_hidden
                    .iter()
                    .filter(|(_, hidden)| !hidden)
                    .map(|(entity, _)| *entity)
                    .collect()
            } else {
                relevant_entities(&clients[index + 1 - first].text())
                    .iter()
                    .map(|(entity, _)| *entity)
                    .collect()
            };
            fog_ok &= expected == received;
            let allies: Vec<u64> = samples
                .iter()
                .filter(|(_, owner, _)| {
                    *owner != participant && *owner != 0 && team_of(*owner) == team
                })
                .map(|(entity, _, _)| *entity)
                .collect();
            allied_ok &= allies.iter().all(|e| received.contains(e));
            cross_contact.extend(
                samples
                    .iter()
                    .filter(|(e, owner, _)| team_of(*owner) != team && received.contains(e))
                    .map(|(e, owner, _)| format!("p{participant} sees entity {e} (owner {owner})")),
            );
            fog_detail.push(format!(
                "p{participant} team {team}: expected {expected:?} received {received:?} allies {allies:?}"
            ));
        }
        report.fact(
            &format!("{layout}_initial_cross_team_contact"),
            format!("{cross_contact:?}"),
        );
        report.verdict(
            &id("FOG-INITIAL"),
            fog_ok,
            format!("server activation samples {samples:?}; {fog_detail:?}"),
        );
        if seats > 2 {
            report.verdict(
                &id("FOG-ALLIED-SHARED"),
                allied_ok,
                format!("every client receives all allied participants' units: {fog_detail:?}"),
            );
        } else {
            report.mark(
                &id("FOG-ALLIED-SHARED"),
                "NOT_RUN",
                "1v1 has no allied participants",
            );
        }

        // Probing clients finish their hostile sweep before any order is issued.
        for client in clients
            .iter_mut()
            .filter(|c| c.label == "client-2" || c.label == "client-3")
        {
            client.wait(&["MP_PROBE_COMPLETE"], Duration::from_secs(25))?;
        }
        thread::sleep(Duration::from_secs(2));
        let first_unit = |participant: u32| -> Option<u64> {
            samples
                .iter()
                .find(|(_, owner, _)| *owner == participant)
                .map(|(e, _, _)| *e)
        };
        let second_unit = |participant: u32| -> Option<u64> {
            samples
                .iter()
                .filter(|(_, owner, _)| *owner == participant)
                .nth(1)
                .map(|(e, _, _)| *e)
        };

        // Every client: real-input Stop and Guard on its own first unit.
        let mut order_detail = Vec::new();
        for index in 0..seats {
            let participant = index as u32 + 1;
            let unit =
                first_unit(participant).ok_or(format!("participant {participant} has no unit"))?;
            let (sample, _) = seat_markers(listen, index);
            select_entity(
                seat_game(&mut server, &mut clients, listen, index),
                sample,
                unit,
            )?;
            for (key, order) in [(STOP_KEY, "Stop"), ("g", "Guard")] {
                let mark = server.offset();
                seat_game(&mut server, &mut clients, listen, index).key(key)?;
                let admitted = server.wait_line(
                    &[
                        &format!("RTS_ORDER_ADMITTED order={order} "),
                        &format!("unit={unit} "),
                        &format!("participant={participant} "),
                    ],
                    Duration::from_secs(5),
                    mark,
                )?;
                order_detail.push(admitted.trim().to_owned());
                thread::sleep(Duration::from_millis(300));
            }
        }
        // Each command arrives attributed to its own seat: the listen host's as local
        // (remote=0) participant 1, every client's as a remote participant.
        let server_text = server.text();
        let local = count_lines(&server_text, &["MP_COMMAND_RECEIVED", "remote=0"]);
        let host_local = count_lines(
            &server_text,
            &["MP_COMMAND_RECEIVED participant=1 remote=0"],
        );
        let attributed = (if listen {
            host_local > 0 && host_local == local
        } else {
            local == 0
        }) && (first..=seats).all(|p| {
            count_lines(
                &server_text,
                &[&format!("MP_COMMAND_RECEIVED participant={p} remote=1")],
            ) > 0
        });
        report.verdict(
            &id("ORDERS-EVERY-CLIENT"),
            attributed && order_detail.len() == seats * 2,
            format!(
                "{} real-input orders; local commands {local} (host participant 1: {host_local}), every client remote: {order_detail:?}",
                order_detail.len()
            ),
        );

        // Contest the objective: team B moves part-way, team A takes it (participant 1
        // moves onto it, other team-A members attack-move to it).
        let since = server.offset();
        for index in 0..seats {
            let participant = index as u32 + 1;
            let unit = first_unit(participant).ok_or("missing unit")?;
            let (sample, objective_marker) = seat_markers(listen, index);
            let client = seat_game(&mut server, &mut clients, listen, index);
            zoom_out(client, 5)?;
            thread::sleep(Duration::from_millis(1500));
            let objective = wait_screen(
                client,
                |t| objective_screen(t, objective_marker),
                "objective",
            )?;
            if teams[index] == 2 {
                let own = wait_screen(client, |t| entity_screen(t, sample, unit), "own unit")?;
                select_entity(client, sample, unit)?;
                client.click(
                    objective.0 + (own.0 - objective.0) * 0.45,
                    objective.1 + (own.1 - objective.1) * 0.45,
                    "3",
                )?;
            } else if participant == 1 {
                select_entity(client, sample, unit)?;
                client.click(objective.0, objective.1, "3")?;
                // 1v1 has no team-A ally to support the capture; a lone unit can lose the
                // duel at the point and leave the match uncapturable. Its second unit
                // attack-moves in support, as the allied seats do in larger layouts.
                if let Some(support) = (seats == 2).then(|| second_unit(1)).flatten() {
                    thread::sleep(Duration::from_millis(400));
                    select_entity(client, sample, support)?;
                    client.hover(objective.0 - 18.0, objective.1 + 10.0)?;
                    thread::sleep(Duration::from_millis(150));
                    client.key(ATTACK_MOVE_KEY)?;
                }
            } else {
                select_entity(client, sample, unit)?;
                client.hover(objective.0 + 18.0, objective.1 + 10.0)?;
                thread::sleep(Duration::from_millis(150));
                client.key(ATTACK_MOVE_KEY)?;
            }
            thread::sleep(Duration::from_millis(400));
        }
        let mut advance = Vec::new();
        for index in 0..seats {
            let participant = index + 1;
            let unit = first_unit(participant as u32).ok_or("missing unit")?;
            let order = if teams[index] == 2 || participant == 1 {
                "Move"
            } else {
                "AttackMove"
            };
            advance.push(
                server
                    .wait_line(
                        &[
                            &format!("RTS_ORDER_ADMITTED order={order} "),
                            &format!("unit={unit} "),
                            &format!("participant={participant} "),
                        ],
                        Duration::from_secs(5),
                        since,
                    )?
                    .trim()
                    .to_owned(),
            );
        }
        let mut expected_advance = seats;
        if let Some(support) = (seats == 2).then(|| second_unit(1)).flatten() {
            expected_advance += 1;
            advance.push(
                server
                    .wait_line(
                        &[
                            "RTS_ORDER_ADMITTED order=AttackMove ",
                            &format!("unit={support} "),
                            "participant=1 ",
                        ],
                        Duration::from_secs(5),
                        since,
                    )?
                    .trim()
                    .to_owned(),
            );
        }
        report.verdict(
            &id("ADVANCE-ORDERS"),
            advance.len() == expected_advance,
            format!("{advance:?}"),
        );
        for client in rendered_games(&server, &clients, listen) {
            report.artifact(
                &format!("{layout}_{}_orders_png", client.label),
                &client.screenshot(&evidence, "02-orders")?,
            );
        }

        // Contact: each team-B client eventually receives a team-A unit it did not see at
        // start. Team B is never seat 1, so it is always a remote client.
        let mut reveals = Vec::new();
        let mut reveal_ok = true;
        for index in (0..seats).filter(|i| teams[*i] == 2) {
            let index = index + 1 - first;
            let initially: Vec<u64> = samples
                .iter()
                .filter(|(_, _, mask)| mask & (1 << 2) != 0)
                .map(|(e, _, _)| *e)
                .collect();
            let start = Instant::now();
            let mut found = None;
            while start.elapsed() < Duration::from_secs(45) && found.is_none() {
                found = relevant_entities(&clients[index].text())
                    .into_iter()
                    .find(|(e, owner)| {
                        *owner != 0 && team_of(*owner) == 1 && !initially.contains(e)
                    });
                thread::sleep(Duration::from_millis(250));
            }
            reveal_ok &= found.is_some();
            reveals.push(format!("{}: {found:?}", clients[index].label));
        }
        report.verdict(&id("FOG-REVEAL"), reveal_ok, format!("{reveals:?}"));
        for client in rendered_games(&server, &clients, listen) {
            report.artifact(
                &format!("{layout}_{}_contact_png", client.label),
                &client.screenshot(&evidence, "03-contact")?,
            );
        }

        let results = server.wait(
            &[
                "MATCH_RESULTS outcome=objective_secured",
                &format!("generation={generation}"),
            ],
            Duration::from_secs(120),
        )?;
        let winner = field(&results, "team").unwrap_or("?").to_owned();
        let mut observed = Vec::new();
        let mut observed_ok = true;
        // The listen host committed the result itself; every remote client observes it.
        for (index, client) in clients.iter_mut().enumerate() {
            let line = client.wait(
                &[
                    "MP_CLIENT_LIFECYCLE",
                    "result=1",
                    "reason=objective_secured",
                ],
                Duration::from_secs(20),
            )?;
            observed_ok &= field(&line, "team") == Some(winner.as_str())
                && field(&line, "viewer_team")
                    == Some(teams[index + first - 1].to_string().as_str());
            observed.push(line.trim().to_owned());
        }
        thread::sleep(Duration::from_millis(1500));
        for client in rendered_games(&server, &clients, listen) {
            report.artifact(
                &format!("{layout}_{}_results_png", client.label),
                &client.screenshot(&evidence, "04-results")?,
            );
        }
        let commits = count_lines(
            &server.text(),
            &["MATCH_RESULTS", &format!("generation={generation}")],
        );
        report.verdict(
            &id("RESULT-ONCE-ALL-VIEWERS"),
            commits == 1 && observed_ok && (winner == "1" || winner == "2"),
            format!(
                "{commits} commit(s): {}; observed {observed:?}",
                results.trim()
            ),
        );

        for client in &clients {
            let (states, deaths, problems) = health_records(&server, client);
            let name = id(&format!("DAMAGE-{}", client.label.to_uppercase()));
            let detail = format!(
                "{states} damaged states, {deaths} deaths, all server-originated; problems={problems:?}"
            );
            if !problems.is_empty() {
                report.verdict(&name, false, detail);
            } else if states == 0 {
                report.mark(
                    &name,
                    "INCONCLUSIVE",
                    format!("client observed no damage; {detail}"),
                );
            } else {
                report.verdict(&name, true, detail);
            }
        }

        let remote: Vec<u32> = (first as u32..=seats as u32).collect();
        // Probing clients: participant 2 (team B) and, with three or more seats, 3 (team A).
        // Both are remote clients in either hosting mode.
        let probing: Vec<usize> = [1usize, 2].into_iter().filter(|i| *i < seats).collect();
        for &index in &probing {
            let client = &mut clients[index + 1 - first];
            client.wait(
                &["MP_PROBE_SENT name=after_termination"],
                Duration::from_secs(10),
            )?;
        }
        thread::sleep(Duration::from_secs(1));
        for &index in &probing {
            let client = &clients[index + 1 - first];
            let label = client.label.to_uppercase();
            check_probes_for(
                report,
                &id(&format!("PROBES-{label}")),
                client,
                &server,
                &remote,
            );
            let text = client.text();
            let begin = text
                .lines()
                .find(|l| l.contains("MP_PROBE_BEGIN"))
                .unwrap_or("");
            let has_ally = field(begin, "ally").is_some_and(|a| a != "0");
            let expects_ally = teams.iter().filter(|t| **t == teams[index]).count() > 1;
            let allied_probes = count_lines(&text, &["MP_PROBE_SENT name=allied_"]);
            report.verdict(
                &id(&format!("ALLIED-PROBES-{label}")),
                has_ally == expects_ally && allied_probes == if expects_ally { 2 } else { 0 },
                format!("ally present={has_ally} (expected {expects_ally}); allied probes sent={allied_probes}; {}", begin.trim()),
            );
        }

        let (received, counts, problems) =
            shot_cue_consistency(&server, &clients.iter().collect::<Vec<_>>());
        report.fact(
            &format!("{layout}_shot_cues"),
            format!(
                "server decisions exact={} facing={} none={}; client receipts={received}",
                counts[0], counts[1], counts[2]
            ),
        );
        report.verdict(
            &id("SHOT-CUES-FOG-SAFE"),
            problems.is_empty() && counts[0] > 0 && received > 0,
            format!(
                "decisions exact={} facing={} none={}, {received} client receipts; no multicast; problems={problems:?}",
                counts[0], counts[1], counts[2]
            ),
        );
        if listen {
            // The host draws every shot from team A's vision, exactly as its remote allies.
            let allies: Vec<u32> = (2..=seats as u32)
                .filter(|p| teams[*p as usize - 1] == teams[0])
                .collect();
            let (total, host_counts, compared, problems) =
                host_shot_cue_audit(&server.text(), &allies);
            let detail = format!(
                "host cues {total}: exact={} facing={} none={}; {compared} matched allied decisions {allies:?}; problems={problems:?}",
                host_counts[0], host_counts[1], host_counts[2]
            );
            if allies.is_empty() {
                report.verdict(
                    &id("HOST-SHOT-CUES"),
                    problems.is_empty() && host_counts[0] > 0,
                    format!("{detail}; no allied seat to compare"),
                );
            } else {
                report.verdict(
                    &id("HOST-SHOT-CUES"),
                    problems.is_empty() && host_counts[0] > 0 && compared == total * allies.len(),
                    detail,
                );
            }
        }
        let mut memory_detail = Vec::new();
        let mut memory_ok = true;
        if listen {
            let (recorded, transitions, problems) = host_contact_audit(&server.text());
            memory_ok &= problems.is_empty() && transitions > 0;
            memory_detail.push(format!(
                "host: recorded {recorded}, fog transitions {transitions}, problems={problems:?}"
            ));
        }
        for client in &clients {
            let (recorded, deaths, problems) = contact_memory_audit(&client.text());
            memory_ok &= problems.is_empty();
            memory_detail.push(format!(
                "{}: recorded {recorded}, deaths seen {deaths}, problems={problems:?}",
                client.label
            ));
        }
        report.verdict(
            &id("FOG-MEMORY-NO-LEAK"),
            memory_ok,
            memory_detail.join("; "),
        );
        network_checks(
            report,
            &id,
            layout,
            profile,
            &server,
            &clients,
            first as u32,
            &generation,
        );

        if listen {
            // Normal completion: the host leaves the finished match for MainMenu, and every
            // client follows, explaining the ended match from the result it observed.
            let marks: Vec<usize> = clients.iter().map(Game::offset).collect();
            let host_mark = server.offset();
            server.hover(640.0, 360.0)?;
            server.key("Escape")?;
            server.wait_line(
                &["LoadMap: /Game/Maps/MainMenu"],
                Duration::from_secs(30),
                host_mark,
            )?;
            let mut failures = Vec::new();
            for (client, mark) in clients.iter_mut().zip(&marks) {
                let failure = client.wait_line(
                    &["MP_CLIENT_NETWORK_FAILURE"],
                    Duration::from_secs(30),
                    *mark,
                )?;
                client.wait_line(
                    &["LoadMap: /Game/Maps/MainMenu"],
                    Duration::from_secs(30),
                    *mark,
                )?;
                failures.push(failure.trim().to_owned());
            }
            thread::sleep(Duration::from_secs(1));
            report.verdict(
                &id("RETURN-AFTER-RESULTS"),
                failures.len() == clients.len()
                    && failures
                        .iter()
                        .all(|f| field(f, "result_observed") == Some("1")),
                format!(
                    "host and {} clients returned to MainMenu; {failures:?}",
                    clients.len()
                ),
            );
            for game in rendered_games(&server, &clients, listen) {
                report.artifact(
                    &format!("{layout}_{}_menu_png", game.label),
                    &game.screenshot(&evidence, "05-menu")?,
                );
            }
            return Ok(());
        }

        // Every client leaves the finished match; the same server resets for a new generation.
        let server_mark = server.offset();
        let marks: Vec<usize> = clients.iter().map(Game::offset).collect();
        for client in &clients {
            client.hover(640.0, 360.0)?;
            client.key("Escape")?;
        }
        for (client, mark) in clients.iter_mut().zip(&marks) {
            client.wait_line(
                &["LoadMap: /Game/Maps/MainMenu"],
                Duration::from_secs(30),
                *mark,
            )?;
        }
        let closing = server.wait_line(
            &[
                "MP_DEDICATED_SESSION_CLOSING reason=results_complete",
                &format!("generation={generation}"),
            ],
            Duration::from_secs(30),
            server_mark,
        )?;
        let awaiting2 = server.wait_line(
            &["MP_MATCH_AWAITING_PARTICIPANTS"],
            Duration::from_secs(60),
            server_mark,
        )?;
        let layout2 =
            server.wait_line(&["MP_SESSION_LAYOUT"], Duration::from_secs(10), server_mark)?;
        report.verdict(
            &id("RESET-AFTER-RESULTS"),
            server.alive()
                && server.pid() == server_pid
                && field(&closing, "remaining") == Some("0")
                && field(&layout2, "layout") == Some(layout)
                && field(&awaiting2, "generation").and_then(|g| g.parse::<u64>().ok())
                    > generation.parse::<u64>().ok(),
            format!(
                "{}; {}; {}",
                closing.trim(),
                awaiting2.trim(),
                layout2.trim()
            ),
        );
        for client in &clients {
            report.artifact(
                &format!("{layout}_{}_menu_png", client.label),
                &client.screenshot(&evidence, "05-menu")?,
            );
        }
        Ok(())
    })();
    for (recorder, client) in recorders
        .into_iter()
        .zip(rendered_games(&server, &clients, listen))
    {
        match recorder.finish() {
            Ok(path) => report.artifact(&format!("{layout}_{}_mp4", client.label), &path),
            Err(error) => report.mark(
                &id(&format!("VIDEO-{}", client.label)),
                "INFRA_ERROR",
                error,
            ),
        }
    }
    let mut launched: Vec<&Game> = vec![&server];
    launched.extend(clients.iter());
    launch_options_verdict(report, &id("DEV-LAUNCH-OPTIONS"), &launched);
    for client in &mut clients {
        client.stop();
        report.artifact(
            &format!("{layout}_{}_log", client.label),
            &client.log.clone(),
        );
    }
    server.stop();
    report.artifact(
        &format!("{layout}_{}_log", server.label),
        &server.log.clone(),
    );
    result
}

/// A client built for an older network protocol is refused before any seat exists.
fn legacy_protocol_refusal(
    options: &LayoutOptions,
    port: u16,
    report: &mut Report,
) -> Result<(), String> {
    let Some(legacy) = options.legacy_client.as_ref() else {
        report.mark(
            "MP2-PROTOCOL-LEGACY-REFUSED",
            "NOT_RUN",
            "no --legacy-client given",
        );
        return Ok(());
    };
    let evidence = options.evidence.join("legacy-protocol");
    fs::create_dir_all(&evidence).map_err(|e| e.to_string())?;
    let mut server = Game::launch_server(
        &options.server,
        "server",
        &evidence,
        &server_args(port, "1v1"),
    )?;
    server.wait(
        &["MP_MATCH_AWAITING_PARTICIPANTS"],
        Duration::from_secs(120),
    )?;
    let client_options = Options {
        binary: legacy.clone(),
        layout: "1v1".into(),
        evidence: evidence.clone(),
        port,
    };
    let mut client = Game::launch(
        legacy,
        "legacy-client",
        &evidence,
        &client_args(&client_options, false, false, false),
        None,
    )?;
    report.fact("legacy_client_binary", legacy.display().to_string());
    if let Ok(digest) = sha256(legacy) {
        report.fact("legacy_client_executable_sha256", digest);
    }
    let refused = server.wait(&["MP_PRELOGIN REJECTED"], Duration::from_secs(60));
    let failure = client.wait(&["MP_CLIENT_NETWORK_FAILURE"], Duration::from_secs(30));
    let menu = client.wait(&["LoadMap: /Game/Maps/MainMenu"], Duration::from_secs(30));
    let no_seat = !server.text().contains("MP_PARTICIPANT_BOUND");
    let current = network_protocol();
    let pass = refused.as_ref().is_ok_and(|l| {
        field(l, "reason") == Some("MP.PROTOCOL.MISMATCH")
            && field(l, "protocol") != Some(current.as_str())
    }) && failure.is_ok()
        && menu.is_ok()
        && no_seat;
    report.verdict(
        "MP2-PROTOCOL-LEGACY-REFUSED",
        pass,
        format!("server protocol {current}; {refused:?}; client {failure:?}; menu={}; no seat bound={no_seat}", menu.is_ok()),
    );
    client.stop();
    server.stop();
    report.artifact("legacy_server_log", &server.log.clone());
    report.artifact("legacy_client_log", &client.log.clone());
    Ok(())
}

fn run_layouts(listen: bool) -> ! {
    let mut options = match parse_layout_options(listen) {
        Ok(options) => options,
        Err(error) => {
            eprintln!("{error}");
            std::process::exit(2);
        }
    };
    match fs::create_dir_all(&options.evidence).and_then(|_| options.evidence.canonicalize()) {
        Ok(absolute) => options.evidence = absolute,
        Err(error) => {
            eprintln!("evidence directory: {error}");
            std::process::exit(2);
        }
    }
    let mut report = Report {
        schema: Some(if listen {
            "politika.mp2.listen-layouts-evidence.v1"
        } else {
            "politika.mp2.multiclient-evidence.v1"
        }),
        ..Report::default()
    };
    report.fact("network_protocol", network_protocol());
    report.fact("hosting", if listen { "listen" } else { "dedicated" });
    for (name, binary) in [("server", &options.server), ("client", &options.client)] {
        report.fact(&format!("{name}_binary"), binary.display().to_string());
        if let Ok(digest) = sha256(binary) {
            report.fact(&format!("{name}_executable_sha256"), digest);
        }
        if let Some(archive) = binary.ancestors().nth(4)
            && let Ok(identity) = fs::read_to_string(archive.join("BuildIdentity.json"))
        {
            report.fact(
                &format!("{name}_build_identity"),
                identity.trim().to_owned(),
            );
        }
    }
    report.fact("layouts", options.layouts.join(","));
    report.fact("net_profile", options.profile.name);
    report.fact("net_profile_args", options.profile.args.join(" "));
    let layouts = options.layouts.clone();
    for (index, layout) in layouts.iter().enumerate() {
        let port = options.port + index as u16 * 2;
        if let Err(error) = layout_session(&options, layout, port, &mut report) {
            report.mark(
                &format!("SESSION-{layout}"),
                if error.starts_with("BLOCKED") {
                    "BLOCKED"
                } else {
                    "FAIL"
                },
                error,
            );
        }
    }
    let legacy_port = options.port + layouts.len() as u16 * 2;
    if listen {
        // Protocol refusal happens at PreLogin, before hosting mode matters; the dedicated
        // layouts run qualifies it.
        report.mark(
            "MP2-PROTOCOL-LEGACY-REFUSED",
            "NOT_RUN",
            "listen layouts: covered by --dedicated-layouts --legacy-client",
        );
    } else if let Err(error) = legacy_protocol_refusal(&options, legacy_port, &mut report) {
        report.mark("SESSION-legacy-protocol", "FAIL", error);
    }
    let path = options.evidence.join(if listen {
        "listen-layouts-evidence.json"
    } else {
        "multiclient-evidence.json"
    });
    if let Err(error) = report.write(&path) {
        eprintln!("write evidence: {error}");
        std::process::exit(2);
    }
    println!("evidence: {}", path.display());
    std::process::exit(if report.failed() { 1 } else { 0 });
}

// ----------------------------------------------------------------------------
// MP-2 / audit F33: viewer-side fog memory (last-known contacts) and fog shading
// ----------------------------------------------------------------------------

/// Last-known contacts a client recorded must come only from contacts it was sent: each
/// one follows a live withdrawal of the same entity at the same position, never names the
/// viewer's own side, and a death the client saw never leaves a contact behind.
/// Returns (recorded contacts, deaths seen, problems).
fn contact_memory_audit(text: &str) -> (usize, usize, Vec<String>) {
    let lines: Vec<&str> = text.lines().collect();
    let mut problems = Vec::new();
    let mut recorded = 0;
    let mut deaths: Vec<(usize, String)> = Vec::new();
    for (index, line) in lines.iter().enumerate() {
        if line.contains("MP_CLIENT_UNIT_HEALTH")
            && field(line, "health") == Some("0")
            && let Some(entity) = field(line, "entity")
        {
            deaths.push((index, entity.to_owned()));
        }
    }
    for (index, line) in lines.iter().enumerate() {
        if !line.contains("MP_LOCAL_CONTACT_LAST_KNOWN") {
            continue;
        }
        recorded += 1;
        let entity = field(line, "entity").unwrap_or("?");
        let withdrawn = lines[..index]
            .iter()
            .rev()
            .find(|l| l.contains("MP_CLIENT_UNIT_WITHDRAWN") && field(l, "entity") == Some(entity));
        let relevant = lines[..index]
            .iter()
            .any(|l| l.contains("MP_CLIENT_UNIT_RELEVANT") && field(l, "entity") == Some(entity));
        let close = |a: Option<f64>, b: Option<f64>| matches!((a, b), (Some(a), Some(b)) if (a - b).abs() < 0.6);
        match withdrawn {
            Some(w)
                if relevant
                    && field(w, "alive") == Some("1")
                    && close(number(w, "x"), number(line, "x"))
                    && close(number(w, "y"), number(line, "y")) => {}
            _ => problems.push(format!(
                "contact {entity} not preceded by a matching live withdrawal: {}",
                line.trim()
            )),
        }
        if field(line, "owner") == field(line, "viewer")
            || field(line, "team") == field(line, "viewer_team")
        {
            problems.push(format!(
                "contact names the viewer's own side: {}",
                line.trim()
            ));
        }
        if deaths
            .iter()
            .any(|(at, dead)| *at < index && dead == entity)
        {
            problems.push(format!(
                "contact {entity} recorded after its death was seen"
            ));
        }
    }
    (recorded, deaths.len(), problems)
}

fn fog_samples(text: &str, role: &str) -> Vec<(u64, u64, u64)> {
    text.lines()
        .filter(|l| l.contains("MP_LOCAL_FOG") && field(l, "role") == Some(role))
        .filter_map(|l| {
            Some((
                field(l, "visible")?.parse().ok()?,
                field(l, "explored")?.parse().ok()?,
                field(l, "unexplored")?.parse().ok()?,
            ))
        })
        .collect()
}

/// Right-clicks a ground point until the server admits a Move for the unit.
fn order_move(
    game: &mut Game,
    server: &mut Game,
    unit: u64,
    target: (f64, f64),
) -> Result<String, String> {
    select_entity(game, "MP_CLIENT_SAMPLE", unit)?;
    let mark = server.offset();
    game.click(target.0, target.1, "3")?;
    server.wait_line(
        &[
            "RTS_ORDER_ADMITTED order=Move",
            &format!("unit={unit}"),
            "participant=1",
        ],
        Duration::from_secs(5),
        mark,
    )
}

fn fog_memory_session(options: &DedicatedOptions, report: &mut Report) -> Result<(), String> {
    let client_options = &options.client;
    let evidence = &client_options.evidence;
    let port = client_options.port;
    let mut server = Game::launch_server(
        &options.server,
        "server",
        evidence,
        &server_args(port, "1v1"),
    )?;
    report.fact("server_command", server.command.join(" "));
    server.wait(&["MP_DEDICATED_SERVER_READY"], Duration::from_secs(120))?;
    let awaiting = server.wait(&["MP_MATCH_AWAITING_PARTICIPANTS"], Duration::from_secs(30))?;
    let generation = field(&awaiting, "generation").unwrap_or("0").to_owned();

    let display_a = Display::start(evidence, "client-a")?;
    let mut client_a = Game::launch(
        &client_options.binary,
        "client-a",
        evidence,
        &client_args(client_options, true, false, false),
        Some(&display_a.name()),
    )?;
    report.fact("client_a_command", client_a.command.join(" "));
    server.wait(
        &["MP_PARTICIPANT_BOUND participant=1", "remote=1"],
        Duration::from_secs(60),
    )?;
    // Seat 1 only fills the roster; it renders nothing.
    let mut client_b = Game::launch(
        &client_options.binary,
        "client-b",
        evidence,
        &client_args(client_options, false, false, false),
        None,
    )?;
    report.fact("client_b_command", client_b.command.join(" "));
    server.wait(
        &["MP_PARTICIPANT_BOUND participant=2", "remote=1"],
        Duration::from_secs(60),
    )?;
    server.wait(
        &["MP_MATCH_ACTIVE", &format!("generation={generation}")],
        Duration::from_secs(10),
    )?;
    client_a.wait(
        &[
            "MP_CLIENT_MATCH_OBSERVED",
            "map=map.test.multiplayer_skirmish",
        ],
        Duration::from_secs(30),
    )?;
    client_a.attach_window()?;
    let recorder = Recorder::start(&display_a.name(), evidence, "client-a")?;

    let result = (|| -> Result<(), String> {
        let owners = entity_owners(&server.text());
        let unit_a1 = owners
            .iter()
            .find(|(_, o)| *o == 1)
            .map(|(e, _)| *e)
            .ok_or_else(|| format!("no participant-1 unit in {owners:?}"))?;
        // The southern Unaligned militia: stationary, neutral to auto-engagement, and out
        // of team A's starting sight.
        let militia = server
            .text()
            .lines()
            .filter(|l| l.contains("MP_ACCEPTANCE_UNALIGNED"))
            .filter_map(|l| {
                Some((
                    field(l, "entity")?.parse::<u64>().ok()?,
                    number(l, "x")?,
                    number(l, "y")?,
                ))
            })
            .find(|(_, _, y)| *y < 0.0)
            .ok_or("no southern Unaligned placement logged")?;
        report.fact(
            "militia",
            format!("entity={} x={:.1} y={:.1}", militia.0, militia.1, militia.2),
        );

        let start_fog = client_a
            .wait(
                &["MP_LOCAL_FOG role=client", "viewer=1"],
                Duration::from_secs(10),
            )
            .map(|l| l.trim().to_owned())?;
        let militia_marker = format!("entity={} ", militia.0);
        let seen_at_start = client_a
            .text()
            .lines()
            .any(|l| l.contains("MP_CLIENT_UNIT_RELEVANT") && l.contains(&militia_marker));
        report.verdict(
            "MP2-FOG-SHADING-ACTIVE",
            !seen_at_start
                && field(&start_fog, "visible").and_then(|v| v.parse::<u64>().ok()) > Some(0)
                && field(&start_fog, "unexplored").and_then(|v| v.parse::<u64>().ok()) > Some(0),
            format!(
                "fogged start (militia not received={}); {start_fog}",
                !seen_at_start
            ),
        );
        zoom_out(&client_a, 5)?;
        thread::sleep(Duration::from_millis(2500));
        report.artifact(
            "client_a_start_png",
            &client_a.screenshot(evidence, "01-fogged-start")?,
        );

        let home = wait_screen(
            &mut client_a,
            |t| entity_screen(t, "MP_CLIENT_SAMPLE", unit_a1),
            "unit a1",
        )?;
        let objective = wait_screen(
            &mut client_a,
            |t| objective_screen(t, "MP_CLIENT_OBJECTIVE"),
            "objective",
        )?;
        let forward = ((home.0 + objective.0) / 2.0, (home.1 + objective.1) / 2.0);
        report.fact("screen_home", format!("{:.0},{:.0}", home.0, home.1));
        report.fact(
            "screen_forward",
            format!("{:.0},{:.0}", forward.0, forward.1),
        );

        // 1. Advance: the militia comes into sight and is replicated.
        let order1 = order_move(&mut client_a, &mut server, unit_a1, forward)?;
        let relevant = client_a.wait(
            &["MP_CLIENT_UNIT_RELEVANT", &militia_marker],
            Duration::from_secs(40),
        )?;
        report.verdict(
            "MP2-FOG-CONTACT-SEEN",
            true,
            format!("{}; {}", order1.trim(), relevant.trim()),
        );
        thread::sleep(Duration::from_millis(1500));
        report.artifact(
            "client_a_contact_png",
            &client_a.screenshot(evidence, "02-contact")?,
        );

        // 2. Retreat: the server withdraws the militia; the client remembers it.
        let order2 = order_move(&mut client_a, &mut server, unit_a1, home)?;
        let withdrawn = client_a.wait(
            &["MP_CLIENT_UNIT_WITHDRAWN", &militia_marker],
            Duration::from_secs(40),
        )?;
        let last_known = client_a.wait(
            &["MP_LOCAL_CONTACT_LAST_KNOWN", &militia_marker],
            Duration::from_secs(5),
        )?;
        let server_sample = server
            .text()
            .lines()
            .rev()
            .find(|l| l.contains("MP_SERVER_SAMPLE") && l.contains(&militia_marker))
            .map(str::to_owned)
            .unwrap_or_default();
        let same = |a: &str, b: &str, key: &str, tolerance: f64| matches!((number(a, key), number(b, key)), (Some(x), Some(y)) if (x - y).abs() <= tolerance);
        report.verdict(
            "MP2-FOG-LAST-KNOWN",
            field(&withdrawn, "alive") == Some("1")
                && field(&last_known, "role") == Some("client")
                && field(&last_known, "viewer") == Some("1")
                && field(&last_known, "owner") == Some("0")
                && same(&withdrawn, &last_known, "x", 0.6)
                && same(&withdrawn, &last_known, "y", 0.6)
                && same(&last_known, &server_sample, "x", 5.0)
                && same(&last_known, &server_sample, "y", 5.0),
            format!(
                "{}; {}; {} (stationary contact)",
                order2.trim(),
                withdrawn.trim(),
                last_known.trim()
            ),
        );
        // Wait for a periodic fog sample after the unit is home again.
        thread::sleep(Duration::from_secs(6));
        let samples = fog_samples(&client_a.text(), "client");
        let first = samples.first().copied().unwrap_or_default();
        let last = samples.last().copied().unwrap_or_default();
        let remembered = client_a
            .text()
            .lines()
            .rev()
            .find(|l| l.contains("MP_LOCAL_FOG role=client"))
            .map(str::to_owned)
            .unwrap_or_default();
        report.verdict(
            "MP2-FOG-EXPLORED-PERSISTS",
            first.1 == 0
                && last.1 > 0
                && last.0 > 0
                && last.2 < first.2
                && field(&remembered, "contacts") == Some("1"),
            format!(
                "start visible/explored/unexplored={first:?}; after retreat={last:?}; {}",
                remembered.trim()
            ),
        );
        report.artifact(
            "client_a_last_known_png",
            &client_a.screenshot(evidence, "03-last-known")?,
        );

        // 3. Advance again: the live contact replaces the memory.
        let order3 = order_move(&mut client_a, &mut server, unit_a1, forward)?;
        let cleared = client_a.wait(
            &[
                "MP_LOCAL_CONTACT_CLEARED",
                &militia_marker,
                "reason=reobserved",
            ],
            Duration::from_secs(40),
        )?;
        report.verdict(
            "MP2-FOG-REOBSERVED",
            true,
            format!("{}; {}", order3.trim(), cleared.trim()),
        );
        thread::sleep(Duration::from_millis(1500));
        report.artifact(
            "client_a_reobserved_png",
            &client_a.screenshot(evidence, "04-reobserved")?,
        );

        let (recorded, deaths, problems) = contact_memory_audit(&client_a.text());
        let b_contacts = count_lines(&client_b.text(), &["MP_LOCAL_CONTACT_LAST_KNOWN"]);
        report.verdict(
            "MP2-FOG-MEMORY-NO-LEAK",
            problems.is_empty() && recorded >= 1,
            format!("client A recorded {recorded}, deaths seen {deaths}; client B (never saw it) recorded {b_contacts}; problems={problems:?}"),
        );
        let server_text = server.text();
        report.verdict(
            "MP2-FOG-SERVER-PRESENTS-NOTHING",
            count_lines(&server_text, &["MP_LOCAL_FOG"]) == 0
                && count_lines(&server_text, &["MP_LOCAL_CONTACT"]) == 0,
            "the dedicated server derives no viewer fog or contacts",
        );
        Ok(())
    })();
    match recorder.finish() {
        Ok(path) => report.artifact("client_a_video", &path),
        Err(error) => report.fact("client_a_video_error", error),
    }
    launch_options_verdict(
        report,
        "MP2-FOG-DEV-LAUNCH-OPTIONS",
        &[&server, &client_a, &client_b],
    );
    client_a.stop();
    client_b.stop();
    server.stop();
    result
}

/// Where a team-A unit attacks the northern militia from: 550 cm from it, the distance an
/// Attack approach stops at for the 650 cm weapon, so the unit fires without moving and the
/// facing-cue leak check runs at a real engagement distance (audit F43). It is about 1045 cm
/// from team B's nearer start unit (inside its 1200 cm sight, outside its 800 cm idle
/// acquisition), while that unit stays 1307 cm from the militia. Team B sees the shooter
/// but not its target.
const ENGAGE_FACING_POST: (f64, f64) = (480.0, 1518.0);
/// Where a second team-A unit first sees the southern militia (707 cm away), far from
/// every team-B unit: team B sees neither shooter nor target.
const ENGAGE_WITHHELD_POST: (f64, f64) = (-700.0, -950.0);
const SIGHT_CM: f64 = 1200.0;
const TEAM_B_BIT: u32 = 1 << 2;

fn distance(a: (f64, f64), b: (f64, f64)) -> f64 {
    ((a.0 - b.0).powi(2) + (a.1 - b.1).powi(2)).sqrt()
}

/// Screen position of a `-PolitikaMPGroundProbe` point on a rendered client.
fn probe_screen(text: &str, index: usize) -> Option<(f64, f64)> {
    let index = index.to_string();
    text.lines()
        .rev()
        .find(|l| l.contains("MP_CLIENT_GROUND_PROBE") && field(l, "index") == Some(&index))
        .filter(|l| field(l, "on_screen") == Some("1"))
        .and_then(|l| Some((number(l, "screen_x")?, number(l, "screen_y")?)))
        .filter(|(x, y)| {
            *x > 8.0 && *y > 8.0 && *x < f64::from(WIDTH) - 8.0 && *y < f64::from(HEIGHT) - 8.0
        })
}

/// The server's latest sample of an entity: position and the teams observing it.
fn server_unit(text: &str, entity: u64) -> Option<((f64, f64), u32, i64)> {
    text.lines()
        .rev()
        .filter(|l| l.contains("MP_SERVER_SAMPLE"))
        .find(|l| field(l, "entity").and_then(|v| v.parse::<u64>().ok()) == Some(entity))
        .and_then(|l| {
            Some((
                (number(l, "x")?, number(l, "y")?),
                field(l, "observed")?.parse().ok()?,
                field(l, "health")?.parse().ok()?,
            ))
        })
}

/// Waits until the server samples `entity` within `tolerance` cm of `point`.
fn wait_arrival(
    server: &mut Game,
    entity: u64,
    point: (f64, f64),
    tolerance: f64,
    timeout: Duration,
) -> Result<(f64, f64), String> {
    let start = Instant::now();
    loop {
        let last = server_unit(&server.text(), entity).map(|u| u.0);
        if let Some(position) = last
            && distance(position, point) <= tolerance
        {
            return Ok(position);
        }
        if start.elapsed() >= timeout || !server.alive() {
            return Err(format!(
                "entity {entity} did not reach {point:?} within {timeout:?} (last {last:?})"
            ));
        }
        thread::sleep(Duration::from_millis(250));
    }
}

/// Selects `unit`, then right-clicks `target` until the server admits an explicit Attack.
/// A zoomed-out unit is small, so nearby points on its body are also tried. Samples are a second
/// apart, so the first aim waits for one logged after the call: an older one can place a target
/// that has just stopped where it was still walking. A click that lands on something else is still
/// an order the unit carries out, so it is stopped before the next try.
fn order_attack(
    game: &mut Game,
    server: &mut Game,
    unit: u64,
    target: u64,
) -> Result<String, String> {
    select_entity(game, "MP_CLIENT_SAMPLE", unit)?;
    let unit_tag = format!("unit={unit} ");
    let target_tag = format!("target={target} ");
    let sampled = game.offset();
    let _ = game.wait_line(
        &["MP_CLIENT_SAMPLE", &format!("entity={target} ")],
        Duration::from_secs(3),
        sampled,
    );
    for (dx, dy) in [
        (0.0, 0.0),
        (0.0, -8.0),
        (0.0, -16.0),
        (4.0, -4.0),
        (-4.0, -4.0),
        (0.0, 6.0),
    ] {
        let (x, y) = wait_screen(
            game,
            |t| entity_screen(t, "MP_CLIENT_SAMPLE", target),
            &format!("target {target}"),
        )?;
        let mark = server.offset();
        game.click(x + dx, y + dy, "3")?;
        if let Ok(line) = server.wait_line(
            &["RTS_ORDER_ADMITTED order=", &unit_tag],
            Duration::from_secs(3),
            mark,
        ) {
            if line.contains("order=Attack ") && line.contains(&target_tag) {
                return Ok(line);
            }
            let stopping = server.offset();
            game.key(STOP_KEY)?;
            server
                .wait_line(
                    &["RTS_ORDER_ADMITTED order=Stop ", &unit_tag],
                    Duration::from_secs(3),
                    stopping,
                )
                .map_err(|e| format!("stopping {unit} after a stray order: {e}"))?;
        }
    }
    Err(format!(
        "{}: no admitted Attack of {target} by {unit} with real input",
        game.label
    ))
}

/// One server shot-cue decision: (sequence, participant, shooter, target, cue).
fn cue_decisions(server_text: &str) -> Vec<(u32, u32, u64, u64, String)> {
    server_text
        .lines()
        .filter(|l| l.contains("MP_SHOT_CUE_SENT"))
        .filter_map(|l| {
            Some((
                field(l, "sequence")?.parse().ok()?,
                field(l, "participant")?.parse().ok()?,
                field(l, "shooter")?.parse().ok()?,
                field(l, "target")?.parse().ok()?,
                field(l, "cue")?.to_owned(),
            ))
        })
        .collect()
}

/// Shot cues a client received: (sequence, exact).
fn cues_received(client_text: &str) -> Vec<(u32, bool)> {
    client_text
        .lines()
        .filter(|l| l.contains("MP_CLIENT_SHOT_CUE"))
        .filter_map(|l| {
            Some((
                field(l, "sequence")?.parse().ok()?,
                field(l, "exact")? == "1",
            ))
        })
        .collect()
}

fn relevant_on(client_text: &str, entity: u64) -> bool {
    let marker = format!("entity={entity} ");
    client_text
        .lines()
        .any(|l| l.contains("MP_CLIENT_UNIT_RELEVANT") && l.contains(&marker))
}

/// Waits until the server has decided `count` cues against `target`, or it is destroyed.
fn wait_cues(server: &mut Game, target: u64, count: usize, timeout: Duration) -> usize {
    let start = Instant::now();
    loop {
        let text = server.text();
        let decided = cue_decisions(&text)
            .iter()
            .filter(|d| d.3 == target && d.1 == 1)
            .count();
        let destroyed = text
            .lines()
            .any(|l| l.contains("RTS_UNIT_DESTROYED") && l.contains(&format!("unit={target}")));
        if decided >= count || destroyed || start.elapsed() >= timeout || !server.alive() {
            return decided;
        }
        thread::sleep(Duration::from_millis(250));
    }
}

/// MP-2 Unaligned engagement: explicit team-A attacks on the Unaligned militia produce
/// facing-only cues for a team that sees only the shooter, and nothing at all for a team
/// that sees neither, over real sockets with real input.
fn engagement_session(options: &DedicatedOptions, report: &mut Report) -> Result<(), String> {
    let client_options = &options.client;
    let evidence = &client_options.evidence;
    let port = client_options.port;
    let mut server = Game::launch_server(
        &options.server,
        "server",
        evidence,
        &server_args(port, "1v1"),
    )?;
    report.fact("server_command", server.command.join(" "));
    server.wait(&["MP_DEDICATED_SERVER_READY"], Duration::from_secs(120))?;
    let awaiting = server.wait(&["MP_MATCH_AWAITING_PARTICIPANTS"], Duration::from_secs(30))?;
    let generation = field(&awaiting, "generation").unwrap_or("0").to_owned();

    let display_a = Display::start(evidence, "client-a")?;
    let mut args_a = client_args(client_options, true, false, false);
    args_a.push(format!(
        "-PolitikaMPGroundProbe={}:{}/{}:{}",
        ENGAGE_FACING_POST.0, ENGAGE_FACING_POST.1, ENGAGE_WITHHELD_POST.0, ENGAGE_WITHHELD_POST.1
    ));
    let mut client_a = Game::launch(
        &client_options.binary,
        "client-a",
        evidence,
        &args_a,
        Some(&display_a.name()),
    )?;
    report.fact("client_a_command", client_a.command.join(" "));
    server.wait(
        &["MP_PARTICIPANT_BOUND participant=1", "remote=1"],
        Duration::from_secs(60),
    )?;
    // Team B renders too: it is the viewer whose cues are redacted.
    let display_b = Display::start(evidence, "client-b")?;
    let mut client_b = Game::launch(
        &client_options.binary,
        "client-b",
        evidence,
        &client_args(client_options, true, false, false),
        Some(&display_b.name()),
    )?;
    report.fact("client_b_command", client_b.command.join(" "));
    server.wait(
        &["MP_PARTICIPANT_BOUND participant=2", "remote=1"],
        Duration::from_secs(60),
    )?;
    server.wait(
        &["MP_MATCH_ACTIVE", &format!("generation={generation}")],
        Duration::from_secs(10),
    )?;
    for client in [&mut client_a, &mut client_b] {
        client.wait(
            &[
                "MP_CLIENT_MATCH_OBSERVED",
                "map=map.test.multiplayer_skirmish",
            ],
            Duration::from_secs(30),
        )?;
        client.attach_window()?;
    }
    let recorder_a = Recorder::start(&display_a.name(), evidence, "client-a")?;
    let recorder_b = Recorder::start(&display_b.name(), evidence, "client-b")?;

    let result = (|| -> Result<(), String> {
        let owners = entity_owners(&server.text());
        let units = |owner: u32| -> Vec<u64> {
            owners
                .iter()
                .filter(|(_, o)| *o == owner)
                .map(|(e, _)| *e)
                .collect()
        };
        let (team_a, team_b) = (units(1), units(2));
        if team_a.len() < 2 || team_b.is_empty() {
            return Err(format!("unexpected 1v1 roster {owners:?}"));
        }
        let militia: Vec<(u64, (f64, f64))> = server
            .text()
            .lines()
            .filter(|l| l.contains("MP_ACCEPTANCE_UNALIGNED"))
            .filter_map(|l| {
                Some((
                    field(l, "entity")?.parse::<u64>().ok()?,
                    (number(l, "x")?, number(l, "y")?),
                ))
            })
            .collect();
        let north = *militia
            .iter()
            .find(|(_, p)| p.1 > 0.0)
            .ok_or("no northern Unaligned placement logged")?;
        let south = *militia
            .iter()
            .find(|(_, p)| p.1 < 0.0)
            .ok_or("no southern Unaligned placement logged")?;
        let (facing_shooter, withheld_shooter) = (team_a[0], team_a[1]);
        report.fact("team_a", format!("{team_a:?}"));
        report.fact("team_b", format!("{team_b:?}"));
        report.fact("militia", format!("north={north:?} south={south:?}"));

        zoom_out(&client_a, 5)?;
        zoom_out(&client_b, 5)?;
        thread::sleep(Duration::from_millis(2500));
        report.artifact(
            "client_a_start_png",
            &client_a.screenshot(evidence, "01-start")?,
        );

        // 1. Facing-only: shoot the northern militia from where team B sees only the shooter.
        let post = wait_screen(&mut client_a, |t| probe_screen(t, 0), "facing post")?;
        let moved = order_move(&mut client_a, &mut server, facing_shooter, post)?;
        let arrived = wait_arrival(
            &mut server,
            facing_shooter,
            ENGAGE_FACING_POST,
            150.0,
            Duration::from_secs(45),
        )?;
        client_a.wait(
            &["MP_CLIENT_UNIT_RELEVANT", &format!("entity={} ", north.0)],
            Duration::from_secs(10),
        )?;
        // A fresh sample after the unit stops, so team B's observation is current.
        thread::sleep(Duration::from_millis(2500));
        let text = server.text();
        let shooter = server_unit(&text, facing_shooter).ok_or("no shooter sample")?;
        let target = server_unit(&text, north.0).ok_or("no militia sample")?;
        let nearest_b = team_b
            .iter()
            .filter_map(|e| server_unit(&text, *e))
            .map(|u| (distance(u.0, shooter.0), distance(u.0, target.0)))
            .fold((f64::MAX, f64::MAX), |acc, d| {
                (acc.0.min(d.0), acc.1.min(d.1))
            });
        report.verdict(
            "MP2-ENGAGE-FACING-GEOMETRY",
            distance(shooter.0, target.0) <= 625.0
                && shooter.1 & TEAM_B_BIT != 0
                && target.1 & TEAM_B_BIT == 0
                && nearest_b.0 < SIGHT_CM
                && nearest_b.1 > SIGHT_CM,
            format!(
                "{}; arrived {arrived:?}; shooter-target {:.0}cm; nearest team-B unit {:.0}cm from shooter, {:.0}cm from target; observed masks shooter={} target={}",
                moved.trim(),
                distance(shooter.0, target.0),
                nearest_b.0,
                nearest_b.1,
                shooter.1,
                target.1
            ),
        );
        let attack_north = order_attack(&mut client_a, &mut server, facing_shooter, north.0)?;
        let decided = wait_cues(&mut server, north.0, 12, Duration::from_secs(20));
        thread::sleep(Duration::from_millis(400));
        report.artifact(
            "client_a_facing_png",
            &client_a.screenshot(evidence, "02-owner-exact")?,
        );
        report.artifact(
            "client_b_facing_png",
            &client_b.screenshot(evidence, "02-opponent-facing")?,
        );
        wait_cues(&mut server, north.0, usize::MAX, Duration::from_secs(25));
        report.fact(
            "facing_attack",
            format!("{} ({decided} cues by the screenshot)", attack_north.trim()),
        );

        // 2. Withheld: shoot the southern militia far from every team-B unit.
        let post = wait_screen(&mut client_a, |t| probe_screen(t, 1), "withheld post")?;
        let moved = order_move(&mut client_a, &mut server, withheld_shooter, post)?;
        wait_arrival(
            &mut server,
            withheld_shooter,
            ENGAGE_WITHHELD_POST,
            150.0,
            Duration::from_secs(30),
        )?;
        client_a.wait(
            &["MP_CLIENT_UNIT_RELEVANT", &format!("entity={} ", south.0)],
            Duration::from_secs(10),
        )?;
        let attack_south = order_attack(&mut client_a, &mut server, withheld_shooter, south.0)?;
        wait_cues(&mut server, south.0, 12, Duration::from_secs(20));
        report.artifact(
            "client_b_withheld_png",
            &client_b.screenshot(evidence, "03-opponent-withheld")?,
        );
        wait_cues(&mut server, south.0, usize::MAX, Duration::from_secs(25));
        report.fact(
            "withheld_attack",
            format!("{}; {}", moved.trim(), attack_south.trim()),
        );
        // Let in-flight cues and the death presentation reach both clients.
        thread::sleep(Duration::from_secs(3));

        let server_text = server.text();
        let (text_a, text_b) = (client_a.text(), client_b.text());
        let decisions = cue_decisions(&server_text);
        let on = |target: u64, participant: u32| -> Vec<&(u32, u32, u64, u64, String)> {
            decisions
                .iter()
                .filter(|d| d.3 == target && d.1 == participant)
                .collect()
        };
        let all = |cues: &[&(u32, u32, u64, u64, String)], cue: &str| {
            !cues.is_empty() && cues.iter().all(|d| d.4 == cue)
        };
        let (north_a, north_b) = (on(north.0, 1), on(north.0, 2));
        let (south_a, south_b) = (on(south.0, 1), on(south.0, 2));
        let (received_a, received_b) = (cues_received(&text_a), cues_received(&text_b));
        let got = |received: &[(u32, bool)], cues: &[&(u32, u32, u64, u64, String)]| {
            received
                .iter()
                .filter(|r| cues.iter().any(|d| d.0 == r.0))
                .copied()
                .collect::<Vec<_>>()
        };
        let (owner_north, owner_south) = (got(&received_a, &north_a), got(&received_a, &south_a));
        let (facing_got, withheld_got) = (got(&received_b, &north_b), got(&received_b, &south_b));

        report.verdict(
            "MP2-ENGAGE-EXPLICIT-ATTACK-ON-UNALIGNED",
            server_text.lines().any(|l| {
                l.contains("RTS_UNIT_DESTROYED") && l.contains(&format!("unit={}", north.0))
            }) && server_text.lines().any(|l| {
                l.contains("RTS_UNIT_DESTROYED") && l.contains(&format!("unit={}", south.0))
            }),
            format!(
                "{}; {}; both militia destroyed by explicit orders",
                attack_north.trim(),
                attack_south.trim()
            ),
        );
        report.verdict(
            "MP2-ENGAGE-UNALIGNED-NEVER-RETALIATES",
            !decisions.iter().any(|d| d.2 == north.0 || d.2 == south.0),
            "no shot decision has an Unaligned shooter",
        );
        report.verdict(
            "MP2-SHOT-CUE-OWNER-EXACT",
            all(&north_a, "exact")
                && all(&south_a, "exact")
                && !owner_north.is_empty()
                && !owner_south.is_empty()
                && owner_north.iter().chain(&owner_south).all(|r| r.1),
            format!(
                "team A decided exact {}+{}, received {}+{} all exact",
                north_a.len(),
                south_a.len(),
                owner_north.len(),
                owner_south.len()
            ),
        );
        report.verdict(
            "MP2-SHOT-CUE-FACING-OVER-SOCKETS",
            all(&north_b, "facing")
                && !facing_got.is_empty()
                && facing_got.iter().all(|r| !r.1)
                && !relevant_on(&text_b, north.0),
            format!(
                "team B decided facing {} of {}, received {} (all exact=0); northern militia never replicated to team B={}",
                north_b.iter().filter(|d| d.4 == "facing").count(),
                north_b.len(),
                facing_got.len(),
                !relevant_on(&text_b, north.0)
            ),
        );
        report.verdict(
            "MP2-SHOT-CUE-WITHHELD-OVER-SOCKETS",
            all(&south_b, "none")
                && withheld_got.is_empty()
                && !relevant_on(&text_b, south.0)
                && !relevant_on(&text_b, withheld_shooter),
            format!(
                "team B decided none {} of {}, received {}; shooter and southern militia never replicated to team B={}",
                south_b.iter().filter(|d| d.4 == "none").count(),
                south_b.len(),
                withheld_got.len(),
                !relevant_on(&text_b, south.0) && !relevant_on(&text_b, withheld_shooter)
            ),
        );
        let (received, counts, problems) = shot_cue_consistency(&server, &[&client_a, &client_b]);
        report.verdict(
            "MP2-SHOT-CUE-CONSISTENT",
            problems.is_empty() && counts[1] > 0 && counts[2] > 0,
            format!(
                "server decided exact/facing/none={counts:?}, clients received {received}; problems={problems:?}"
            ),
        );
        let remembered_b = count_lines(&text_b, &["MP_LOCAL_CONTACT_LAST_KNOWN"]);
        let remembered_a: Vec<&str> = text_a
            .lines()
            .filter(|l| l.contains("MP_LOCAL_CONTACT_LAST_KNOWN"))
            .filter(|l| {
                l.contains(&format!("entity={} ", north.0))
                    || l.contains(&format!("entity={} ", south.0))
            })
            .collect();
        report.verdict(
            "MP2-ENGAGE-FOG-MEMORY",
            remembered_b == 0 && remembered_a.is_empty(),
            format!(
                "team B (never saw a militia) remembered {remembered_b} contacts; team A (saw both die) remembered {:?}",
                remembered_a
            ),
        );
        let objective = server_text
            .lines()
            .rev()
            .find(|l| l.contains("MP_SERVER_OBJECTIVE"))
            .unwrap_or_default()
            .to_owned();
        report.verdict(
            "MP2-ENGAGE-MATCH-UNDISTURBED",
            !server_text.contains("MATCH_RESULTS")
                && field(&objective, "progress_milli") == Some("0"),
            format!("no result committed; {}", objective.trim()),
        );
        Ok(())
    })();
    for (recorder, name) in [
        (recorder_a, "client_a_video"),
        (recorder_b, "client_b_video"),
    ] {
        match recorder.finish() {
            Ok(path) => report.artifact(name, &path),
            Err(error) => report.fact(&format!("{name}_error"), error),
        }
    }
    launch_options_verdict(
        report,
        "MP2-ENGAGE-DEV-LAUNCH-OPTIONS",
        &[&server, &client_a, &client_b],
    );
    client_a.stop();
    client_b.stop();
    server.stop();
    result
}

type DedicatedSession = fn(&DedicatedOptions, &mut Report) -> Result<(), String>;

/// Runs one packaged dedicated-server session (`--fog-memory`, `--engagement`) and writes
/// its evidence file; exits 1 on any failed verdict.
// ---------------------------------------------------------------------------------------
// MP-2 unit-scale budgets (U-12): `--unit-scale SERVER CLIENT [--layouts 1v1,2v2,3v1]
// [--units 5,10,20] [--engage-seconds 45] [--evidence DIR] [--port N]`. The skirmish test map
// holds at most 20 units per participant (owner decision); 300 is the game-wide limit for
// larger maps and is not exercised here.
//
// For each layout and per-seat count, one headless server hosts the multiplayer skirmish
// with `-PolitikaMPUnitScale=N`; seat 1's client renders on Xvfb, the others run -nullrhi
// so host contention stays bounded. The server holds a 10 s idle window, then attack-moves
// every seated unit at the objective. The harness splits engine traffic and authoritative
// step cost by phase and checks spawn, order admission, replication completeness, step
// budget, send-rate headroom and fog-safe shot cues at that scale.
// ---------------------------------------------------------------------------------------

/// Mirrors `politika::unitscale::kMaxUnitsPerSeat`: the skirmish test map limit.
const MAX_UNITS_PER_SEAT: u32 = 20;

struct UnitScaleOptions {
    server: PathBuf,
    client: PathBuf,
    layouts: Vec<String>,
    units: Vec<u32>,
    engage_seconds: u64,
    evidence: PathBuf,
    port: u16,
}

fn parse_unit_scale_options() -> Result<UnitScaleOptions, String> {
    let usage = "usage: rts_multiplayer --unit-scale SERVER_BINARY CLIENT_BINARY \
                 [--layouts 1v1,2v2,3v1] [--units 5,10,20] [--engage-seconds 45] \
                 [--evidence DIR] [--port N]";
    let mut args = env::args().skip(2);
    let server = PathBuf::from(args.next().ok_or(usage)?)
        .canonicalize()
        .map_err(|e| format!("server binary: {e}"))?;
    let client = PathBuf::from(args.next().ok_or(usage)?)
        .canonicalize()
        .map_err(|e| format!("client binary: {e}"))?;
    let mut layouts = vec!["1v1".to_owned(), "2v2".to_owned(), "3v1".to_owned()];
    let mut units = vec![5, 10, 20];
    let mut engage_seconds = 45;
    let mut evidence = None;
    let mut port = 19200u16;
    while let Some(flag) = args.next() {
        let value = args.next().ok_or_else(|| format!("{flag} needs a value"))?;
        match flag.as_str() {
            "--layouts" => {
                layouts = value.split(',').map(str::to_owned).collect();
                if let Some(bad) = layouts.iter().find(|l| layout_teams(l).is_none()) {
                    return Err(format!("unknown layout {bad}"));
                }
            }
            "--units" => {
                units = value
                    .split(',')
                    .map(|v| v.parse::<u32>().map_err(|e| format!("--units {v}: {e}")))
                    .collect::<Result<_, _>>()?;
                if let Some(bad) = units
                    .iter()
                    .find(|n| !(2..=MAX_UNITS_PER_SEAT).contains(*n))
                {
                    return Err(format!(
                        "--units {bad}: the skirmish test map accepts 2..={MAX_UNITS_PER_SEAT} per seat"
                    ));
                }
            }
            "--engage-seconds" => {
                engage_seconds = value
                    .parse()
                    .map_err(|e| format!("--engage-seconds: {e}"))?
            }
            "--evidence" => evidence = Some(PathBuf::from(value)),
            "--port" => port = value.parse().map_err(|e| format!("--port: {e}"))?,
            other => return Err(format!("unknown option {other}\n{usage}")),
        }
    }
    let epoch = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .map_err(|e| e.to_string())?
        .as_secs();
    let evidence = evidence
        .unwrap_or_else(|| PathBuf::from(format!("build/diagnostics/mp2-unit-scale/run-{epoch}")));
    fs::create_dir_all(&evidence).map_err(|e| e.to_string())?;
    let evidence = evidence.canonicalize().map_err(|e| e.to_string())?;
    Ok(UnitScaleOptions {
        server,
        client,
        layouts,
        units,
        engage_seconds,
        evidence,
        port,
    })
}

/// Wall-clock prefix of a UE log line (`[2026.10.29-20.57.41:274]`). It is fixed width,
/// so lines from processes on the same host compare lexicographically.
fn log_time(line: &str) -> Option<&str> {
    (line.len() > 25 && line.starts_with('[') && line.as_bytes()[24] == b']').then(|| &line[1..24])
}

/// `MP_NET_STATS` samples for one role and generation with their wall-clock time and the
/// connection's configured send rate (`net_speed`, bytes per second).
fn timed_net_samples(text: &str, role: &str, generation: &str) -> Vec<(String, NetSample, f64)> {
    text.lines()
        .filter(|l| {
            l.contains("MP_NET_STATS")
                && field(l, "role") == Some(role)
                && field(l, "generation") == Some(generation)
        })
        .filter_map(|l| {
            let sample = net_samples(l, role, generation).pop()?;
            Some((
                log_time(l)?.to_owned(),
                sample,
                number(l, "net_speed").unwrap_or(0.0),
            ))
        })
        .collect()
}

/// Bytes per second over a phase for one participant, from the server's view of that
/// connection: `down` is server-to-client, `up` client-to-server, `peak_down` the highest
/// one-sample interval. `begin`/`end` bound the phase in wall-clock time; the sample
/// just before `begin` opens the interval so no traffic is lost at the boundary.
#[derive(Debug, Default, Clone, Copy, PartialEq)]
struct PhaseTraffic {
    down: f64,
    up: f64,
    peak_down: f64,
    net_speed: f64,
    seconds: f64,
}

fn phase_traffic(
    samples: &[(String, NetSample, f64)],
    participant: u32,
    begin: Option<&str>,
    end: Option<&str>,
) -> Option<PhaseTraffic> {
    let own: Vec<&(String, NetSample, f64)> = samples
        .iter()
        .filter(|(_, s, _)| s.participant == participant)
        .collect();
    let first = match begin {
        Some(begin) => own.iter().rposition(|(t, _, _)| t.as_str() < begin)?,
        None => 0,
    };
    let last = match end {
        Some(end) => own.iter().rposition(|(t, _, _)| t.as_str() < end)?,
        None => own.len().checked_sub(1)?,
    };
    if last <= first {
        return None;
    }
    let window = &own[first..=last];
    let (a, b) = (&window[0].1, &window[window.len() - 1].1);
    let seconds = b.elapsed - a.elapsed;
    if seconds <= 0.0 {
        return None;
    }
    let peak_down = window
        .windows(2)
        .filter_map(|pair| {
            let dt = pair[1].1.elapsed - pair[0].1.elapsed;
            (dt > 0.2).then(|| (pair[1].1.get("out_bytes") - pair[0].1.get("out_bytes")) / dt)
        })
        .fold(0.0, f64::max);
    Some(PhaseTraffic {
        down: (b.get("out_bytes") - a.get("out_bytes")) / seconds,
        up: (b.get("in_bytes") - a.get("in_bytes")) / seconds,
        peak_down,
        net_speed: window
            .iter()
            .map(|(_, _, speed)| *speed)
            .fold(0.0, f64::max),
        seconds,
    })
}

/// Mean of `mean_ms` and the highest `max_ms` of a client's frame windows in a phase.
fn frame_windows(text: &str, begin: Option<&str>, end: Option<&str>) -> Option<(f64, f64, usize)> {
    let windows: Vec<(f64, f64)> = text
        .lines()
        .filter(|l| l.contains("MP_CLIENT_FRAME_WINDOW"))
        .filter(|l| {
            log_time(l).is_some_and(|t| begin.is_none_or(|b| t >= b) && end.is_none_or(|e| t < e))
        })
        .filter(|l| number(l, "frames").unwrap_or(0.0) > 0.0)
        .filter_map(|l| Some((number(l, "mean_ms")?, number(l, "max_ms")?)))
        .collect();
    (!windows.is_empty()).then(|| {
        (
            windows.iter().map(|w| w.0).sum::<f64>() / windows.len() as f64,
            windows.iter().map(|w| w.1).fold(0.0, f64::max),
            windows.len(),
        )
    })
}

/// Send-rate headroom: the busiest one-second interval must stay below this share of the
/// connection's configured rate, or the engine is starving replication.
const UNIT_SCALE_HEADROOM: f64 = 0.8;

#[allow(clippy::too_many_lines)]
fn unit_scale_run(
    options: &UnitScaleOptions,
    report: &mut Report,
    layout: &str,
    per_seat: u32,
    port: u16,
    summary: &mut Vec<String>,
) -> Result<(), String> {
    let id = |name: &str| format!("{layout}_n{per_seat}_{name}");
    let seats = layout_teams(layout).ok_or("unknown layout")?.len();
    let evidence = options.evidence.join(format!("{layout}-n{per_seat}"));
    fs::create_dir_all(&evidence).map_err(|e| e.to_string())?;
    let load = |when: &str, report: &mut Report| {
        if let Ok(value) = fs::read_to_string("/proc/loadavg") {
            report.fact(&id(&format!("loadavg_{when}")), value.trim().to_owned());
        }
    };
    load("start", report);
    let mut args = server_args(port, layout);
    args.push(format!("-PolitikaMPUnitScale={per_seat}"));
    let mut server = Game::launch_server(&options.server, "server", &evidence, &args)?;
    report.fact(&id("server_command"), server.command.join(" "));
    server.wait(&["MP_DEDICATED_SERVER_READY"], Duration::from_secs(180))?;
    let awaiting = server.wait(&["MP_MATCH_AWAITING_PARTICIPANTS"], Duration::from_secs(60))?;
    let generation = field(&awaiting, "generation").unwrap_or("0").to_owned();

    let display = Display::start(&evidence, "client-a")?;
    let client_options = Options {
        binary: options.client.clone(),
        layout: layout.into(),
        evidence: evidence.clone(),
        port,
    };
    let mut clients: Vec<Game> = Vec::new();
    for seat in 0..seats {
        let label = format!("client-{}", (b'a' + seat as u8) as char);
        let rendered = seat == 0;
        let client = Game::launch(
            &options.client,
            &label,
            &evidence,
            &client_args(&client_options, rendered, false, false),
            rendered.then(|| display.name()).as_deref(),
        )?;
        report.fact(&id(&format!("{label}_command")), client.command.join(" "));
        clients.push(client);
        server.wait(
            &[
                &format!("MP_PARTICIPANT_BOUND participant={}", seat + 1),
                "remote=1",
            ],
            Duration::from_secs(90),
        )?;
    }
    let active = server.wait(
        &["MP_MATCH_ACTIVE", &format!("generation={generation}")],
        Duration::from_secs(30),
    )?;
    report.fact(&id("match_active"), active.trim().to_owned());
    clients[0].wait(
        &[
            "MP_CLIENT_MATCH_OBSERVED",
            "map=map.test.multiplayer_skirmish",
        ],
        Duration::from_secs(60),
    )?;
    clients[0].attach_window()?;
    thread::sleep(Duration::from_secs(6));
    if let Ok(path) = clients[0].screenshot(&evidence, "idle") {
        report.artifact(&id("client_a_idle_png"), &path);
    }
    let engage = server.wait(
        &[
            "MP_UNIT_SCALE_PHASE phase=engage",
            &format!("generation={generation}"),
        ],
        Duration::from_secs(60),
    )?;
    let engage_time = log_time(&engage).unwrap_or_default().to_owned();
    let orders = server.wait(&["MP_UNIT_SCALE_ORDERS"], Duration::from_secs(20));
    thread::sleep(Duration::from_secs(12));
    if let Ok(path) = clients[0].screenshot(&evidence, "engage") {
        report.artifact(&id("client_a_engage_png"), &path);
    }
    // Hold the engagement for the requested time unless the match ends first.
    let deadline = Instant::now() + Duration::from_secs(options.engage_seconds.saturating_sub(12));
    while Instant::now() < deadline && !server.text().contains("MATCH_RESULTS ") {
        thread::sleep(Duration::from_millis(500));
    }
    // A participant leaving ends an unfinished match; either way the server reports once.
    if let Some(last) = clients.last_mut() {
        last.stop();
    }
    let admission = server.wait(&["MP_UNIT_SCALE_ADMISSION"], Duration::from_secs(30));
    load("end", report);
    let mut launched: Vec<&Game> = vec![&server];
    launched.extend(clients.iter());
    launch_options_verdict(report, &id("DEV-LAUNCH-OPTIONS"), &launched);
    let server_text = server.text();
    for client in &mut clients {
        client.stop();
    }
    server.stop();
    drop(display);
    report.artifact(&id("server_log"), &server.log);
    for client in &clients {
        report.artifact(&id(&format!("{}_log", client.label)), &client.log);
    }
    let admission = admission?;

    // 1. Spawn: N units per seat plus the authored Unaligned side.
    let fixture = server_text
        .lines()
        .find(|l| l.contains("MP_UNIT_SCALE_FIXTURE"))
        .unwrap_or("no MP_UNIT_SCALE_FIXTURE line");
    let unaligned = server_text
        .lines()
        .find(|l| l.contains("MP_ACCEPTANCE_FIXTURE"))
        .and_then(|l| number(l, "unaligned_units"))
        .unwrap_or(-1.0);
    let content_units = server_text
        .lines()
        .find(|l| l.contains("MATCH_CONTENT_READY"))
        .and_then(|l| number(l, "units"))
        .unwrap_or(-1.0);
    let seat_units = (seats as u32 * per_seat) as f64;
    report.verdict(
        &id("UNITSCALE-SPAWNED"),
        number(fixture, "seat_units") == Some(seat_units)
            && number(fixture, "per_seat") == Some(f64::from(per_seat))
            && content_units == seat_units + unaligned
            && unaligned >= 0.0,
        format!(
            "content units={content_units} expected={seat_units}+{unaligned} unaligned; max_radius_cm={} min_opposing_gap_cm={}",
            field(fixture, "max_radius_cm").unwrap_or("?"),
            field(fixture, "min_opposing_gap_cm").unwrap_or("?")
        ),
    );

    // 2. Every seated unit's engagement order is admitted.
    let issued = number(&engage, "issued").unwrap_or(-1.0);
    let orders = orders.unwrap_or_default();
    let (accepted, rejected) = (
        number(&orders, "accepted").unwrap_or(-1.0),
        number(&orders, "rejected").unwrap_or(-1.0),
    );
    report.verdict(
        &id("UNITSCALE-ORDERS-ADMITTED"),
        issued == seat_units && accepted == issued && rejected == 0.0,
        format!(
            "issued={issued} accepted={accepted} rejected={rejected} expected={seat_units}; admission step {} us",
            field(&admission, "admission_step_us").unwrap_or("?")
        ),
    );

    // 3. Idle replication completeness: each client holds exactly the units its team sees.
    let server_samples = timed_net_samples(&server_text, "server", &generation);
    let mut complete = true;
    let mut detail = Vec::new();
    for (index, client) in clients.iter().enumerate() {
        let participant = index as u32 + 1;
        let visible = server_samples
            .iter()
            .filter(|(t, s, _)| s.participant == participant && t.as_str() < engage_time.as_str())
            .next_back()
            .map(|(_, s, _)| s.get("visible_units"));
        let client_text = client.text();
        let present = timed_net_samples(&client_text, "client", &generation)
            .into_iter()
            .filter(|(t, _, _)| t.as_str() < engage_time.as_str())
            .next_back()
            .map(|(_, s, _)| s.get("visible_units"));
        complete &= visible.is_some() && visible == present;
        detail.push(format!(
            "p{participant}: server_visible={visible:?} client_present={present:?}"
        ));
    }
    report.verdict(
        &id("UNITSCALE-REPLICATION-COMPLETE"),
        complete,
        detail.join("; "),
    );

    // 4. Authoritative step cost per phase against the 50 ms step.
    let step_summary = |phase: &str| {
        server_text
            .lines()
            .find(|l| l.contains("MP_UNIT_SCALE_STEP_SUMMARY") && field(l, "phase") == Some(phase))
            .map(str::to_owned)
            .unwrap_or_default()
    };
    let (idle, engaged) = (step_summary("idle"), step_summary("engage"));
    let overall = server_text
        .lines()
        .find(|l| l.contains("MP_SERVER_STEP_SUMMARY"))
        .unwrap_or_default()
        .to_owned();
    let budget = number(&overall, "budget_us").unwrap_or(50_000.0);
    let catchup = number(&overall, "catchup_cap_frames").unwrap_or(-1.0);
    let p99 = |line: &str| number(line, "p99_us").unwrap_or(f64::INFINITY);
    report.verdict(
        &id("UNITSCALE-STEP-WITHIN-BUDGET"),
        p99(&idle) < budget && p99(&engaged) < budget && catchup == 0.0,
        format!(
            "idle steps={} p99={}us max={}us; engage steps={} p99={}us max={}us; catch-up cap frames={catchup}; frame p95={}ms; budget={budget}us",
            field(&idle, "steps").unwrap_or("?"),
            field(&idle, "p99_us").unwrap_or("?"),
            field(&idle, "max_us").unwrap_or("?"),
            field(&engaged, "steps").unwrap_or("?"),
            field(&engaged, "p99_us").unwrap_or("?"),
            field(&engaged, "max_us").unwrap_or("?"),
            field(&overall, "frame_p95_ms").unwrap_or("?")
        ),
    );

    // 5. Bandwidth by phase and headroom under the configured send rate.
    let mut measured = true;
    let mut headroom = true;
    let mut detail = Vec::new();
    let (mut idle_down, mut engage_down, mut engage_peak, mut net_speed) =
        (0.0f64, 0.0f64, 0.0f64, 0.0f64);
    for participant in 1..=seats as u32 {
        let idle = phase_traffic(&server_samples, participant, None, Some(&engage_time));
        let fight = phase_traffic(&server_samples, participant, Some(&engage_time), None);
        match (idle, fight) {
            (Some(i), Some(f)) => {
                headroom &= f.net_speed > 0.0 && f.peak_down < UNIT_SCALE_HEADROOM * f.net_speed;
                idle_down = idle_down.max(i.down);
                engage_down = engage_down.max(f.down);
                engage_peak = engage_peak.max(f.peak_down);
                net_speed = net_speed.max(f.net_speed);
                detail.push(format!(
                    "p{participant}: idle down {:.0} up {:.0} B/s; engage down {:.0} up {:.0} peak {:.0} B/s over {:.1}s; net_speed {:.0}",
                    i.down, i.up, f.down, f.up, f.peak_down, f.seconds, f.net_speed
                ));
            }
            _ => {
                measured = false;
                detail.push(format!("p{participant}: missing samples"));
            }
        }
    }
    report.verdict(
        &id("UNITSCALE-BANDWIDTH-MEASURED"),
        measured,
        detail.join("; "),
    );
    report.verdict(
        &id("UNITSCALE-NET-HEADROOM"),
        measured && headroom,
        format!(
            "busiest second {engage_peak:.0} B/s vs {:.0}% of net_speed {net_speed:.0} B/s",
            UNIT_SCALE_HEADROOM * 100.0
        ),
    );

    // 6. The engagement really happened, and shot cues stayed fog-safe at this scale.
    let destroyed = number(&admission, "destroyed").or_else(|| {
        server_text
            .lines()
            .rfind(|l| l.contains("MP_UNIT_SCALE_STEP_SUMMARY"))
            .and_then(|l| number(l, "destroyed"))
    });
    let refs: Vec<&Game> = clients.iter().collect();
    let (received, counts, problems) = shot_cue_consistency(&server, &refs);
    let decisions: usize = counts.iter().sum();
    report.verdict(
        &id("UNITSCALE-ENGAGED"),
        destroyed.unwrap_or(0.0) > 0.0 && decisions > 0,
        format!("destroyed={destroyed:?}; shot-cue decisions={decisions}"),
    );
    report.verdict(
        &id("UNITSCALE-SHOT-CUE-CONSISTENT"),
        problems.is_empty() && decisions > 0,
        format!(
            "decisions exact/facing/none={counts:?}, received={received}; problems={}",
            problems
                .iter()
                .take(5)
                .cloned()
                .collect::<Vec<_>>()
                .join(" | ")
        ),
    );

    // Client frame time is a host-specific fact (software rendering under Xvfb), not a gate.
    let client_a = clients[0].text();
    let frames = |phase: Option<(f64, f64, usize)>| {
        phase.map_or("n/a".to_owned(), |(mean, max, n)| {
            format!("{mean:.1}/{max:.1}ms over {n}s")
        })
    };
    let idle_frames = frame_windows(&client_a, None, Some(&engage_time));
    let engage_frames = frame_windows(&client_a, Some(&engage_time), None);
    report.fact(
        &id("client_a_frame_mean_max"),
        format!(
            "idle {}; engage {}",
            frames(idle_frames),
            frames(engage_frames)
        ),
    );
    summary.push(format!(
        "{layout}\t{per_seat}\t{content_units}\t{idle_down:.0}\t{engage_down:.0}\t{engage_peak:.0}\t{net_speed:.0}\t{}\t{}\t{}\t{}\t{catchup}\t{:.1}\t{:.1}\t{}\t{decisions}\t{}",
        field(&idle, "p99_us").unwrap_or("?"),
        field(&engaged, "p99_us").unwrap_or("?"),
        field(&engaged, "max_us").unwrap_or("?"),
        field(&admission, "admission_step_us").unwrap_or("?"),
        idle_frames.map_or(f64::NAN, |f| f.0),
        engage_frames.map_or(f64::NAN, |f| f.0),
        destroyed.map_or("?".to_owned(), |d| d.to_string()),
        field(fixture, "min_opposing_gap_cm").unwrap_or("?"),
    ));
    Ok(())
}

/// An out-of-range per-seat count must fail admission closed (exit 3, no session).
fn unit_scale_refusal(options: &UnitScaleOptions, report: &mut Report) -> Result<(), String> {
    let evidence = options
        .evidence
        .join(format!("invalid-{}", MAX_UNITS_PER_SEAT + 1));
    fs::create_dir_all(&evidence).map_err(|e| e.to_string())?;
    let mut args = server_args(options.port + 2, "1v1");
    args.push(format!("-PolitikaMPUnitScale={}", MAX_UNITS_PER_SEAT + 1));
    let mut server =
        Game::launch_server(&options.server, "server-invalid-scale", &evidence, &args)?;
    let start = Instant::now();
    let mut status = None;
    while start.elapsed() < Duration::from_secs(180) {
        if let Ok(Some(code)) = server.child.try_wait() {
            status = Some(code);
            break;
        }
        thread::sleep(Duration::from_millis(200));
    }
    let text = server.text();
    let no_session =
        !text.contains("MP_MATCH_AWAITING_PARTICIPANTS") && !text.contains("MATCH_CONTENT_READY");
    report.verdict(
        "UNITSCALE-INVALID-REFUSED",
        status.and_then(|s| s.code()) == Some(3)
            && text.contains(&format!(
                "MP_UNIT_SCALE_INVALID value={}",
                MAX_UNITS_PER_SEAT + 1
            ))
            && text.contains("reason=MP.UNIT_SCALE.INVALID")
            && text.contains("MP_DEDICATED_ADMISSION_FAILED")
            && no_session,
        format!("exit={status:?}; no session/units={no_session}"),
    );
    launch_options_verdict(report, "UNITSCALE-INVALID-DEV-LAUNCH-OPTIONS", &[&server]);
    report.artifact("invalid_scale_server_log", &server.log);
    server.stop();
    Ok(())
}

fn run_unit_scale() -> ! {
    let options = match parse_unit_scale_options() {
        Ok(options) => options,
        Err(error) => {
            eprintln!("{error}");
            std::process::exit(2);
        }
    };
    let mut report = Report {
        schema: Some("politika.mp2.unit-scale-evidence.v1"),
        ..Report::default()
    };
    for (name, binary) in [("server", &options.server), ("client", &options.client)] {
        report.fact(&format!("{name}_binary"), binary.display().to_string());
        if let Ok(digest) = sha256(binary) {
            report.fact(&format!("{name}_executable_sha256"), digest);
        }
        if let Some(archive) = binary.ancestors().nth(4)
            && let Ok(identity) = fs::read_to_string(archive.join("BuildIdentity.json"))
        {
            report.fact(
                &format!("{name}_build_identity"),
                identity.trim().to_owned(),
            );
        }
    }
    report.fact("engage_seconds", options.engage_seconds.to_string());
    if let Err(error) = unit_scale_refusal(&options, &mut report) {
        report.mark("SESSION-unit-scale-refusal", "FAIL", error);
    }
    let mut summary = vec![
        "layout\tper_seat\tunits\tidle_down_Bps\tengage_down_Bps\tengage_peak_down_Bps\tnet_speed\tidle_p99_us\tengage_p99_us\tengage_max_us\tadmission_step_us\tcatchup_frames\tclient_a_idle_frame_ms\tclient_a_engage_frame_ms\tdestroyed\tshot_cues\tmin_opposing_gap_cm".to_owned(),
    ];
    let mut port = options.port;
    for layout in &options.layouts {
        for &per_seat in &options.units {
            port += 10;
            if let Err(error) =
                unit_scale_run(&options, &mut report, layout, per_seat, port, &mut summary)
            {
                report.mark(
                    &format!("{layout}_n{per_seat}_SESSION"),
                    if error.starts_with("BLOCKED") {
                        "BLOCKED"
                    } else {
                        "FAIL"
                    },
                    error,
                );
            }
        }
    }
    let table = summary.join("\n") + "\n";
    print!("{table}");
    let _ = fs::write(options.evidence.join("unit-scale-summary.tsv"), &table);
    let path = options.evidence.join("unit-scale-evidence.json");
    if let Err(error) = report.write(&path) {
        eprintln!("write evidence: {error}");
        std::process::exit(2);
    }
    println!("evidence: {}", path.display());
    std::process::exit(if report.failed() { 1 } else { 0 });
}

// ---------------------------------------------------------------------------------------
// GRP 4C baseline (RTS_SELECTION_AND_GROUP_ORDERS.md): `--group-orders SERVER CLIENT
// [--evidence DIR] [--port N]`.
//
// A dedicated 1v1 skirmish holds 20 units per seat (the test map cap) with the unit-scale
// fixture kept idle. Seat 1's client renders on Xvfb and is driven with real X input: a box
// drag selects its army, Shift+click toggles one unit out, Esc cancels a drag, a right-click
// orders the selection, and a click on empty ground clears it. Client selection logs are
// checked against the server's authoritative ownership and admitted commands.
// ---------------------------------------------------------------------------------------

const GROUP_UNITS_PER_SEAT: usize = 20;

fn selection_ids(line: &str) -> Vec<u64> {
    let mut ids: Vec<u64> = field(line, "ids")
        .unwrap_or("")
        .split(',')
        .filter_map(|v| v.parse().ok())
        .collect();
    ids.sort_unstable();
    ids
}

impl Game {
    fn xdo_here(&self, args: &[&str]) -> Result<(), String> {
        let display = self.display.as_deref().ok_or("no display")?;
        xdo(display, args).map(|_| ())
    }

    fn pointer_to(&self, x: f64, y: f64) -> Result<(), String> {
        let window = self.window.as_deref().ok_or("no window")?;
        self.xdo_here(&[
            "mousemove",
            "--window",
            window,
            "--sync",
            &format!("{}", x.round()),
            &format!("{}", y.round()),
        ])
    }

    /// Presses at `from`, moves to `to` in steps the game sees across several frames, and
    /// returns with the button still held.
    fn drag_hold(&self, from: (f64, f64), to: (f64, f64)) -> Result<(), String> {
        self.pointer_to(from.0, from.1)?;
        thread::sleep(Duration::from_millis(150));
        self.xdo_here(&["mousedown", "1"])?;
        thread::sleep(Duration::from_millis(150));
        for step in 1..=8 {
            let t = f64::from(step) / 8.0;
            self.pointer_to(from.0 + (to.0 - from.0) * t, from.1 + (to.1 - from.1) * t)?;
            thread::sleep(Duration::from_millis(60));
        }
        thread::sleep(Duration::from_millis(250));
        Ok(())
    }

    fn release(&self) -> Result<(), String> {
        self.xdo_here(&["mouseup", "1"])?;
        thread::sleep(Duration::from_millis(250));
        Ok(())
    }

    fn modified_click(&self, x: f64, y: f64, modifier: &str) -> Result<(), String> {
        self.pointer_to(x, y)?;
        thread::sleep(Duration::from_millis(150));
        self.xdo_here(&["keydown", modifier])?;
        thread::sleep(Duration::from_millis(120));
        let clicked = self.xdo_here(&["click", "1"]);
        thread::sleep(Duration::from_millis(120));
        self.xdo_here(&["keyup", modifier])?;
        clicked
    }
}

#[allow(clippy::too_many_lines)]
fn group_orders_session(options: &DedicatedOptions, report: &mut Report) -> Result<(), String> {
    let client_options = &options.client;
    let evidence = &client_options.evidence;
    let port = client_options.port;
    let mut args = server_args(port, "1v1");
    args.push(format!("-PolitikaMPUnitScale={GROUP_UNITS_PER_SEAT}"));
    args.push("-PolitikaMPUnitScaleHold".into());
    let mut server = Game::launch_server(&options.server, "server", evidence, &args)?;
    report.fact("server_command", server.command.join(" "));
    server.wait(&["MP_DEDICATED_SERVER_READY"], Duration::from_secs(180))?;
    let awaiting = server.wait(&["MP_MATCH_AWAITING_PARTICIPANTS"], Duration::from_secs(60))?;
    let generation = field(&awaiting, "generation").unwrap_or("0").to_owned();
    let fixture = server.wait(&["MP_UNIT_SCALE_FIXTURE"], Duration::from_secs(5))?;

    let display_a = Display::start(evidence, "client-a")?;
    let mut client_a = Game::launch(
        &client_options.binary,
        "client-a",
        evidence,
        &client_args(client_options, true, false, false),
        Some(&display_a.name()),
    )?;
    report.fact("client_a_command", client_a.command.join(" "));
    server.wait(
        &["MP_PARTICIPANT_BOUND participant=1", "remote=1"],
        Duration::from_secs(90),
    )?;
    let mut client_b = Game::launch(
        &client_options.binary,
        "client-b",
        evidence,
        &client_args(client_options, false, false, false),
        None,
    )?;
    report.fact("client_b_command", client_b.command.join(" "));
    server.wait(
        &["MP_PARTICIPANT_BOUND participant=2", "remote=1"],
        Duration::from_secs(90),
    )?;
    server.wait(
        &["MP_MATCH_ACTIVE", &format!("generation={generation}")],
        Duration::from_secs(30),
    )?;
    client_a.wait(
        &[
            "MP_CLIENT_MATCH_OBSERVED",
            "map=map.test.multiplayer_skirmish",
        ],
        Duration::from_secs(60),
    )?;
    client_a.attach_window()?;
    // Replication of both armies and the camera settling on seat 1's start.
    thread::sleep(Duration::from_secs(6));

    let result = (|| -> Result<(), String> {
        let owners = entity_owners(&server.text());
        let own: Vec<u64> = owners
            .iter()
            .filter(|(_, o)| *o == 1)
            .map(|(e, _)| *e)
            .collect();
        let foreign: Vec<u64> = owners
            .iter()
            .filter(|(_, o)| *o != 1)
            .map(|(e, _)| *e)
            .collect();
        report.verdict(
            "GRP-FIXTURE-20-PER-SEAT",
            own.len() == GROUP_UNITS_PER_SEAT
                && field(&fixture, "hold") == Some("1")
                && field(&fixture, "per_seat") == Some("20"),
            format!(
                "participant 1 units={} other units={}; {}",
                own.len(),
                foreign.len(),
                fixture.trim()
            ),
        );
        report.artifact(
            "client_a_start_png",
            &client_a.screenshot(evidence, "01-start")?,
        );

        // GRP-02/03: a box drag over the whole view selects exactly the 20 own units.
        let before_box = client_a.offset();
        client_a.drag_hold(
            (24.0, 24.0),
            (f64::from(WIDTH) - 24.0, f64::from(HEIGHT) - 24.0),
        )?;
        report.artifact(
            "client_a_dragging_png",
            &client_a.screenshot(evidence, "02-dragging")?,
        );
        client_a.release()?;
        let boxed = client_a.wait_line(
            &["RTS_SELECTION_SET", "source=box", "op=replace"],
            Duration::from_secs(5),
            before_box,
        )?;
        let boxed_ids = selection_ids(&boxed);
        report.verdict(
            "GRP-02-BOX-SELECTS-OWN-UNITS",
            boxed_ids == own && field(&boxed, "count") == Some("20"),
            format!(
                "selected {} ids; own {} ids; foreign selected={}; {}",
                boxed_ids.len(),
                own.len(),
                boxed_ids.iter().filter(|id| foreign.contains(id)).count(),
                boxed.trim()
            ),
        );
        thread::sleep(Duration::from_millis(500));
        report.artifact(
            "client_a_box_selected_png",
            &client_a.screenshot(evidence, "03-box-selected")?,
        );

        // Shift+click toggles one selected unit out. Pick the unit farthest from its
        // nearest neighbour on screen so the click cannot land on another unit.
        let screens: Vec<(u64, f64, f64)> = client_a.text()[before_box..]
            .lines()
            .filter(|l| l.contains("RTS_SELECTION_SCREEN"))
            .filter_map(|l| {
                Some((
                    field(l, "unit")?.parse().ok()?,
                    number(l, "x")?,
                    number(l, "y")?,
                ))
            })
            .filter(|(_, x, y)| {
                *x > 40.0
                    && *y > 200.0
                    && *x < f64::from(WIDTH) - 40.0
                    && *y < f64::from(HEIGHT) - 40.0
            })
            .collect();
        let toggle = screens
            .iter()
            .map(|a| {
                let nearest = screens
                    .iter()
                    .filter(|b| b.0 != a.0)
                    .map(|b| (a.1 - b.1).hypot(a.2 - b.2))
                    .fold(f64::MAX, f64::min);
                (a, nearest)
            })
            .max_by(|a, b| a.1.total_cmp(&b.1))
            .map(|(a, _)| *a)
            .ok_or("no on-screen position for any selected unit")?;
        report.fact(
            "toggle_unit",
            format!("entity={} x={:.0} y={:.0}", toggle.0, toggle.1, toggle.2),
        );
        let before_toggle = client_a.offset();
        client_a.modified_click(toggle.1, toggle.2, "shift")?;
        let toggled = client_a.wait_line(
            &["RTS_SELECTION_SET", "source=click", "op=toggle"],
            Duration::from_secs(5),
            before_toggle,
        )?;
        let selected: Vec<u64> = selection_ids(&toggled);
        let expected: Vec<u64> = own.iter().copied().filter(|id| *id != toggle.0).collect();
        report.verdict(
            "GRP-TOGGLE-REMOVES-ONE",
            selected == expected,
            format!(
                "{} selected after Shift+click; {}",
                selected.len(),
                toggled.trim()
            ),
        );

        // GRP-11: Esc during a drag cancels it; no selection change and the match continues.
        let before_cancel = client_a.offset();
        client_a.drag_hold((60.0, 640.0), (360.0, 690.0))?;
        client_a.key("Escape")?;
        thread::sleep(Duration::from_millis(300));
        client_a.release()?;
        thread::sleep(Duration::from_millis(500));
        let cancel_text = client_a.text()[before_cancel..].to_owned();
        let cancelled = cancel_text.lines().any(|l| {
            l.contains("RTS_SELECTION_CANCELLED")
                && l.contains("reason=Escape")
                && l.contains("box=1")
        });
        let changed = cancel_text.lines().any(|l| l.contains("RTS_SELECTION_SET"));
        let left = cancel_text.contains("LoadMap: /Game/Maps/MainMenu") || !client_a.alive();
        report.verdict(
            "GRP-11-ESC-CANCELS-DRAG",
            cancelled && !changed && !left,
            format!("cancelled={cancelled} selection_changed={changed} left_match={left}"),
        );

        // GRP-04: a right-click on ground orders every selected unit; each request is
        // admitted on its own, and the 19-unit batch fits the pending bound.
        // The control point: walkable ground inside the playable area, away from the army.
        let order_point = wait_screen(
            &mut client_a,
            |t| objective_screen(t, "MP_CLIENT_OBJECTIVE"),
            "objective",
        )?;
        report.fact(
            "order_point",
            format!("{:.0},{:.0}", order_point.0, order_point.1),
        );
        let before_order_client = client_a.offset();
        let before_order_server = server.offset();
        client_a.click(order_point.0, order_point.1, "3")?;
        let sent = client_a.wait_line(
            &["RTS_GROUP_ORDER_SENT", "order=0"],
            Duration::from_secs(5),
            before_order_client,
        )?;
        let outcome = client_a.wait_line(
            &["RTS_GROUP_ORDER_OUTCOME", "order=0"],
            Duration::from_secs(15),
            before_order_client,
        )?;
        thread::sleep(Duration::from_millis(400));
        report.artifact(
            "client_a_group_order_png",
            &client_a.screenshot(evidence, "04-group-order")?,
        );
        let server_text = server.text()[before_order_server..].to_owned();
        let mut received: Vec<u64> = server_text
            .lines()
            .filter(|l| {
                l.contains("MP_COMMAND_RECEIVED")
                    && l.contains(" participant=1 ")
                    && l.contains(" order=0 ")
            })
            .filter_map(|l| field(l, "issuer")?.parse().ok())
            .collect();
        received.sort_unstable();
        let intake_rejected = server_text
            .lines()
            .filter(|l| l.contains("MP_COMMAND_REJECTED_AT_INTAKE") && l.contains("participant=1 "))
            .count();
        report.verdict(
            "GRP-04-GROUP-ORDER-ADMITTED",
            field(&sent, "issuers") == Some("19")
                && received == expected
                && intake_rejected == 0
                && field(&outcome, "expected") == Some("19")
                && field(&outcome, "accepted") == Some("19")
                && field(&outcome, "rejected") == Some("0"),
            format!(
                "server received {} move intents (intake rejections {intake_rejected}); {}; {}",
                received.len(),
                sent.trim(),
                outcome.trim()
            ),
        );
        report.verdict(
            "GRP-08-UNSELECTED-UNIT-UNTOUCHED",
            !received.contains(&toggle.0),
            format!("toggled-out unit {} received no order", toggle.0),
        );

        // Every ordered unit starts moving; arrivals are recorded as facts (one shared
        // destination point, no formation spacing in this slice).
        thread::sleep(Duration::from_secs(10));
        report.artifact(
            "client_a_moved_png",
            &client_a.screenshot(evidence, "05-moved")?,
        );
        let server_text = server.text()[before_order_server..].to_owned();
        let changes: Vec<(u64, String, String, String)> = server_text
            .lines()
            .filter(|l| l.contains("RTS_GAMEPLAY_EVENT kind=OrderChanged"))
            .filter_map(|l| {
                Some((
                    field(l, "entity")?.parse().ok()?,
                    field(l, "previous")?.to_owned(),
                    field(l, "current")?.to_owned(),
                    field(l, "reason")?.to_owned(),
                ))
            })
            .collect();
        let started = expected
            .iter()
            .filter(|id| changes.iter().any(|c| c.0 == **id && c.2 == "1"))
            .count();
        let ended = |reason: &str| {
            expected
                .iter()
                .filter(|id| {
                    changes
                        .iter()
                        .any(|c| c.0 == **id && c.1 == "1" && c.3 == reason)
                })
                .count()
        };
        report.fact(
            "move_end_reasons",
            format!(
                "arrived={} obstructed={} navigation_failed={} still_moving_or_other={}",
                ended("5"),
                ended("6"),
                ended("7"),
                expected.len() - ended("5") - ended("6") - ended("7")
            ),
        );
        report.verdict(
            "GRP-04-EVERY-MEMBER-MOVES",
            started == expected.len() && ended("7") == 0,
            format!(
                "{started}/{} entered Move; navigation failures {}",
                expected.len(),
                ended("7")
            ),
        );

        // A click on empty ground clears the selection; Stop with nothing selected sends nothing.
        let before_clear = client_a.offset();
        client_a.click(f64::from(WIDTH) * 0.12, f64::from(HEIGHT) * 0.80, "1")?;
        let cleared = client_a.wait_line(
            &["RTS_SELECTION_SET", "source=click", "op=replace"],
            Duration::from_secs(5),
            before_clear,
        )?;
        client_a.key("x")?;
        thread::sleep(Duration::from_secs(2));
        let sent_after_clear = client_a.text()[before_clear..]
            .lines()
            .filter(|l| l.contains("RTS_COMMAND_INTENT_SENT"))
            .count();
        report.verdict(
            "GRP-EMPTY-SELECTION-SENDS-NOTHING",
            field(&cleared, "count") == Some("0") && sent_after_clear == 0,
            format!("{}; intents after clear={sent_after_clear}", cleared.trim()),
        );
        report.artifact(
            "client_a_cleared_png",
            &client_a.screenshot(evidence, "06-cleared")?,
        );
        Ok(())
    })();
    launch_options_verdict(
        report,
        "GRP-DEV-LAUNCH-OPTIONS",
        &[&server, &client_a, &client_b],
    );
    report.artifact("server_log", &server.log);
    report.artifact("client_a_log", &client_a.log);
    report.artifact("client_b_log", &client_b.log);
    client_a.stop();
    client_b.stop();
    server.stop();
    drop(display_a);
    result
}

// 4C-G control groups (RTS_SELECTION_AND_GROUP_ORDERS.md §2B/§2D): `--control-groups SERVER
// CLIENT [--evidence DIR] [--port N]`.
//
// Same dedicated 1v1 fixture as `--group-orders` (20 idle units per seat). Seat 1's client is
// driven with real X input: Ctrl/Shift/Alt+digit edit slots, digits recall them, a double tap
// centres the camera, Ctrl+click and double-click select the on-screen archetype, Tab cycles
// subgroups and the bottom group bar is clicked. Client logs are checked against the server's
// ownership and received commands; no group operation may send an order.
// ---------------------------------------------------------------------------------------

/// Bar slot centre in the 1280x720 client window, mirroring `GroupBarSlotRect` at UI scale 1.
fn group_bar_slot_centre(slot: usize) -> (f64, f64) {
    let (w, h, gap, bottom, min_left) = (64.0, 46.0, 6.0, 22.0, 446.0);
    let total = 10.0 * w + 9.0 * gap;
    let left = ((f64::from(WIDTH) - total) * 0.5).max(min_left);
    let position = (slot + 9) % 10;
    (
        left + position as f64 * (w + gap) + w * 0.5,
        f64::from(HEIGHT) - bottom - h * 0.5,
    )
}

fn group_counts(line: &str) -> Vec<u32> {
    field(line, "counts")
        .unwrap_or("")
        .split(',')
        .filter_map(|v| v.parse().ok())
        .collect()
}

impl Game {
    /// Holds a modifier across a separate key press so the game sees both in one state.
    fn modified_key(&self, modifier: &str, key: &str) -> Result<(), String> {
        self.xdo_here(&["keydown", modifier])?;
        thread::sleep(Duration::from_millis(150));
        let pressed = self.xdo_here(&["key", key]);
        thread::sleep(Duration::from_millis(150));
        self.xdo_here(&["keyup", modifier])?;
        thread::sleep(Duration::from_millis(250));
        pressed
    }
}

#[allow(clippy::too_many_lines)]
fn control_groups_session(options: &DedicatedOptions, report: &mut Report) -> Result<(), String> {
    let client_options = &options.client;
    let evidence = &client_options.evidence;
    let port = client_options.port;
    let mut args = server_args(port, "1v1");
    args.push(format!("-PolitikaMPUnitScale={GROUP_UNITS_PER_SEAT}"));
    args.push("-PolitikaMPUnitScaleHold".into());
    let mut server = Game::launch_server(&options.server, "server", evidence, &args)?;
    report.fact("server_command", server.command.join(" "));
    server.wait(&["MP_DEDICATED_SERVER_READY"], Duration::from_secs(180))?;
    let awaiting = server.wait(&["MP_MATCH_AWAITING_PARTICIPANTS"], Duration::from_secs(60))?;
    let generation = field(&awaiting, "generation").unwrap_or("0").to_owned();
    server.wait(&["MP_UNIT_SCALE_FIXTURE"], Duration::from_secs(5))?;
    let display_a = Display::start(evidence, "client-a")?;
    let mut client_a = Game::launch(
        &client_options.binary,
        "client-a",
        evidence,
        &client_args(client_options, true, false, false),
        Some(&display_a.name()),
    )?;
    report.fact("client_a_command", client_a.command.join(" "));
    server.wait(
        &["MP_PARTICIPANT_BOUND participant=1", "remote=1"],
        Duration::from_secs(90),
    )?;
    let mut client_b = Game::launch(
        &client_options.binary,
        "client-b",
        evidence,
        &client_args(client_options, false, false, false),
        None,
    )?;
    server.wait(
        &["MP_PARTICIPANT_BOUND participant=2", "remote=1"],
        Duration::from_secs(90),
    )?;
    server.wait(
        &["MP_MATCH_ACTIVE", &format!("generation={generation}")],
        Duration::from_secs(30),
    )?;
    client_a.wait(
        &[
            "MP_CLIENT_MATCH_OBSERVED",
            "map=map.test.multiplayer_skirmish",
        ],
        Duration::from_secs(60),
    )?;
    client_a.attach_window()?;
    thread::sleep(Duration::from_secs(6));

    let result = (|| -> Result<(), String> {
        let owners = entity_owners(&server.text());
        let own: Vec<u64> = owners
            .iter()
            .filter(|(_, o)| *o == 1)
            .map(|(e, _)| *e)
            .collect();
        report.fact("own_units", own.len().to_string());
        let wait = |game: &mut Game, fragments: &[&str], from: usize| {
            game.wait_line(fragments, Duration::from_secs(5), from)
        };

        // Setup: the whole army by box, as in the 4C baseline.
        let before_box = client_a.offset();
        client_a.drag_hold(
            (24.0, 24.0),
            (f64::from(WIDTH) - 24.0, f64::from(HEIGHT) - 24.0),
        )?;
        client_a.release()?;
        let boxed = wait(
            &mut client_a,
            &["RTS_SELECTION_SET", "source=box"],
            before_box,
        )?;
        report.verdict(
            "CG-SETUP-BOX-20",
            selection_ids(&boxed) == own && own.len() == GROUP_UNITS_PER_SEAT,
            boxed.trim().to_owned(),
        );
        // Unit screen positions clear of the HUD text, the command panel and the group bar.
        let screens: Vec<(u64, f64, f64)> = client_a.text()[before_box..]
            .lines()
            .filter(|l| l.contains("RTS_SELECTION_SCREEN"))
            .filter_map(|l| {
                Some((
                    field(l, "unit")?.parse().ok()?,
                    number(l, "x")?,
                    number(l, "y")?,
                ))
            })
            .filter(|(_, x, y)| {
                *x > 40.0 && *x < f64::from(WIDTH) - 40.0 && *y > 220.0 && *y < 500.0
            })
            .collect();
        // Greedy pick of five well-separated units so no click lands on a neighbour.
        let mut picked: Vec<(u64, f64, f64)> = Vec::new();
        while picked.len() < 5 {
            let next = screens
                .iter()
                .filter(|s| !picked.iter().any(|p| p.0 == s.0))
                .map(|s| {
                    let nearest = screens
                        .iter()
                        .filter(|o| o.0 != s.0)
                        .map(|o| (s.1 - o.1).hypot(s.2 - o.2))
                        .fold(f64::MAX, f64::min);
                    let from_picked = picked
                        .iter()
                        .map(|p| (s.1 - p.1).hypot(s.2 - p.2))
                        .fold(f64::MAX, f64::min);
                    (*s, nearest.min(from_picked * 4.0))
                })
                .max_by(|a, b| a.1.total_cmp(&b.1))
                .map(|(s, _)| s)
                .ok_or("fewer than five clickable own units on screen")?;
            picked.push(next);
        }
        let mut five: Vec<u64> = picked.iter().map(|p| p.0).collect();
        five.sort_unstable();
        report.fact(
            "picked_units",
            picked
                .iter()
                .map(|p| format!("{}@{:.0},{:.0}", p.0, p.1, p.2))
                .collect::<Vec<_>>()
                .join(" "),
        );

        // GRP-13: Ctrl+1 stores the army.
        let before_ops = client_a.offset();
        let before_ops_server = server.offset();
        let at = client_a.offset();
        client_a.modified_key("ctrl", "1")?;
        let set1 = wait(&mut client_a, &["RTS_CONTROL_GROUP_ASSIGN", "slot=1 "], at)?;
        report.verdict(
            "CG-13-CTRL-REPLACE",
            field(&set1, "op") == Some("replace")
                && selection_ids(&set1) == own
                && field(&set1, "generation") == Some(generation.as_str()),
            set1.trim().to_owned(),
        );

        // Five units by click and Shift+click, then Ctrl+2: overlapping membership.
        let at = client_a.offset();
        client_a.click(picked[0].1, picked[0].2, "1")?;
        wait(
            &mut client_a,
            &["RTS_SELECTION_SET", "source=click", "count=1 "],
            at,
        )?;
        for p in &picked[1..] {
            let at = client_a.offset();
            client_a.modified_click(p.1, p.2, "shift")?;
            wait(
                &mut client_a,
                &["RTS_SELECTION_SET", "source=click", "op=toggle"],
                at,
            )?;
        }
        let five_selected = client_a.text()[before_ops..]
            .lines()
            .rev()
            .find(|l| l.contains("RTS_SELECTION_SET"))
            .map(selection_ids)
            .unwrap_or_default();
        report.verdict(
            "CG-SETUP-FIVE",
            five_selected == five,
            format!("selected {five_selected:?}; picked {five:?}"),
        );
        let after_pick = client_a.offset();
        let at = client_a.offset();
        client_a.modified_key("ctrl", "2")?;
        let set2 = wait(&mut client_a, &["RTS_CONTROL_GROUP_ASSIGN", "slot=2 "], at)?;
        let counts = group_counts(&set2);
        report.verdict(
            "CG-14-OVERLAP",
            selection_ids(&set2) == five && counts.get(1) == Some(&20) && counts.get(2) == Some(&5),
            set2.trim().to_owned(),
        );
        // Alt+3 moves the five out of groups 1 and 2 into 3.
        let at = client_a.offset();
        client_a.modified_key("alt", "3")?;
        let set3 = wait(&mut client_a, &["RTS_CONTROL_GROUP_ASSIGN", "slot=3 "], at)?;
        let counts = group_counts(&set3);
        report.verdict(
            "CG-13-ALT-TRANSFER",
            field(&set3, "op") == Some("transfer")
                && selection_ids(&set3) == five
                && counts.get(1) == Some(&15)
                && counts.get(2) == Some(&0)
                && counts.get(3) == Some(&5),
            set3.trim().to_owned(),
        );
        // Shift+1 adds them back to 1 without taking them from 3.
        let at = client_a.offset();
        client_a.modified_key("shift", "1")?;
        let add1 = wait(&mut client_a, &["RTS_CONTROL_GROUP_ASSIGN", "slot=1 "], at)?;
        let counts = group_counts(&add1);
        report.verdict(
            "CG-13-SHIFT-UNION",
            field(&add1, "op") == Some("union")
                && selection_ids(&add1) == own
                && counts.get(1) == Some(&20)
                && counts.get(3) == Some(&5),
            add1.trim().to_owned(),
        );
        let reselected = client_a.text()[after_pick..]
            .lines()
            .filter(|l| l.contains("RTS_SELECTION_SET"))
            .count();
        report.verdict(
            "CG-13-ASSIGN-KEEPS-SELECTION",
            reselected == 0,
            format!("selection changes during Ctrl+2, Alt+3, Shift+1: {reselected}"),
        );
        report.artifact(
            "assigned_png",
            &client_a.screenshot(evidence, "01-assigned")?,
        );

        // GRP-15: clear, then 1 recalls the army; an empty slot keeps the selection.
        let at = client_a.offset();
        client_a.click(f64::from(WIDTH) * 0.12, f64::from(HEIGHT) * 0.80, "1")?;
        wait(
            &mut client_a,
            &["RTS_SELECTION_SET", "source=click", "count=0 "],
            at,
        )?;
        let at = client_a.offset();
        client_a.key("1")?;
        let recall = wait(&mut client_a, &["RTS_CONTROL_GROUP_RECALL", "slot=1 "], at)?;
        let recalled = wait(&mut client_a, &["RTS_SELECTION_SET", "source=group"], at)?;
        report.verdict(
            "CG-15-RECALL",
            field(&recall, "selected") == Some("20")
                && field(&recall, "focus") == Some("0")
                && selection_ids(&recalled) == own,
            format!("{}; {}", recall.trim(), recalled.trim()),
        );
        let at = client_a.offset();
        client_a.key("7")?;
        let empty = wait(&mut client_a, &["RTS_CONTROL_GROUP_RECALL", "slot=7 "], at)?;
        thread::sleep(Duration::from_millis(300));
        let changed = client_a.text()[at..].contains("RTS_SELECTION_SET");
        report.verdict(
            "CG-16-EMPTY-SLOT-KEEPS-SELECTION",
            field(&empty, "members") == Some("0")
                && field(&empty, "selected") == Some("20")
                && !changed,
            format!("{}; selection_changed={changed}", empty.trim()),
        );

        // GRP-17: Ctrl+click and double-click select the archetype on screen; Tab cycles.
        let at = client_a.offset();
        client_a.click(picked[1].1, picked[1].2, "1")?;
        wait(
            &mut client_a,
            &["RTS_SELECTION_SET", "source=click", "count=1 "],
            at,
        )?;
        thread::sleep(Duration::from_millis(600));
        let at = client_a.offset();
        client_a.modified_click(picked[0].1, picked[0].2, "ctrl")?;
        let same = wait(
            &mut client_a,
            &["RTS_SELECTION_SET", "source=ctrl_click"],
            at,
        )?;
        report.verdict(
            "CG-17-CTRL-CLICK-SAME-TYPE",
            selection_ids(&same) == own,
            same.trim().to_owned(),
        );
        thread::sleep(Duration::from_millis(600));
        let at = client_a.offset();
        client_a.pointer_to(picked[2].1, picked[2].2)?;
        thread::sleep(Duration::from_millis(150));
        client_a.xdo_here(&["click", "--repeat", "2", "--delay", "150", "1"])?;
        let double = wait(
            &mut client_a,
            &["RTS_SELECTION_SET", "source=double_click"],
            at,
        )?;
        report.verdict(
            "CG-17-DOUBLE-CLICK-SAME-TYPE",
            selection_ids(&double) == own,
            double.trim().to_owned(),
        );
        let at = client_a.offset();
        client_a.key("Tab")?;
        let tab = wait(&mut client_a, &["RTS_SUBGROUP_CYCLE"], at)?;
        report.verdict(
            "CG-17-TAB-SINGLE-ARCHETYPE",
            field(&tab, "subgroups") == Some("1") && field(&tab, "units") == Some("20"),
            format!(
                "{} (mixed archetypes: Automation ControlGroups)",
                tab.trim()
            ),
        );

        // GRP-15: pan away, then double-tap 3 centres the camera on group 3.
        client_a.xdo_here(&["keydown", "d"])?;
        thread::sleep(Duration::from_millis(1300));
        client_a.xdo_here(&["keyup", "d"])?;
        thread::sleep(Duration::from_millis(400));
        report.artifact(
            "panned_png",
            &client_a.screenshot(evidence, "02-panned-away")?,
        );
        let at = client_a.offset();
        client_a.xdo_here(&["key", "--delay", "120", "3", "3"])?;
        let focus = wait(&mut client_a, &["RTS_CONTROL_GROUP_FOCUS", "slot=3 "], at)?;
        let focus_recall = wait(
            &mut client_a,
            &["RTS_CONTROL_GROUP_RECALL", "slot=3 ", "focus=1"],
            at,
        )?;
        let moved = distance(
            (
                number(&focus, "before_x").unwrap_or(0.0),
                number(&focus, "before_y").unwrap_or(0.0),
            ),
            (
                number(&focus, "after_x").unwrap_or(0.0),
                number(&focus, "after_y").unwrap_or(0.0),
            ),
        );
        let error = number(&focus, "error_cm").unwrap_or(f64::MAX);
        report.verdict(
            "CG-15-DOUBLE-TAP-FOCUS",
            field(&focus, "units") == Some("5")
                && field(&focus_recall, "selected") == Some("5")
                && moved > 300.0
                && error < 30.0,
            format!(
                "camera moved {moved:.0} cm, centre error {error:.1} cm; {}",
                focus.trim()
            ),
        );
        thread::sleep(Duration::from_millis(400));
        report.artifact(
            "focused_png",
            &client_a.screenshot(evidence, "03-focused-group-3")?,
        );

        // GRP-21: the bar recalls on click and captures a right-click.
        let (bx, by) = group_bar_slot_centre(1);
        let at = client_a.offset();
        client_a.click(bx, by, "1")?;
        let bar = wait(
            &mut client_a,
            &["RTS_CONTROL_GROUP_RECALL", "slot=1 ", "source=bar"],
            at,
        )?;
        report.verdict(
            "CG-21-BAR-CLICK-RECALLS",
            field(&bar, "selected") == Some("20"),
            format!("clicked {bx:.0},{by:.0}; {}", bar.trim()),
        );
        let at = client_a.offset();
        client_a.click(bx, by, "3")?;
        thread::sleep(Duration::from_millis(1500));
        let ordered = client_a.text()[at..].contains("RTS_GROUP_ORDER_SENT");
        report.verdict(
            "CG-21-BAR-CAPTURES-RIGHT-CLICK",
            !ordered,
            format!("right-click on slot 1 sent an order: {ordered}"),
        );
        client_a.hover(f64::from(WIDTH) * 0.5, f64::from(HEIGHT) * 0.4)?;
        thread::sleep(Duration::from_millis(400));
        report.artifact("bar_png", &client_a.screenshot(evidence, "04-bar-slot-1")?);

        // GRP-15: no group operation so far sent an intent or reached the server.
        let intents = client_a.text()[before_ops..]
            .lines()
            .filter(|l| l.contains("RTS_COMMAND_INTENT_SENT"))
            .count();
        let received = server.text()[before_ops_server..]
            .lines()
            .filter(|l| l.contains("MP_COMMAND_RECEIVED") && l.contains(" participant=1 "))
            .count();
        report.verdict(
            "CG-15-NO-ORDERS-FROM-GROUP-OPS",
            intents == 0 && received == 0,
            format!("client intents {intents}; server commands from participant 1 {received}"),
        );

        // GRP-14: an order from the recalled overlapping group has one request per unit.
        let at = client_a.offset();
        client_a.xdo_here(&["key", "--delay", "120", "1", "1"])?;
        wait(&mut client_a, &["RTS_CONTROL_GROUP_FOCUS", "slot=1 "], at)?;
        thread::sleep(Duration::from_millis(600));
        let order_point = wait_screen(
            &mut client_a,
            |t| objective_screen(t, "MP_CLIENT_OBJECTIVE"),
            "objective",
        )?;
        let (lx, ly) = group_bar_slot_centre(1);
        if order_point.1 > ly - 60.0 && order_point.0 > lx - 60.0 {
            return Err(format!(
                "objective at {order_point:?} is behind the group bar"
            ));
        }
        let at = client_a.offset();
        let at_server = server.offset();
        client_a.click(order_point.0, order_point.1, "3")?;
        let sent = wait(&mut client_a, &["RTS_GROUP_ORDER_SENT", "order=0"], at)?;
        let outcome = client_a.wait_line(
            &["RTS_GROUP_ORDER_OUTCOME", "order=0"],
            Duration::from_secs(15),
            at,
        )?;
        let mut issuers: Vec<u64> = server.text()[at_server..]
            .lines()
            .filter(|l| {
                l.contains("MP_COMMAND_RECEIVED")
                    && l.contains(" participant=1 ")
                    && l.contains(" order=0 ")
            })
            .filter_map(|l| field(l, "issuer")?.parse().ok())
            .collect();
        issuers.sort_unstable();
        let total = issuers.len();
        issuers.dedup();
        report.verdict(
            "CG-14-ONE-REQUEST-PER-UNIT",
            field(&sent, "issuers") == Some("20")
                && total == 20
                && issuers == own
                && field(&outcome, "accepted") == Some("20"),
            format!(
                "server received {total} intents from {} distinct issuers; {}",
                issuers.len(),
                outcome.trim()
            ),
        );
        thread::sleep(Duration::from_secs(2));
        report.artifact(
            "ordered_png",
            &client_a.screenshot(evidence, "05-group-1-ordered")?,
        );
        Ok(())
    })();
    launch_options_verdict(
        report,
        "CG-DEV-LAUNCH-OPTIONS",
        &[&server, &client_a, &client_b],
    );
    report.artifact("server_log", &server.log);
    report.artifact("client_a_log", &client_a.log);
    report.artifact("client_b_log", &client_b.log);
    client_a.stop();
    client_b.stop();
    server.stop();
    drop(display_a);
    result
}

// FORM-F2 group-move geometry (RTS_UNIT_FORMATIONS_AND_DISCIPLINE.md §4, §10):
// `--group-move-shape SERVER CLIENT [--evidence DIR] [--port N]`.
//
// Same dedicated 1v1 fixture as `--group-orders` (20 idle units per seat). Seat 1's client
// box-selects its 20 units with real X input and right-clicks five destinations: a Line
// move clear of the control point (a capture would end the match), V then a Column move, V then
// a Dispersed move and V then a Perimeter move (FORM-F6), V then Wedge, Echelon Left and Echelon
// Right moves (FORM-F6 part two), Shift+V back to Echelon Left and V twice to Line for a move so
// close to the western playable edge that part of the nominal line is outside it. For each move the
// client's plan and per-unit slots are checked against what the server received, projected
// and routed, and against where every unit actually stopped.
// ---------------------------------------------------------------------------------------

/// Ground probe points (world cm), all well outside the control point's 300 cm capture radius
/// at (0, 150): a Line destination east of seat 1, a Column destination west of it, a Dispersed
/// destination whose 2025 x 1350 cm block clears the control point by 500 cm and the map edge by
/// 90 cm when ordered from the Column, a Perimeter centre in the middle of a triangle of the
/// Dispersed block (about 280 cm from the nearest of its soldiers, so the right-click hits the
/// ground, not a soldier, and the 479.4 cm ring stays inside the map and 1250 cm clear of the
/// control point), a Wedge point 490 cm outside that ring (its 675 x 650 cm arrowhead stays
/// clear of the ring and 850 cm from the control point), an Echelon Left point 800 cm from the
/// Wedge (its 1061 cm diagonal stays inside the map), an Echelon Right point 740 cm off the Echelon
/// Left diagonal (its own diagonal ends 900 cm from the control point), and a Line destination
/// 100 cm inside the western playable edge (x = -2000), 630 cm from the nearest Echelon Right
/// slot. Every point is at least 490 cm from where the previous move's soldiers stand, so the
/// right-click hits the ground.
const SHAPE_LINE_POINT: (f64, f64) = (-450.0, -1000.0);
const SHAPE_COLUMN_POINT: (f64, f64) = (-1000.0, -1050.0);
const SHAPE_DISPERSED_POINT: (f64, f64) = (-750.0, -850.0);
const SHAPE_PERIMETER_POINT: (f64, f64) = (-850.0, -1360.0);
const SHAPE_WEDGE_POINT: (f64, f64) = (-1300.0, -500.0);
const SHAPE_ECHELON_LEFT_POINT: (f64, f64) = (-1300.0, -1300.0);
const SHAPE_ECHELON_RIGHT_POINT: (f64, f64) = (-800.0, -750.0);
const SHAPE_EDGE_POINT: (f64, f64) = (-1900.0, -650.0);
/// The planner's slot spacing (`politika::formation::Layout::spacingCm`).
const SHAPE_SPACING_CM: f64 = 150.0;
/// `politika::formation::Layout::dispersedSpacingCm`: beyond the 400 cm Discipline support radius.
const SHAPE_DISPERSED_SPACING_CM: f64 = 450.0;

/// Slot spacing of a shape as the planner logs it.
fn shape_spacing(shape: &str) -> f64 {
    if shape == "dispersed" {
        SHAPE_DISPERSED_SPACING_CM
    } else {
        SHAPE_SPACING_CM
    }
}

/// Radius of a single Perimeter ring of `count` slots (`PerimeterRadius`): neighbouring chords
/// of one spacing, never under 300 cm.
fn perimeter_radius(count: usize) -> f64 {
    let count = count as f64;
    (SHAPE_SPACING_CM / (2.0 * (std::f64::consts::PI / count).sin())).max(300.0)
}

fn pair(line: &str, key: &str) -> Option<(f64, f64)> {
    let (x, y) = field(line, key)?.split_once(',')?;
    Some((x.parse().ok()?, y.parse().ok()?))
}

/// `key=(x,y,z)` from an RTS_NAVIGATION_READY line, as (x, y).
fn triple_xy(line: &str, key: &str) -> Option<(f64, f64)> {
    let value = field(line, key)?
        .trim_start_matches('(')
        .trim_end_matches(')');
    let mut parts = value.split(',');
    Some((parts.next()?.parse().ok()?, parts.next()?.parse().ok()?))
}

fn min_separation(points: &[(f64, f64)]) -> f64 {
    let mut best = f64::INFINITY;
    for (index, a) in points.iter().enumerate() {
        for b in &points[index + 1..] {
            best = best.min(distance(*a, *b));
        }
    }
    best
}

struct ShapeMove<'a> {
    id: &'a str,
    shape: &'a str,
    screen: (f64, f64),
    /// Some(true): the planner must skip at least one out-of-bounds or blocked candidate;
    /// Some(false): it must skip none; None: either (the ground check decides).
    expect_rejected: Option<bool>,
}

/// Issues one right-click group move with the current selection and checks the plan, the
/// server intake/projection and the arrivals.
#[allow(clippy::too_many_lines)]
fn shape_move(
    client: &mut Game,
    server: &mut Game,
    evidence: &Path,
    report: &mut Report,
    own: &[u64],
    step: &ShapeMove,
) -> Result<(), String> {
    let id = step.id;
    let before_client = client.offset();
    let before_server = server.offset();
    client.click(step.screen.0, step.screen.1, "3")?;
    let plan = client.wait_line(
        &["RTS_GROUP_MOVE_PLAN", "order=0"],
        Duration::from_secs(5),
        before_client,
    )?;
    let batch = field(&plan, "batch").unwrap_or("0").to_owned();
    let outcome = client.wait_line(
        &["RTS_GROUP_ORDER_OUTCOME", &format!("batch={batch} ")],
        Duration::from_secs(15),
        before_client,
    )?;
    thread::sleep(Duration::from_millis(300));
    report.artifact(
        &format!("{id}_ordered_png"),
        &client.screenshot(evidence, &format!("{id}-ordered"))?,
    );
    let slots = group_move_slots(&client.text()[before_client..], &batch);
    let slot_units: Vec<u64> = slots.iter().map(|s| s.0).collect();
    let size = pair(&plan, "playable_size").unwrap_or((0.0, 0.0));
    let map_points: Vec<(f64, f64)> = slots.iter().map(|s| s.2).collect();
    let in_bounds = map_points
        .iter()
        .all(|p| p.0 >= 50.0 && p.1 >= 50.0 && p.0 <= size.0 - 50.0 && p.1 <= size.1 - 50.0);
    let placed = slots.iter().filter(|s| s.1 >= 0).count();
    let separation = min_separation(&map_points);
    let spacing = shape_spacing(step.shape);
    let rejected: usize = field(&plan, "rejected")
        .and_then(|v| v.parse().ok())
        .unwrap_or(usize::MAX);
    report.fact(&format!("{id}_plan"), plan.trim());
    report.verdict(
        &format!("FORM-04-{id}-PLAN"),
        field(&plan, "shape") == Some(step.shape)
            && field(&plan, "members") == Some("20")
            && field(&plan, "unplaced") == Some("0")
            && slot_units == own
            && placed == own.len()
            && (separation - spacing).abs() < 0.5
            && in_bounds
            && step.expect_rejected.is_none_or(|want| (rejected > 0) == want),
        format!(
            "{} slots for {} own units, {placed} placed, min separation {separation:.1} cm (want {spacing}), in bounds {in_bounds}, rejected {rejected}",
            slots.len(),
            own.len()
        ),
    );

    // Perimeter (FORM-F6): every slot on one ring around the clicked point, centred on it, with
    // neighbouring chords of one spacing (when no slot was replaced).
    if step.shape == "perimeter" {
        let anchor = pair(&plan, "anchor").unwrap_or((f64::NAN, f64::NAN));
        let radius = perimeter_radius(own.len());
        let worst = map_points
            .iter()
            .map(|p| (distance(*p, anchor) - radius).abs())
            .fold(0.0, f64::max);
        let count = map_points.len().max(1) as f64;
        let centre = (
            map_points.iter().map(|p| p.0).sum::<f64>() / count,
            map_points.iter().map(|p| p.1).sum::<f64>() / count,
        );
        report.fact(
            &format!("{id}_ring"),
            format!(
                "radius {radius:.1} cm, worst radial error {worst:.2} cm, slot centre {:.1},{:.1}, anchor {:.1},{:.1}",
                centre.0, centre.1, anchor.0, anchor.1
            ),
        );
        if rejected == 0 {
            report.verdict(
                &format!("FORM-04-{id}-SHAPE"),
                worst < 0.5 && distance(centre, anchor) < 0.5,
                format!(
                    "{} slots within {worst:.2} cm of the {radius:.1} cm ring, centre {:.1} cm from the anchor",
                    map_points.len(),
                    distance(centre, anchor)
                ),
            );
        }
    }

    // Geometry along the plan's facing: Line spans 10 slots across and 2 ranks deep; Column spans
    // 2 files across and 10 ranks deep; Dispersed spans 4.5 staggered files of 450 cm across and
    // 4 ranks deep; a Wedge of 20 spans 4.5 files across and 5 ranks of sqrt(3)/2 spacing deep;
    // an Echelon is the Line turned 45 degrees, (9 + 1) x 150 / sqrt(2) both ways (when no slot
    // was replaced).
    if let Some(facing) = pair(&plan, "facing").filter(|_| step.shape != "perimeter") {
        let right = (-facing.1, facing.0);
        let (mut across, mut along) = (Vec::new(), Vec::new());
        for p in &map_points {
            across.push(p.0 * right.0 + p.1 * right.1);
            along.push(p.0 * facing.0 + p.1 * facing.1);
        }
        let span = |v: &[f64]| {
            v.iter().copied().fold(f64::MIN, f64::max) - v.iter().copied().fold(f64::MAX, f64::min)
        };
        let (width, depth) = (span(&across), span(&along));
        report.fact(
            &format!("{id}_extent"),
            format!(
                "across {width:.1} cm, along {depth:.1} cm, facing {:.4},{:.4}",
                facing.0, facing.1
            ),
        );
        if rejected == 0 {
            let (want_width, want_depth) = match step.shape {
                "column" => (SHAPE_SPACING_CM, 9.0 * SHAPE_SPACING_CM),
                "dispersed" => (
                    4.5 * SHAPE_DISPERSED_SPACING_CM,
                    3.0 * SHAPE_DISPERSED_SPACING_CM,
                ),
                "wedge" => (
                    4.5 * SHAPE_SPACING_CM,
                    5.0 * SHAPE_SPACING_CM * 3.0_f64.sqrt() / 2.0,
                ),
                "echelon-left" | "echelon-right" => (
                    10.0 * SHAPE_SPACING_CM * std::f64::consts::FRAC_1_SQRT_2,
                    10.0 * SHAPE_SPACING_CM * std::f64::consts::FRAC_1_SQRT_2,
                ),
                _ => (9.0 * SHAPE_SPACING_CM, SHAPE_SPACING_CM),
            };
            report.verdict(
                &format!("FORM-04-{id}-SHAPE"),
                (width - want_width).abs() < 0.5 && (depth - want_depth).abs() < 0.5,
                format!(
                    "across {width:.1} (want {want_width:.1}), along {depth:.1} (want {want_depth:.1})"
                ),
            );
            // Wedge: one member alone at the point, a rank (sqrt(3)/2 spacing) ahead of the
            // rest. Echelon: the held-back flank's end slot is 1200 / sqrt(2) cm behind the
            // other flank's end slot (the right end on Echelon Right, the left end on Left).
            let mut order: Vec<usize> = (0..along.len()).collect();
            order.sort_by(|a, b| along[*b].total_cmp(&along[*a]));
            match step.shape {
                "wedge" => {
                    let lead = order
                        .get(1)
                        .map_or(0.0, |second| along[order[0]] - along[*second]);
                    let want = SHAPE_SPACING_CM * 3.0_f64.sqrt() / 2.0;
                    report.verdict(
                        &format!("FORM-F6-{id}-POINT"),
                        (lead - want).abs() < 0.5,
                        format!("the point slot leads the next by {lead:.1} cm (want {want:.1})"),
                    );
                }
                "echelon-left" | "echelon-right" => {
                    let by_across = |pick: fn(f64, f64) -> bool| {
                        (0..across.len())
                            .reduce(|best, i| {
                                if pick(across[i], across[best]) {
                                    i
                                } else {
                                    best
                                }
                            })
                            .unwrap_or(0)
                    };
                    let (right_end, left_end) = (by_across(|a, b| a > b), by_across(|a, b| a < b));
                    let (back, front) = if step.shape == "echelon-right" {
                        (right_end, left_end)
                    } else {
                        (left_end, right_end)
                    };
                    let held = along[front] - along[back];
                    let want = 1200.0 * std::f64::consts::FRAC_1_SQRT_2;
                    report.verdict(
                        &format!("FORM-F6-{id}-FLANK-HELD-BACK"),
                        (held - want).abs() < 0.5,
                        format!(
                            "the held-back end slot is {held:.1} cm behind the other end (want {want:.1})"
                        ),
                    );
                }
                _ => {}
            }
        }
    }

    group_move_routes(
        client,
        server,
        evidence,
        report,
        own,
        id,
        &slots,
        &outcome,
        spacing,
        before_server,
    )
}

/// One client-planned group-move slot: unit, slot index (-1 when unplaced), map point and world
/// point, from `RTS_GROUP_MOVE_SLOT`.
type GroupSlot = (u64, i64, (f64, f64), (f64, f64));

/// The client's slots of one group-move batch, sorted by unit.
fn group_move_slots(text: &str, batch: &str) -> Vec<GroupSlot> {
    let mut slots: Vec<GroupSlot> = text
        .lines()
        .filter(|l| l.contains("RTS_GROUP_MOVE_SLOT") && field(l, "batch") == Some(batch))
        .filter_map(|l| {
            Some((
                field(l, "unit")?.parse().ok()?,
                field(l, "slot")?.parse().ok()?,
                (number(l, "x")?, number(l, "y")?),
                (number(l, "world_x")?, number(l, "world_y")?),
            ))
        })
        .collect();
    slots.sort_by_key(|s| s.0);
    slots
}

/// Server intake, projection and arrival of one client-planned group move (FORM-05).
#[allow(clippy::too_many_arguments, clippy::too_many_lines)]
fn group_move_routes(
    client: &mut Game,
    server: &mut Game,
    evidence: &Path,
    report: &mut Report,
    own: &[u64],
    id: &str,
    slots: &[GroupSlot],
    outcome: &str,
    spacing: f64,
    before_server: usize,
) -> Result<(), String> {
    // The server received one ordinary Move per unit at exactly the client's slot, projected
    // it within its own 150 cm threshold and routed it; all were admitted on their own.
    let server_text = server.text()[before_server..].to_owned();
    let mut received: Vec<u64> = server_text
        .lines()
        .filter(|l| {
            l.contains("MP_COMMAND_RECEIVED")
                && l.contains(" participant=1 ")
                && l.contains(" order=0 ")
        })
        .filter_map(|l| field(l, "issuer")?.parse().ok())
        .collect();
    received.sort_unstable();
    let ready: Vec<UnitPoints> = server_text
        .lines()
        .filter(|l| l.contains("RTS_NAVIGATION_READY"))
        .filter_map(|l| {
            Some((
                field(l, "unit")?.parse().ok()?,
                triple_xy(l, "requested")?,
                triple_xy(l, "resolved")?,
            ))
        })
        .filter(|r| own.contains(&r.0))
        .collect();
    let matched = slots
        .iter()
        .filter(|s| {
            ready
                .iter()
                .any(|r| r.0 == s.0 && distance(r.1, s.3) < 1.0 && distance(r.1, r.2) <= 150.0)
        })
        .count();
    let resolved: Vec<(f64, f64)> = ready.iter().map(|r| r.2).collect();
    let resolved_separation = min_separation(&resolved);
    report.verdict(
        &format!("FORM-05-{id}-SERVER-ROUTES"),
        received == own
            && matched == own.len()
            && ready.len() == own.len()
            && field(&outcome, "accepted") == Some("20")
            && field(&outcome, "rejected") == Some("0"),
        format!(
            "{} intents received, {matched}/{} slots routed at the sent point, resolved min separation {resolved_separation:.1} cm; {}",
            received.len(),
            own.len(),
            outcome.trim()
        ),
    );

    // Every unit arrives at its own slot: no navigation failure, and the final positions
    // keep the spacing instead of stacking on one point.
    let deadline = Instant::now() + Duration::from_secs(40);
    let arrived = |text: &str| {
        own.iter()
            .filter(|unit| {
                text.lines().any(|l| {
                    l.contains("RTS_GAMEPLAY_EVENT kind=OrderChanged")
                        && field(l, "entity").and_then(|v| v.parse::<u64>().ok()) == Some(**unit)
                        && field(l, "previous") == Some("1")
                        && field(l, "reason") == Some("5")
                })
            })
            .count()
    };
    while Instant::now() < deadline && arrived(&server.text()[before_server..]) < own.len() {
        thread::sleep(Duration::from_millis(500));
    }
    thread::sleep(Duration::from_millis(1500));
    let server_text = server.text()[before_server..].to_owned();
    let failed = server_text
        .lines()
        .filter(|l| {
            (l.contains("RTS_NAVIGATION_FAILED")
                && field(l, "unit")
                    .and_then(|v| v.parse::<u64>().ok())
                    .is_some_and(|u| own.contains(&u)))
                || (l.contains("kind=OrderChanged")
                    && field(l, "reason") == Some("7")
                    && field(l, "entity")
                        .and_then(|v| v.parse::<u64>().ok())
                        .is_some_and(|u| own.contains(&u)))
        })
        .count();
    let finals: Vec<(f64, f64)> = own
        .iter()
        .filter_map(|unit| server_unit(&server_text, *unit).map(|s| s.0))
        .collect();
    let final_separation = min_separation(&finals);
    let at_slot = slots
        .iter()
        .filter(|s| {
            ready.iter().any(|r| {
                r.0 == s.0
                    && server_unit(&server_text, s.0).is_some_and(|f| distance(f.0, r.2) <= 30.0)
            })
        })
        .count();
    let count = arrived(&server_text);
    report.artifact(
        &format!("{id}_arrived_png"),
        &client.screenshot(evidence, &format!("{id}-arrived"))?,
    );
    report.verdict(
        &format!("FORM-{id}-ARRIVED-SPACED"),
        count == own.len()
            && failed == 0
            && finals.len() == own.len()
            && at_slot == own.len()
            && final_separation >= spacing - 20.0,
        format!(
            "{count}/{} arrived, {failed} navigation failures, {at_slot} stopped within 30 cm of their resolved slot, final min separation {final_separation:.1} cm (want at least {})",
            own.len(),
            spacing - 20.0
        ),
    );
    Ok(())
}

#[allow(clippy::too_many_lines)]
fn group_move_shape_session(options: &DedicatedOptions, report: &mut Report) -> Result<(), String> {
    let client_options = &options.client;
    let evidence = &client_options.evidence;
    let port = client_options.port;
    let mut args = server_args(port, "1v1");
    args.push(format!("-PolitikaMPUnitScale={GROUP_UNITS_PER_SEAT}"));
    args.push("-PolitikaMPUnitScaleHold".into());
    let mut server = Game::launch_server(&options.server, "server", evidence, &args)?;
    report.fact("server_command", server.command.join(" "));
    server.wait(&["MP_DEDICATED_SERVER_READY"], Duration::from_secs(180))?;
    let awaiting = server.wait(&["MP_MATCH_AWAITING_PARTICIPANTS"], Duration::from_secs(60))?;
    let generation = field(&awaiting, "generation").unwrap_or("0").to_owned();
    let fixture = server.wait(&["MP_UNIT_SCALE_FIXTURE"], Duration::from_secs(5))?;

    let display_a = Display::start(evidence, "client-a")?;
    let mut args_a = client_args(client_options, true, false, false);
    let probes = [
        SHAPE_LINE_POINT,
        SHAPE_COLUMN_POINT,
        SHAPE_DISPERSED_POINT,
        SHAPE_PERIMETER_POINT,
        SHAPE_WEDGE_POINT,
        SHAPE_ECHELON_LEFT_POINT,
        SHAPE_ECHELON_RIGHT_POINT,
        SHAPE_EDGE_POINT,
    ];
    args_a.push(format!(
        "-PolitikaMPGroundProbe={}",
        probes
            .iter()
            .map(|(x, y)| format!("{x}:{y}"))
            .collect::<Vec<_>>()
            .join("/")
    ));
    let mut client_a = Game::launch(
        &client_options.binary,
        "client-a",
        evidence,
        &args_a,
        Some(&display_a.name()),
    )?;
    report.fact("client_a_command", client_a.command.join(" "));
    server.wait(
        &["MP_PARTICIPANT_BOUND participant=1", "remote=1"],
        Duration::from_secs(90),
    )?;
    let mut client_b = Game::launch(
        &client_options.binary,
        "client-b",
        evidence,
        &client_args(client_options, false, false, false),
        None,
    )?;
    report.fact("client_b_command", client_b.command.join(" "));
    server.wait(
        &["MP_PARTICIPANT_BOUND participant=2", "remote=1"],
        Duration::from_secs(90),
    )?;
    server.wait(
        &["MP_MATCH_ACTIVE", &format!("generation={generation}")],
        Duration::from_secs(30),
    )?;
    client_a.wait(
        &[
            "MP_CLIENT_MATCH_OBSERVED",
            "map=map.test.multiplayer_skirmish",
        ],
        Duration::from_secs(60),
    )?;
    client_a.attach_window()?;
    thread::sleep(Duration::from_secs(6));

    let result = (|| -> Result<(), String> {
        let owners = entity_owners(&server.text());
        let own: Vec<u64> = owners
            .iter()
            .filter(|(_, o)| *o == 1)
            .map(|(e, _)| *e)
            .collect();
        report.verdict(
            "FORM-FIXTURE-20-PER-SEAT",
            own.len() == GROUP_UNITS_PER_SEAT && field(&fixture, "per_seat") == Some("20"),
            format!("participant 1 units={}; {}", own.len(), fixture.trim()),
        );
        report.artifact(
            "client_a_start_png",
            &client_a.screenshot(evidence, "01-start")?,
        );

        let before_box = client_a.offset();
        client_a.drag_hold(
            (24.0, 24.0),
            (f64::from(WIDTH) - 24.0, f64::from(HEIGHT) - 24.0),
        )?;
        client_a.release()?;
        let boxed = client_a.wait_line(
            &["RTS_SELECTION_SET", "source=box", "op=replace"],
            Duration::from_secs(5),
            before_box,
        )?;
        report.verdict(
            "FORM-01-BOX-SELECTS-OWN-UNITS",
            selection_ids(&boxed) == own,
            boxed.trim().to_owned(),
        );

        // 1. Line (the default), clear of the control point so no capture ends the match.
        let line = wait_screen(&mut client_a, |t| probe_screen(t, 0), "line probe")?;
        shape_move(
            &mut client_a,
            &mut server,
            evidence,
            report,
            &own,
            &ShapeMove {
                id: "LINE",
                shape: "line",
                screen: line,
                expect_rejected: Some(false),
            },
        )?;

        // V cycles Line, Column, Dispersed, Perimeter, Wedge, Echelon Left, Echelon Right, Line;
        // Shift+V steps back. Each press is local: the client sends nothing and the server
        // receives nothing; the HUD names the new shape.
        let press_shape_key = |client: &mut Game,
                               server: &Game,
                               report: &mut Report,
                               verdict: &str,
                               want: &str,
                               backward: bool|
         -> Result<(), String> {
            let before_v = client.offset();
            let before_v_server = server.offset();
            // Shift is held across the V press, as a player holds it: a one-shot `shift+v` can
            // release Shift inside the same frame as V, and the game reads Shift as up.
            if backward {
                client.modified_key("shift", "v")?;
            } else {
                client.key("v")?;
            }
            let shape =
                client.wait_line(&["RTS_GROUP_MOVE_SHAPE"], Duration::from_secs(5), before_v)?;
            thread::sleep(Duration::from_millis(500));
            let sent = client.text()[before_v..]
                .lines()
                .filter(|l| l.contains("RTS_COMMAND_INTENT_SENT"))
                .count();
            let received = server.text()[before_v_server..]
                .lines()
                .filter(|l| l.contains("MP_COMMAND_RECEIVED") && l.contains(" participant=1 "))
                .count();
            let direction = if backward { "1" } else { "0" };
            report.verdict(
                verdict,
                field(&shape, "shape") == Some(want)
                    && field(&shape, "backward") == Some(direction)
                    && sent == 0
                    && received == 0,
                format!(
                    "{}; intents sent {sent}, server received {received}",
                    shape.trim()
                ),
            );
            let name = if backward {
                format!("hud-{want}-selected-back")
            } else {
                format!("hud-{want}-selected")
            };
            report.artifact(
                &format!("client_a_{}_png", name.replace('-', "_")),
                &client.screenshot(evidence, &name)?,
            );
            Ok(())
        };
        let press_v =
            |client: &mut Game, server: &Game, report: &mut Report, verdict: &str, want: &str| {
                press_shape_key(client, server, report, verdict, want, false)
            };

        // 2. V: Column.
        press_v(
            &mut client_a,
            &server,
            report,
            "FORM-02-SHAPE-KEY-IS-LOCAL",
            "column",
        )?;
        let column = wait_screen(&mut client_a, |t| probe_screen(t, 1), "column probe")?;
        shape_move(
            &mut client_a,
            &mut server,
            evidence,
            report,
            &own,
            &ShapeMove {
                id: "COLUMN",
                shape: "column",
                screen: column,
                expect_rejected: Some(false),
            },
        )?;

        // 3. V: Dispersed (FORM-F6), staggered ranks 450 cm apart.
        press_v(
            &mut client_a,
            &server,
            report,
            "FORM-F6-SHAPE-KEY-DISPERSED",
            "dispersed",
        )?;
        let dispersed = wait_screen(&mut client_a, |t| probe_screen(t, 2), "dispersed probe")?;
        shape_move(
            &mut client_a,
            &mut server,
            evidence,
            report,
            &own,
            &ShapeMove {
                id: "DISPERSED",
                shape: "dispersed",
                screen: dispersed,
                expect_rejected: None,
            },
        )?;

        // 4. V: Perimeter (FORM-F6), one ring of twenty around the clicked point.
        press_v(
            &mut client_a,
            &server,
            report,
            "FORM-F6-SHAPE-KEY-PERIMETER",
            "perimeter",
        )?;
        let perimeter = wait_screen(&mut client_a, |t| probe_screen(t, 3), "perimeter probe")?;
        shape_move(
            &mut client_a,
            &mut server,
            evidence,
            report,
            &own,
            &ShapeMove {
                id: "PERIMETER",
                shape: "perimeter",
                screen: perimeter,
                expect_rejected: None,
            },
        )?;

        // 5-7. V: Wedge, Echelon Left and Echelon Right (FORM-F6 part two), each one spacing
        // between neighbours.
        let turned = [
            ("WEDGE", "wedge", "FORM-F6-SHAPE-KEY-WEDGE", 4),
            (
                "ECHELON-LEFT",
                "echelon-left",
                "FORM-F6-SHAPE-KEY-ECHELON-LEFT",
                5,
            ),
            (
                "ECHELON-RIGHT",
                "echelon-right",
                "FORM-F6-SHAPE-KEY-ECHELON-RIGHT",
                6,
            ),
        ];
        for (id, shape, verdict, probe) in turned {
            press_v(&mut client_a, &server, report, verdict, shape)?;
            let point = wait_screen(
                &mut client_a,
                |t| probe_screen(t, probe),
                &format!("{shape} probe"),
            )?;
            shape_move(
                &mut client_a,
                &mut server,
                evidence,
                report,
                &own,
                &ShapeMove {
                    id,
                    shape,
                    screen: point,
                    expect_rejected: Some(false),
                },
            )?;
        }

        // 8. Shift+V steps back to Echelon Left; V twice returns to Line through Echelon Right.
        press_shape_key(
            &mut client_a,
            &server,
            report,
            "FORM-F6-SHIFT-V-STEPS-BACK",
            "echelon-left",
            true,
        )?;
        press_v(
            &mut client_a,
            &server,
            report,
            "FORM-F6-SHAPE-KEY-FORWARD-AGAIN",
            "echelon-right",
        )?;
        press_v(
            &mut client_a,
            &server,
            report,
            "FORM-SHAPE-CYCLES-BACK-TO-LINE",
            "line",
        )?;
        // 9. Line next to the western edge; out-of-bounds slots are replaced.
        let edge = wait_screen(&mut client_a, |t| probe_screen(t, 7), "edge probe")?;
        shape_move(
            &mut client_a,
            &mut server,
            evidence,
            report,
            &own,
            &ShapeMove {
                id: "EDGE",
                shape: "line",
                screen: edge,
                expect_rejected: Some(true),
            },
        )?;
        // The fixture's match stayed Active throughout: no objective capture or result.
        let ended = server.text().lines().any(|l| {
            l.contains("MATCH_LIFECYCLE phase=5")
                || l.contains("RTS_OBJECTIVE_STATE") && l.contains("complete=1")
        });
        report.verdict(
            "FORM-MATCH-STAYS-ACTIVE",
            !ended,
            format!("terminal or captured during the session: {ended}"),
        );
        Ok(())
    })();
    launch_options_verdict(
        report,
        "FORM-SHAPE-DEV-LAUNCH-OPTIONS",
        &[&server, &client_a, &client_b],
    );
    report.artifact("server_log", &server.log);
    report.artifact("client_a_log", &client_a.log);
    report.artifact("client_b_log", &client_b.log);
    client_a.stop();
    client_b.stop();
    server.stop();
    drop(display_a);
    result
}

// ---------------------------------------------------------------------------------------
// FORM-FD right-drag facing and frontage (RTS_FORMATION_FACING_DRAG_ACCEPTANCE.md):
// `--pointer-stall SERVER CLIENT [--evidence DIR] [--port N]`.
//
// Same dedicated 1v1 fixture as `--group-move-facing`, with seat 1's client capped at two frames a
// second, so real X input routinely falls inside one frame. Box selects that span frames and box
// selects whose press, drag and release all fall inside one frame must select the same 20 units
// between the true corners; a right-click followed by a pointer move inside one frame must order at
// the clicked point; and a right-drag inside one frame must commit as a drag between its true ends.
const STALL_FPS: u32 = 2;
/// Longer than a capped frame, so the next gesture starts on a settled cursor.
const STALL_SETTLE: Duration = Duration::from_millis(1200);
/// Pixels of slack on an event position against where the X pointer was sent.
const STALL_SLACK_PX: f64 = 3.0;

fn near(a: Option<(f64, f64)>, b: (f64, f64)) -> bool {
    a.is_some_and(|a| (a.0 - b.0).abs() <= STALL_SLACK_PX && (a.1 - b.1).abs() <= STALL_SLACK_PX)
}

impl Game {
    /// Press, move and release with nothing between them, all inside one capped frame.
    fn fast_stroke(&self, button: &str, from: (f64, f64), to: (f64, f64)) -> Result<(), String> {
        self.pointer_to(from.0, from.1)?;
        thread::sleep(STALL_SETTLE);
        self.xdo_here(&["mousedown", button])?;
        self.pointer_to(to.0, to.1)?;
        self.xdo_here(&["mouseup", button])
    }
}

/// X and Y from `xdotool getmouselocation --shell`.
fn xdo_pointer(text: &str) -> Option<(f64, f64)> {
    let value = |key: &str| {
        text.lines()
            .find_map(|l| l.strip_prefix(key)?.parse::<f64>().ok())
    };
    Some((value("X=")?, value("Y=")?))
}

fn stall_box_verdict(
    client: &mut Game,
    report: &mut Report,
    id: &str,
    own: &[u64],
    before: usize,
    corners: ((f64, f64), (f64, f64)),
) -> Result<(), String> {
    let boxed = client.wait_line(
        &["RTS_SELECTION_SET", "op=replace"],
        Duration::from_secs(8),
        before,
    )?;
    report.verdict(
        id,
        boxed.contains("source=box") && selection_ids(&boxed) == own,
        boxed.trim().to_owned(),
    );
    let text = client.text()[before..].to_owned();
    let stroke = text
        .lines()
        .find(|l| l.contains("RTS_SELECTION_STROKE"))
        .unwrap_or("")
        .to_owned();
    report.verdict(
        &format!("{id}-CORNERS"),
        near(pair(&stroke, "press"), corners.0) && near(pair(&stroke, "end"), corners.1),
        stroke.trim().to_owned(),
    );
    Ok(())
}

fn pointer_stall_session(options: &DedicatedOptions, report: &mut Report) -> Result<(), String> {
    let client_options = &options.client;
    let evidence = &client_options.evidence;
    let port = client_options.port;
    let mut args = server_args(port, "1v1");
    args.push(format!("-PolitikaMPUnitScale={GROUP_UNITS_PER_SEAT}"));
    args.push("-PolitikaMPUnitScaleHold".into());
    let mut server = Game::launch_server(&options.server, "server", evidence, &args)?;
    report.fact("server_command", server.command.join(" "));
    server.wait(&["MP_DEDICATED_SERVER_READY"], Duration::from_secs(180))?;
    let awaiting = server.wait(&["MP_MATCH_AWAITING_PARTICIPANTS"], Duration::from_secs(60))?;
    let generation = field(&awaiting, "generation").unwrap_or("0").to_owned();

    let display_a = Display::start(evidence, "client-a")?;
    let mut args_a = client_args(client_options, true, false, false);
    args_a.push(format!(
        "-PolitikaMPGroundProbe={}",
        FACING_PROBES
            .iter()
            .map(|(x, y)| format!("{x}:{y}"))
            .collect::<Vec<_>>()
            .join("/")
    ));
    args_a.push(format!("-ExecCmds=t.MaxFPS {STALL_FPS}"));
    let mut client_a = Game::launch(
        &client_options.binary,
        "client-a",
        evidence,
        &args_a,
        Some(&display_a.name()),
    )?;
    report.fact("client_a_command", client_a.command.join(" "));
    server.wait(
        &["MP_PARTICIPANT_BOUND participant=1", "remote=1"],
        Duration::from_secs(90),
    )?;
    let mut client_b = Game::launch(
        &client_options.binary,
        "client-b",
        evidence,
        &client_args(client_options, false, false, false),
        None,
    )?;
    server.wait(
        &["MP_PARTICIPANT_BOUND participant=2", "remote=1"],
        Duration::from_secs(90),
    )?;
    server.wait(
        &["MP_MATCH_ACTIVE", &format!("generation={generation}")],
        Duration::from_secs(30),
    )?;
    client_a.wait(
        &[
            "MP_CLIENT_MATCH_OBSERVED",
            "map=map.test.multiplayer_skirmish",
        ],
        Duration::from_secs(90),
    )?;
    client_a.attach_window()?;
    thread::sleep(Duration::from_secs(6));

    let result = (|| -> Result<(), String> {
        let frames = client_a
            .text()
            .lines()
            .rev()
            .find(|l| l.contains("MP_CLIENT_FRAME_WINDOW"))
            .unwrap_or("")
            .to_owned();
        report.verdict(
            "PS-CLIENT-CAPPED",
            number(&frames, "mean_ms").is_some_and(|ms| ms >= 300.0),
            frames.trim().to_owned(),
        );
        let owners = entity_owners(&server.text());
        let own: Vec<u64> = owners
            .iter()
            .filter(|(_, o)| *o == 1)
            .map(|(e, _)| *e)
            .collect();
        let start = (24.0, 24.0);
        let end = (f64::from(WIDTH) - 24.0, f64::from(HEIGHT) - 24.0);
        for run in 1..=5 {
            let before = client_a.offset();
            client_a.drag_hold(start, end)?;
            client_a.release()?;
            let id = format!("PS-BOX-SPANNING-{run}");
            stall_box_verdict(&mut client_a, report, &id, &own, before, (start, end))?;
            thread::sleep(STALL_SETTLE);
        }
        for run in 1..=5 {
            let before = client_a.offset();
            client_a.fast_stroke("1", start, end)?;
            let id = format!("PS-BOX-ONE-FRAME-{run}");
            stall_box_verdict(&mut client_a, report, &id, &own, before, (start, end))?;
            thread::sleep(STALL_SETTLE);
        }

        // The cursor stays where the button came up instead of returning to the press point.
        let before = client_a.offset();
        client_a.drag_hold(start, end)?;
        thread::sleep(STALL_SETTLE);
        client_a.release()?;
        client_a.wait_line(
            &["RTS_SELECTION_SET", "source=box", "op=replace"],
            Duration::from_secs(8),
            before,
        )?;
        thread::sleep(STALL_SETTLE);
        let display = client_a.display.as_deref().ok_or("no display")?;
        let location = xdo(display, &["getmouselocation", "--shell"])?;
        report.verdict(
            "PS-CURSOR-STAYS-AT-RELEASE",
            near(xdo_pointer(&location), end),
            location.split_whitespace().collect::<Vec<_>>().join(" "),
        );

        // A right-click, then the pointer moves away inside the same frame.
        let click = wait_screen(&mut client_a, |t| probe_screen(t, 0), "click probe")?;
        let away = wait_screen(&mut client_a, |t| probe_screen(t, 2), "away probe")?;
        client_a.pointer_to(click.0, click.1)?;
        thread::sleep(STALL_SETTLE);
        let before = client_a.offset();
        client_a.xdo_here(&["mousedown", "3"])?;
        client_a.xdo_here(&["mouseup", "3"])?;
        client_a.pointer_to(away.0, away.1)?;
        let commit = client_a.wait_line(
            &["RTS_GROUP_MOVE_DRAG", "state=commit"],
            Duration::from_secs(8),
            before,
        )?;
        report.verdict(
            "PS-ORDER-CLICK-AT-PRESS",
            field(&commit, "reason") == Some("click") && near(pair(&commit, "screen_start"), click),
            commit.trim().to_owned(),
        );
        thread::sleep(STALL_SETTLE);

        // A right-drag whose press, movement and release all fall inside one frame.
        let from = wait_screen(&mut client_a, |t| probe_screen(t, 1), "drag start probe")?;
        let to = wait_screen(&mut client_a, |t| probe_screen(t, 2), "drag end probe")?;
        let before = client_a.offset();
        client_a.fast_stroke("3", from, to)?;
        let commit = client_a.wait_line(
            &["RTS_GROUP_MOVE_DRAG", "state=commit"],
            Duration::from_secs(8),
            before,
        )?;
        report.verdict(
            "PS-ORDER-DRAG-ONE-FRAME",
            field(&commit, "reason") == Some("drag")
                && near(pair(&commit, "screen_start"), from)
                && near(pair(&commit, "screen_end"), to),
            commit.trim().to_owned(),
        );
        Ok(())
    })();
    launch_options_verdict(
        report,
        "PS-DEV-LAUNCH-OPTIONS",
        &[&server, &client_a, &client_b],
    );
    report.artifact("server_log", &server.log);
    report.artifact("client_a_log", &client_a.log);
    report.artifact("client_b_log", &client_b.log);
    client_a.stop();
    client_b.stop();
    server.stop();
    drop(display_a);
    result
}

// `--group-move-facing SERVER CLIENT [--evidence DIR] [--port N]`.
//
// Same dedicated 1v1 fixture as `--group-move-shape` (20 idle units per seat). Seat 1's client
// box-selects its 20 units with real X input, then: a plain right-click (travel facing, default
// ten-file Line, nothing drag-specific sent); a right-drag across the screen from left to right
// (a five-file Line facing away from the camera, centred on the drag midpoint); a held drag
// cancelled by Escape and another cancelled by a left click (nothing sent, no menu); Shift+RMB
// (no drag: a plain move on press); one selected unit (ordered on press, the release adds
// nothing); dragged Column (four files), Perimeter (facing only) and Echelon Left (five files
// before the turn); and a whole formation formed with T and dragged from right to left, so the
// client sends a protocol-9 facing and frontage and the server plans, stores and routes it. Every
// held drag must show its preview while nothing is sent; every committed plan is checked against
// the drag endpoints the client logged (anchor within 0.5 cm, facing within 0.001) and against
// the server's intake, routes and arrivals. `normalized.txt` records each move's geometry and
// outcomes for `--compare-normalized` across independent runs.
// ---------------------------------------------------------------------------------------

/// Ground probe points (world cm). The camera looks along +Y with screen-right at world -X, so a
/// drag from a larger to a smaller X runs left to right on screen. Each press point is at least
/// 200 cm from where the previous move leaves the soldiers, every block stays inside the map and
/// at least 550 cm from the control point at (0, 150) (checked with the portable planner before
/// the run).
const FACING_PROBES: [(f64, f64); 17] = [
    (-450.0, -1000.0),  // 0 plain click
    (-1150.0, -1000.0), // 1 Line drag start
    (-1750.0, -1000.0), // 2 Line drag end: 600 cm, five files
    (-500.0, -700.0),   // 3 cancelled drag start
    (-1000.0, -700.0),  // 4 cancelled drag end
    (-300.0, -600.0),   // 5 empty ground for the cancelling left click
    (-600.0, -1000.0),  // 6 Shift+RMB press
    (-900.0, -1000.0),  // 7 Shift+RMB drag end
    (-1450.0, -1300.0), // 8 single-unit press
    (-1225.0, -700.0),  // 9 Column drag start
    (-1675.0, -700.0),  // 10 Column drag end: 450 cm, four files
    (-350.0, -1100.0),  // 11 Perimeter drag start
    (-750.0, -1100.0),  // 12 Perimeter drag end
    (-1150.0, -800.0),  // 13 Echelon Left drag start
    (-1750.0, -800.0),  // 14 Echelon Left drag end: 600 cm, five files
    (-850.0, -1050.0),  // 15 formation drag start
    (-400.0, -1050.0),  // 16 formation drag end: right to left, 450 cm, four files
];
/// Planner limits that the harness recomputes independently.
const FACING_MIN_DRAG_CM: f64 = 1.0;
const FACING_ANCHOR_TOLERANCE_CM: f64 = 0.5;
const FACING_TOLERANCE: f64 = 0.001;
/// Pixel quantization of the real pointer moves the ground hits off the nominal probe points.
const FACING_NOMINAL_TOLERANCE: f64 = 0.02;

impl Game {
    /// Presses the right button at `from` and moves to `to` in held steps that the game samples
    /// over several frames; returns with the button still held.
    fn right_drag_hold(&self, from: (f64, f64), to: (f64, f64)) -> Result<(), String> {
        self.pointer_to(from.0, from.1)?;
        thread::sleep(Duration::from_millis(150));
        self.xdo_here(&["mousedown", "3"])?;
        thread::sleep(Duration::from_millis(200));
        for step in 1..=10 {
            let t = f64::from(step) / 10.0;
            self.pointer_to(from.0 + (to.0 - from.0) * t, from.1 + (to.1 - from.1) * t)?;
            thread::sleep(Duration::from_millis(70));
        }
        thread::sleep(Duration::from_millis(350));
        Ok(())
    }

    fn right_release(&self) -> Result<(), String> {
        self.xdo_here(&["mouseup", "3"])?;
        thread::sleep(Duration::from_millis(300));
        Ok(())
    }

    /// After a failure: lets go of every button and modifier the facing harness may hold.
    fn release_inputs(&self) {
        let _ = self.xdo_here(&["mouseup", "3"]);
        let _ = self.xdo_here(&["keyup", "shift"]);
    }
}

/// What a right-drag between two logged ground points must produce, recomputed here from the
/// formation contract: anchor = midpoint (map cm), facing = (dy, -dx) / length, frontage =
/// clamp(round(length / spacing) + 1, 1, members) from one spacing on (halves up), else 0.
#[derive(Clone, Copy, Debug, PartialEq)]
struct DragExpect {
    anchor: (f64, f64),
    facing: (f64, f64),
    length: f64,
    frontage: usize,
}

fn drag_expect(
    start: (f64, f64),
    end: (f64, f64),
    playable_min: (f64, f64),
    spacing: f64,
    members: usize,
    has_frontage: bool,
) -> Option<DragExpect> {
    let (dx, dy) = (end.0 - start.0, end.1 - start.1);
    let length = dx.hypot(dy);
    if !length.is_finite() || length < FACING_MIN_DRAG_CM || spacing <= 0.0 {
        return None;
    }
    let frontage = if !has_frontage || members == 0 || length < spacing {
        0
    } else {
        let ratio = (length / spacing).min(members as f64);
        ((ratio + 0.5).floor() as usize + 1).clamp(1, members)
    };
    Some(DragExpect {
        anchor: (
            (start.0 + end.0) * 0.5 - playable_min.0,
            (start.1 + end.1) * 0.5 - playable_min.1,
        ),
        facing: (dy / length, -dx / length),
        length,
        frontage,
    })
}

fn shape_has_frontage(shape: &str) -> bool {
    shape != "wedge" && shape != "perimeter"
}

/// Orders and formation requests seat 1 sent (client) or the server received from it.
fn sent_orders(client: &str) -> usize {
    count_lines(client, &["RTS_COMMAND_INTENT_SENT"]) + count_lines(client, &["RTS_FORMATION_SENT"])
}

fn received_orders(server: &str) -> usize {
    count_lines(server, &["MP_COMMAND_RECEIVED", " participant=1 "])
        + count_lines(server, &["RTS_FORMATION_INTENT", "participant=1 "])
}

/// Extent across `RightOf(facing)` and along `facing` of map points.
fn frame_extent(points: &[(f64, f64)], facing: (f64, f64)) -> (f64, f64) {
    let right = (-facing.1, facing.0);
    let span = |v: Vec<f64>| {
        v.iter().copied().fold(f64::MIN, f64::max) - v.iter().copied().fold(f64::MAX, f64::min)
    };
    (
        span(
            points
                .iter()
                .map(|p| p.0 * right.0 + p.1 * right.1)
                .collect(),
        ),
        span(
            points
                .iter()
                .map(|p| p.0 * facing.0 + p.1 * facing.1)
                .collect(),
        ),
    )
}

struct FacingDrag<'a> {
    id: &'a str,
    shape: &'a str,
    from: usize,
    to: usize,
    /// Files the drag must request (0: the shape's default or none).
    frontage: usize,
    /// Nominal facing from the probe points.
    facing: (f64, f64),
    /// Planned (across, along) extent, when it is fixed by the files.
    extent: Option<(f64, f64)>,
}

/// One held right-drag and its release; returns the commit and plan lines.
fn held_drag(
    client: &mut Game,
    server: &Game,
    evidence: &Path,
    report: &mut Report,
    step: &FacingDrag,
    planned: usize,
) -> Result<(), String> {
    let id = step.id;
    let before_client = client.offset();
    let before_server = server.offset();
    let from = wait_screen(
        client,
        |t| probe_screen(t, step.from),
        &format!("{id} start"),
    )?;
    let to = wait_screen(client, |t| probe_screen(t, step.to), &format!("{id} end"))?;
    client.right_drag_hold(from, to)?;
    let update = client.wait_line(
        &["RTS_GROUP_MOVE_DRAG", "state=update"],
        Duration::from_secs(5),
        before_client,
    );
    thread::sleep(Duration::from_millis(250));
    report.artifact(
        &format!("{id}_held_png"),
        &client.screenshot(evidence, &format!("{id}-held"))?,
    );
    let held_client = client.text()[before_client..].to_owned();
    let held_server = server.text()[before_server..].to_owned();
    let last_update = held_client
        .lines()
        .rev()
        .find(|l| l.contains("RTS_GROUP_MOVE_DRAG") && l.contains("state=update"))
        .unwrap_or("")
        .to_owned();
    let checked: usize = field(&last_update, "checked")
        .and_then(|v| v.parse().ok())
        .unwrap_or(usize::MAX);
    report.verdict(
        &format!("FD-{id}-PREVIEW-HELD"),
        update.is_ok()
            && field(&last_update, "planned_frontage") == Some(planned.to_string().as_str())
            && field(&last_update, "shape") == Some(step.shape)
            && checked <= 40
            && sent_orders(&held_client) == 0
            && received_orders(&held_server) == 0,
        format!(
            "{} preview replans while held, planned frontage {:?} (want {planned}), {checked} slot checks (at most 40), intents sent {}, server received {}",
            count_lines(&held_client, &["RTS_GROUP_MOVE_DRAG", "state=update"]),
            field(&last_update, "planned_frontage"),
            sent_orders(&held_client),
            received_orders(&held_server)
        ),
    );
    client.right_release()
}

/// Checks a committed ordinary drag against its logged endpoints, then the server's routes.
#[allow(clippy::too_many_lines)]
fn facing_move(
    client: &mut Game,
    server: &mut Game,
    evidence: &Path,
    report: &mut Report,
    own: &[u64],
    step: &FacingDrag,
    normalized: &mut Vec<String>,
) -> Result<(), String> {
    let id = step.id;
    let before_client = client.offset();
    let before_server = server.offset();
    let spacing = shape_spacing(step.shape);
    let planned = if step.frontage != 0 {
        step.frontage
    } else if shape_has_frontage(step.shape) {
        10
    } else {
        0
    };
    held_drag(client, server, evidence, report, step, planned)?;
    let commit = client.wait_line(
        &["RTS_GROUP_MOVE_DRAG", "state=commit"],
        Duration::from_secs(5),
        before_client,
    )?;
    let plan = client.wait_line(
        &["RTS_GROUP_MOVE_PLAN", "order=0", "drag=1"],
        Duration::from_secs(5),
        before_client,
    )?;
    let batch = field(&plan, "batch").unwrap_or("0").to_owned();
    let outcome = client.wait_line(
        &["RTS_GROUP_ORDER_OUTCOME", &format!("batch={batch} ")],
        Duration::from_secs(15),
        before_client,
    )?;
    thread::sleep(Duration::from_millis(300));
    report.artifact(
        &format!("{id}_ordered_png"),
        &client.screenshot(evidence, &format!("{id}-ordered"))?,
    );
    let client_text = client.text()[before_client..].to_owned();
    report.fact(&format!("{id}_commit"), commit.trim());
    report.fact(&format!("{id}_plan"), plan.trim());

    // The plan is exactly the drag: midpoint anchor, right-hand perpendicular facing, files from
    // the length; recomputed here from the client's own logged endpoints.
    let start = pair(&commit, "world_start").unwrap_or((f64::NAN, f64::NAN));
    let end = pair(&commit, "world_end").unwrap_or((f64::NAN, f64::NAN));
    let min = pair(&plan, "playable_min").unwrap_or((f64::NAN, f64::NAN));
    let expect = drag_expect(
        start,
        end,
        min,
        spacing,
        own.len(),
        shape_has_frontage(step.shape),
    );
    let anchor = pair(&plan, "anchor").unwrap_or((f64::NAN, f64::NAN));
    let facing = pair(&plan, "facing").unwrap_or((f64::NAN, f64::NAN));
    let requested: usize = field(&plan, "requested_frontage")
        .and_then(|v| v.parse().ok())
        .unwrap_or(usize::MAX);
    let effective: usize = field(&plan, "frontage")
        .and_then(|v| v.parse().ok())
        .unwrap_or(usize::MAX);
    let (anchor_error, facing_error) = expect.map_or((f64::INFINITY, f64::INFINITY), |e| {
        (distance(anchor, e.anchor), distance(facing, e.facing))
    });
    let nominal_error = distance(facing, step.facing);
    report.verdict(
        &format!("FD-{id}-DRAG-GEOMETRY"),
        field(&commit, "reason") == Some("drag")
            && count_lines(&client_text, &["RTS_GROUP_MOVE_DRAG", "state=commit"]) == 1
            && count_lines(&client_text, &["RTS_GROUP_ORDER_SENT"]) == 1
            && count_lines(&client_text, &["RTS_FORMATION_SENT"]) == 0
            && field(&plan, "explicit_facing") == Some("1")
            && anchor_error <= FACING_ANCHOR_TOLERANCE_CM
            && facing_error <= FACING_TOLERANCE
            && nominal_error <= FACING_NOMINAL_TOLERANCE
            && expect.is_some_and(|e| e.frontage == step.frontage && requested == e.frontage)
            && effective == planned,
        format!(
            "drag {start:?} -> {end:?} ({:.1} cm): anchor error {anchor_error:.3} cm, facing {facing:?} error {facing_error:.6} (nominal {:?}, {nominal_error:.4}), requested frontage {requested} (want {}), effective {effective} (want {planned})",
            expect.map_or(f64::NAN, |e| e.length),
            step.facing,
            step.frontage
        ),
    );

    let slots = group_move_slots(&client_text, &batch);
    let slot_units: Vec<u64> = slots.iter().map(|s| s.0).collect();
    let size = pair(&plan, "playable_size").unwrap_or((0.0, 0.0));
    let map_points: Vec<(f64, f64)> = slots.iter().map(|s| s.2).collect();
    let in_bounds = map_points
        .iter()
        .all(|p| p.0 >= 50.0 && p.1 >= 50.0 && p.0 <= size.0 - 50.0 && p.1 <= size.1 - 50.0);
    let placed = slots.iter().filter(|s| s.1 >= 0).count();
    let separation = min_separation(&map_points);
    let rejected = field(&plan, "rejected").unwrap_or("?").to_owned();
    report.verdict(
        &format!("FD-{id}-PLAN"),
        field(&plan, "shape") == Some(step.shape)
            && field(&plan, "members") == Some("20")
            && field(&plan, "unplaced") == Some("0")
            && slot_units == own
            && placed == own.len()
            && (separation - spacing).abs() < 0.5
            && in_bounds
            && rejected == "0",
        format!(
            "{} slots for {} own units, {placed} placed, min separation {separation:.1} cm (want {spacing}), in bounds {in_bounds}, rejected {rejected}",
            slots.len(),
            own.len()
        ),
    );
    let (across, along) = frame_extent(&map_points, facing);
    report.fact(
        &format!("{id}_extent"),
        format!("across {across:.1} cm, along {along:.1} cm"),
    );
    if let Some((want_across, want_along)) = step.extent {
        report.verdict(
            &format!("FD-{id}-SHAPE"),
            (across - want_across).abs() < 0.5 && (along - want_along).abs() < 0.5,
            format!(
                "across {across:.1} (want {want_across:.1}), along {along:.1} (want {want_along:.1}) of the dragged facing"
            ),
        );
    }
    if step.shape == "perimeter" {
        // Facing only: the same 479.4 cm ring, its first slot straight ahead of the anchor.
        let radius = perimeter_radius(own.len());
        let worst = map_points
            .iter()
            .map(|p| (distance(*p, anchor) - radius).abs())
            .fold(0.0, f64::max);
        let front = (anchor.0 + radius * facing.0, anchor.1 + radius * facing.1);
        let ahead = map_points.iter().any(|p| distance(*p, front) < 0.5);
        report.verdict(
            &format!("FD-{id}-SHAPE"),
            worst < 0.5 && ahead && requested == 0 && effective == 0,
            format!(
                "{} slots within {worst:.2} cm of the {radius:.1} cm ring, a slot ahead along the dragged facing {ahead}, frontage requested {requested} effective {effective}",
                map_points.len()
            ),
        );
    }
    if step.shape.starts_with("echelon") {
        // The Line block of `files` is turned 45 degrees: its front-rank end on the held-back
        // side stands (files - ranks) x 150 / sqrt(2) behind the rear-rank end on the other side.
        let right = (-facing.1, facing.0);
        let across_of = |p: &(f64, f64)| p.0 * right.0 + p.1 * right.1;
        let along_of = |p: &(f64, f64)| p.0 * facing.0 + p.1 * facing.1;
        let by = |pick: fn(f64, f64) -> bool| {
            map_points
                .iter()
                .copied()
                .reduce(|best, p| {
                    if pick(across_of(&p), across_of(&best)) {
                        p
                    } else {
                        best
                    }
                })
                .unwrap_or((f64::NAN, f64::NAN))
        };
        let (right_end, left_end) = (by(|a, b| a > b), by(|a, b| a < b));
        let (back, front) = if step.shape == "echelon-right" {
            (right_end, left_end)
        } else {
            (left_end, right_end)
        };
        let files = step.frontage.max(1);
        let ranks = own.len().div_ceil(files);
        let want =
            (files as f64 - ranks as f64) * SHAPE_SPACING_CM * std::f64::consts::FRAC_1_SQRT_2;
        let held = along_of(&front) - along_of(&back);
        report.verdict(
            &format!("FD-{id}-FLANK-HELD-BACK"),
            (held - want).abs() < 0.5,
            format!("held-back end {held:.1} cm behind the other end (want {want:.1} for {files} files in {ranks} ranks)"),
        );
    }
    let mut assignment: Vec<String> = slots
        .iter()
        .map(|s| format!("{}:{}:{:.3}:{:.3}", s.0, s.1, s.2.0, s.2.1))
        .collect();
    assignment.sort();
    group_move_routes(
        client,
        server,
        evidence,
        report,
        own,
        id,
        &slots,
        &outcome,
        spacing,
        before_server,
    )?;
    normalized.push(format!(
        "{id} shape={} anchor={:.3},{:.3} facing={:.6},{:.6} frontage={requested} effective={effective} accepted={} rejected={} slots={}",
        step.shape,
        anchor.0,
        anchor.1,
        facing.0,
        facing.1,
        field(&outcome, "accepted").unwrap_or("?"),
        field(&outcome, "rejected").unwrap_or("?"),
        assignment.join(",")
    ));
    Ok(())
}

/// A held drag that is cancelled while the button is down: nothing may be sent or received.
#[allow(clippy::too_many_arguments)]
fn cancelled_drag(
    client: &mut Game,
    server: &Game,
    evidence: &Path,
    report: &mut Report,
    id: &str,
    reason: &str,
    cancel: &dyn Fn(&mut Game) -> Result<(), String>,
    normalized: &mut Vec<String>,
) -> Result<(), String> {
    let before_client = client.offset();
    let before_server = server.offset();
    let from = wait_screen(client, |t| probe_screen(t, 3), "cancel start")?;
    let to = wait_screen(client, |t| probe_screen(t, 4), "cancel end")?;
    client.right_drag_hold(from, to)?;
    let update = client.wait_line(
        &["RTS_GROUP_MOVE_DRAG", "state=update"],
        Duration::from_secs(5),
        before_client,
    );
    thread::sleep(Duration::from_millis(250));
    report.artifact(
        &format!("{id}_held_png"),
        &client.screenshot(evidence, &format!("{id}-held"))?,
    );
    cancel(client)?;
    let cancelled = client.wait_line(
        &[
            "RTS_GROUP_MOVE_DRAG",
            "state=cancel",
            &format!("reason={reason}"),
        ],
        Duration::from_secs(5),
        before_client,
    );
    thread::sleep(Duration::from_millis(400));
    report.artifact(
        &format!("{id}_cancelled_png"),
        &client.screenshot(evidence, &format!("{id}-cancelled"))?,
    );
    client.right_release()?;
    thread::sleep(Duration::from_millis(1500));
    let client_text = client.text()[before_client..].to_owned();
    let server_text = server.text()[before_server..].to_owned();
    let routed = count_lines(&server_text, &["RTS_NAVIGATION_READY"]);
    let menu = client_text.contains("MainMenu") || client_text.contains("main_menu");
    report.verdict(
        &format!("FD-{id}"),
        update.is_ok()
            && cancelled.is_ok()
            && count_lines(&client_text, &["RTS_GROUP_MOVE_DRAG", "state=commit"]) == 0
            && sent_orders(&client_text) == 0
            && count_lines(&client_text, &["RTS_GROUP_ORDER_SENT"]) == 0
            && received_orders(&server_text) == 0
            && routed == 0
            && !menu,
        format!(
            "held preview {}, cancelled {}, commits {}, intents sent {}, server received {}, routes installed {routed}, menu travel {menu}",
            update.is_ok(),
            cancelled.as_deref().map_or("no", str::trim),
            count_lines(&client_text, &["RTS_GROUP_MOVE_DRAG", "state=commit"]),
            sent_orders(&client_text),
            received_orders(&server_text)
        ),
    );
    normalized.push(format!(
        "{id} cancel={reason} sent={} received={} routed={routed}",
        sent_orders(&client_text),
        received_orders(&server_text)
    ));
    Ok(())
}

/// Box-selects every own unit on screen with real input and checks the selection.
fn box_select_all(
    client: &mut Game,
    report: &mut Report,
    own: &[u64],
    verdict: &str,
) -> Result<(), String> {
    let before_box = client.offset();
    client.drag_hold(
        (24.0, 24.0),
        (f64::from(WIDTH) - 24.0, f64::from(HEIGHT) - 24.0),
    )?;
    client.release()?;
    let boxed = client.wait_line(
        &["RTS_SELECTION_SET", "source=box", "op=replace"],
        Duration::from_secs(5),
        before_box,
    )?;
    report.verdict(
        verdict,
        selection_ids(&boxed) == own,
        boxed.trim().to_owned(),
    );
    Ok(())
}

/// V presses until the HUD's group-move shape is `want`.
fn cycle_shape_to(
    client: &mut Game,
    report: &mut Report,
    want: &str,
    presses: usize,
) -> Result<(), String> {
    let mut last = String::new();
    for _ in 0..presses {
        let before = client.offset();
        client.key("v")?;
        last = client.wait_line(&["RTS_GROUP_MOVE_SHAPE"], Duration::from_secs(5), before)?;
        thread::sleep(Duration::from_millis(300));
    }
    report.verdict(
        &format!("FD-SHAPE-KEY-{}", want.to_uppercase()),
        field(&last, "shape") == Some(want),
        last.trim().to_owned(),
    );
    Ok(())
}

#[allow(clippy::too_many_lines)]
fn group_move_facing_session(
    options: &DedicatedOptions,
    report: &mut Report,
) -> Result<(), String> {
    let client_options = &options.client;
    let evidence = &client_options.evidence;
    let port = client_options.port;
    let mut args = server_args(port, "1v1");
    args.push(format!("-PolitikaMPUnitScale={GROUP_UNITS_PER_SEAT}"));
    args.push("-PolitikaMPUnitScaleHold".into());
    let mut server = Game::launch_server(&options.server, "server", evidence, &args)?;
    report.fact("server_command", server.command.join(" "));
    let ready = server.wait(&["MP_DEDICATED_SERVER_READY"], Duration::from_secs(180))?;
    report.fact("server_ready", ready.trim());
    let awaiting = server.wait(&["MP_MATCH_AWAITING_PARTICIPANTS"], Duration::from_secs(60))?;
    let generation = field(&awaiting, "generation").unwrap_or("0").to_owned();
    let fixture = server.wait(&["MP_UNIT_SCALE_FIXTURE"], Duration::from_secs(5))?;

    let display_a = Display::start(evidence, "client-a")?;
    let mut args_a = client_args(client_options, true, false, false);
    args_a.push(format!(
        "-PolitikaMPGroundProbe={}",
        FACING_PROBES
            .iter()
            .map(|(x, y)| format!("{x}:{y}"))
            .collect::<Vec<_>>()
            .join("/")
    ));
    let mut client_a = Game::launch(
        &client_options.binary,
        "client-a",
        evidence,
        &args_a,
        Some(&display_a.name()),
    )?;
    report.fact("client_a_command", client_a.command.join(" "));
    server.wait(
        &["MP_PARTICIPANT_BOUND participant=1", "remote=1"],
        Duration::from_secs(90),
    )?;
    let mut client_b = Game::launch(
        &client_options.binary,
        "client-b",
        evidence,
        &client_args(client_options, false, false, false),
        None,
    )?;
    report.fact("client_b_command", client_b.command.join(" "));
    server.wait(
        &["MP_PARTICIPANT_BOUND participant=2", "remote=1"],
        Duration::from_secs(90),
    )?;
    server.wait(
        &["MP_MATCH_ACTIVE", &format!("generation={generation}")],
        Duration::from_secs(30),
    )?;
    client_a.wait(
        &[
            "MP_CLIENT_MATCH_OBSERVED",
            "map=map.test.multiplayer_skirmish",
        ],
        Duration::from_secs(60),
    )?;
    client_a.attach_window()?;
    thread::sleep(Duration::from_secs(6));

    let mut normalized: Vec<String> = Vec::new();
    let result = (|| -> Result<(), String> {
        let owners = entity_owners(&server.text());
        let own: Vec<u64> = owners
            .iter()
            .filter(|(_, o)| *o == 1)
            .map(|(e, _)| *e)
            .collect();
        report.verdict(
            "FORM-FIXTURE-20-PER-SEAT",
            own.len() == GROUP_UNITS_PER_SEAT && field(&fixture, "per_seat") == Some("20"),
            format!("participant 1 units={}; {}", own.len(), fixture.trim()),
        );
        report.verdict(
            "FD-PROTOCOL-CURRENT",
            field(&ready, "protocol") == Some(network_protocol().as_str()),
            ready.trim().to_owned(),
        );
        report.artifact(
            "client_a_start_png",
            &client_a.screenshot(evidence, "01-start")?,
        );
        box_select_all(&mut client_a, report, &own, "FORM-01-BOX-SELECTS-OWN-UNITS")?;

        // FD-P02: a plain right-click is the FORM-F2 click it always was.
        let before_click = client_a.offset();
        let before_click_server = server.offset();
        let click = wait_screen(&mut client_a, |t| probe_screen(t, 0), "click probe")?;
        shape_move(
            &mut client_a,
            &mut server,
            evidence,
            report,
            &own,
            &ShapeMove {
                id: "CLICK",
                shape: "line",
                screen: click,
                expect_rejected: Some(false),
            },
        )?;
        let click_text = client_a.text()[before_click..].to_owned();
        let click_plan = click_text
            .lines()
            .find(|l| l.contains("RTS_GROUP_MOVE_PLAN"))
            .unwrap_or("")
            .to_owned();
        report.verdict(
            "FD-CLICK-COMPATIBLE",
            field(&click_plan, "drag") == Some("0")
                && field(&click_plan, "explicit_facing") == Some("0")
                && field(&click_plan, "requested_frontage") == Some("0")
                && field(&click_plan, "frontage") == Some("10")
                && count_lines(&click_text, &["RTS_GROUP_MOVE_DRAG", "state=commit", "reason=click"]) == 1
                && count_lines(&click_text, &["RTS_GROUP_MOVE_DRAG", "state=update"]) == 0
                && count_lines(&click_text, &["RTS_GROUP_ORDER_SENT"]) == 1
                && count_lines(&click_text, &["RTS_COMMAND_INTENT_SENT"]) == own.len()
                && count_lines(&click_text, &["RTS_FORMATION_SENT"]) == 0
                && count_lines(&server.text()[before_click_server..], &["RTS_FORMATION_INTENT"]) == 0,
            format!(
                "travel facing, default frontage, one batch of {} ordinary intents and no formation request: {}",
                count_lines(&click_text, &["RTS_COMMAND_INTENT_SENT"]),
                click_plan.trim()
            ),
        );
        normalized.push(format!(
            "CLICK anchor={} facing={} frontage={}",
            field(&click_plan, "anchor").unwrap_or("?"),
            field(&click_plan, "facing").unwrap_or("?"),
            field(&click_plan, "frontage").unwrap_or("?")
        ));

        // FD-P01: a left-to-right drag faces away from the camera (+Y) in five files.
        facing_move(
            &mut client_a,
            &mut server,
            evidence,
            report,
            &own,
            &FacingDrag {
                id: "LINE",
                shape: "line",
                from: 1,
                to: 2,
                frontage: 5,
                facing: (0.0, 1.0),
                extent: Some((4.0 * SHAPE_SPACING_CM, 3.0 * SHAPE_SPACING_CM)),
            },
            &mut normalized,
        )?;

        // FD-P03: Escape while the button is held cancels and stays in the match.
        cancelled_drag(
            &mut client_a,
            &server,
            evidence,
            report,
            "ESCAPE-CANCEL",
            "Escape",
            &|client: &mut Game| client.key("Escape"),
            &mut normalized,
        )?;
        // FD-P07: a left click during the drag (a selection gesture) cancels it.
        let empty = wait_screen(&mut client_a, |t| probe_screen(t, 5), "empty ground")?;
        cancelled_drag(
            &mut client_a,
            &server,
            evidence,
            report,
            "SELECT-CANCEL",
            "SelectPress",
            &|client: &mut Game| client.click(empty.0, empty.1, "1"),
            &mut normalized,
        )?;
        box_select_all(&mut client_a, report, &own, "FD-RESELECT-AFTER-CANCEL")?;

        // FD-A28: Shift+RMB never starts a drag; it is the plain move, sent on press.
        let before_shift = client_a.offset();
        let before_shift_server = server.offset();
        let shift = wait_screen(&mut client_a, |t| probe_screen(t, 6), "shift probe")?;
        let shift_end = wait_screen(&mut client_a, |t| probe_screen(t, 7), "shift end")?;
        client_a.pointer_to(shift.0, shift.1)?;
        thread::sleep(Duration::from_millis(150));
        client_a.xdo_here(&["keydown", "shift"])?;
        thread::sleep(Duration::from_millis(150));
        client_a.xdo_here(&["mousedown", "3"])?;
        let on_press = client_a.wait_line(
            &["RTS_GROUP_MOVE_PLAN", "order=0"],
            Duration::from_secs(3),
            before_shift,
        );
        for step in 1..=6 {
            let t = f64::from(step) / 6.0;
            client_a.pointer_to(
                shift.0 + (shift_end.0 - shift.0) * t,
                shift.1 + (shift_end.1 - shift.1) * t,
            )?;
            thread::sleep(Duration::from_millis(70));
        }
        client_a.xdo_here(&["mouseup", "3"])?;
        thread::sleep(Duration::from_millis(150));
        client_a.xdo_here(&["keyup", "shift"])?;
        let shift_plan = on_press?;
        let batch = field(&shift_plan, "batch").unwrap_or("0").to_owned();
        let shift_outcome = client_a.wait_line(
            &["RTS_GROUP_ORDER_OUTCOME", &format!("batch={batch} ")],
            Duration::from_secs(15),
            before_shift,
        )?;
        let shift_text = client_a.text()[before_shift..].to_owned();
        report.verdict(
            "FD-SHIFT-RMB-RESERVED",
            field(&shift_plan, "drag") == Some("0")
                && field(&shift_plan, "explicit_facing") == Some("0")
                && count_lines(&shift_text, &["RTS_GROUP_MOVE_DRAG"]) == 0
                && count_lines(&shift_text, &["RTS_GROUP_ORDER_SENT"]) == 1
                && field(&shift_outcome, "accepted") == Some("20"),
            format!(
                "planned on press, before the release; drag lines {}; {}",
                count_lines(&shift_text, &["RTS_GROUP_MOVE_DRAG"]),
                shift_plan.trim()
            ),
        );
        let shift_slots = group_move_slots(&shift_text, &batch);
        group_move_routes(
            &mut client_a,
            &mut server,
            evidence,
            report,
            &own,
            "SHIFT",
            &shift_slots,
            &shift_outcome,
            SHAPE_SPACING_CM,
            before_shift_server,
        )?;

        // FD-P04: one selected unit is ordered on press; its release adds nothing.
        let single = own[0];
        select_entity(&mut client_a, "MP_CLIENT_SAMPLE", single)?;
        let before_single = client_a.offset();
        let before_single_server = server.offset();
        let point = wait_screen(&mut client_a, |t| probe_screen(t, 8), "single probe")?;
        client_a.pointer_to(point.0, point.1)?;
        thread::sleep(Duration::from_millis(150));
        client_a.xdo_here(&["mousedown", "3"])?;
        let pressed = client_a.wait_line(
            &["RTS_COMMAND_INTENT_SENT", &format!("issuer={single} ")],
            Duration::from_secs(3),
            before_single,
        );
        thread::sleep(Duration::from_millis(400));
        client_a.xdo_here(&["mouseup", "3"])?;
        thread::sleep(Duration::from_millis(1500));
        let single_text = client_a.text()[before_single..].to_owned();
        let single_server = server.text()[before_single_server..].to_owned();
        report.verdict(
            "FD-SINGLE-UNIT-ON-PRESS",
            pressed.is_ok()
                && count_lines(&single_text, &["RTS_COMMAND_INTENT_SENT"]) == 1
                && count_lines(&single_server, &["MP_COMMAND_RECEIVED", " participant=1 "]) == 1
                && count_lines(&single_text, &["RTS_GROUP_MOVE_DRAG"]) == 0,
            format!(
                "intent sent while the button was held {}, intents in total {}, server received {}, drag lines {}",
                pressed.is_ok(),
                count_lines(&single_text, &["RTS_COMMAND_INTENT_SENT"]),
                count_lines(&single_server, &["MP_COMMAND_RECEIVED", " participant=1 "]),
                count_lines(&single_text, &["RTS_GROUP_MOVE_DRAG"])
            ),
        );
        normalized.push(format!(
            "SINGLE intents={} received={}",
            count_lines(&single_text, &["RTS_COMMAND_INTENT_SENT"]),
            count_lines(&single_server, &["MP_COMMAND_RECEIVED", " participant=1 "])
        ));
        thread::sleep(Duration::from_secs(4));
        box_select_all(&mut client_a, report, &own, "FD-RESELECT-AFTER-SINGLE")?;

        // FD-P06: Column (four files), Perimeter (facing only) and Echelon Left (five files
        // before the 45 degree turn).
        cycle_shape_to(&mut client_a, report, "column", 1)?;
        facing_move(
            &mut client_a,
            &mut server,
            evidence,
            report,
            &own,
            &FacingDrag {
                id: "COLUMN",
                shape: "column",
                from: 9,
                to: 10,
                frontage: 4,
                facing: (0.0, 1.0),
                extent: Some((3.0 * SHAPE_SPACING_CM, 4.0 * SHAPE_SPACING_CM)),
            },
            &mut normalized,
        )?;
        cycle_shape_to(&mut client_a, report, "perimeter", 2)?;
        facing_move(
            &mut client_a,
            &mut server,
            evidence,
            report,
            &own,
            &FacingDrag {
                id: "PERIMETER",
                shape: "perimeter",
                from: 11,
                to: 12,
                frontage: 0,
                facing: (0.0, 1.0),
                extent: None,
            },
            &mut normalized,
        )?;
        cycle_shape_to(&mut client_a, report, "echelon-left", 2)?;
        facing_move(
            &mut client_a,
            &mut server,
            evidence,
            report,
            &own,
            &FacingDrag {
                id: "ECHELON-LEFT",
                shape: "echelon-left",
                from: 13,
                to: 14,
                frontage: 5,
                facing: (0.0, 1.0),
                extent: Some((
                    7.0 * SHAPE_SPACING_CM * std::f64::consts::FRAC_1_SQRT_2,
                    7.0 * SHAPE_SPACING_CM * std::f64::consts::FRAC_1_SQRT_2,
                )),
            },
            &mut normalized,
        )?;
        cycle_shape_to(&mut client_a, report, "line", 2)?;

        // FD-P05: T forms the twenty; a right-to-left drag sends the formation's protocol-9
        // facing and frontage, and the server plans, stores and routes the move.
        formation_facing_move(
            &mut client_a,
            &mut server,
            evidence,
            report,
            &own,
            &mut normalized,
        )?;

        let ended = server.text().lines().any(|l| {
            l.contains("MATCH_LIFECYCLE phase=5")
                || l.contains("RTS_OBJECTIVE_STATE") && l.contains("complete=1")
        });
        report.verdict(
            "FORM-MATCH-STAYS-ACTIVE",
            !ended,
            format!("terminal or captured during the session: {ended}"),
        );
        Ok(())
    })();
    if result.is_err() {
        client_a.release_inputs();
    }
    let path = evidence.join("normalized.txt");
    let _ = fs::write(&path, normalized.join("\n") + "\n");
    report.artifact("normalized_txt", &path);
    launch_options_verdict(
        report,
        "FORM-FD-DEV-LAUNCH-OPTIONS",
        &[&server, &client_a, &client_b],
    );
    report.artifact("server_log", &server.log);
    report.artifact("client_a_log", &client_a.log);
    report.artifact("client_b_log", &client_b.log);
    client_a.stop();
    client_b.stop();
    server.stop();
    drop(display_a);
    result
}

/// A unit with two map or world points (a slot and its world point, or a requested and a
/// resolved navigation point).
type UnitPoints = (u64, (f64, f64), (f64, f64));

/// FD-P05: forms the selection with T and drags the whole formation from right to left.
#[allow(clippy::too_many_lines)]
fn formation_facing_move(
    client: &mut Game,
    server: &mut Game,
    evidence: &Path,
    report: &mut Report,
    own: &[u64],
    normalized: &mut Vec<String>,
) -> Result<(), String> {
    let before_form = client.offset();
    client.key("t")?;
    let formed = client.wait_line(
        &["RTS_FORMATION_OUTCOME", "op=0 ", "accepted=1"],
        Duration::from_secs(5),
        before_form,
    )?;
    let formation = field(&formed, "formation").unwrap_or("0").to_owned();
    report.verdict(
        "FD-FORMATION-FORMED",
        formation != "0" && field(&formed, "members") == Some("20"),
        formed.trim().to_owned(),
    );
    thread::sleep(Duration::from_millis(800));
    let step = FacingDrag {
        id: "FORMATION",
        shape: "line",
        from: 15,
        to: 16,
        frontage: 4,
        facing: (0.0, -1.0),
        extent: Some((3.0 * SHAPE_SPACING_CM, 4.0 * SHAPE_SPACING_CM)),
    };
    let before_client = client.offset();
    let before_server = server.offset();
    held_drag(client, server, evidence, report, &step, 4)?;
    let commit = client.wait_line(
        &["RTS_GROUP_MOVE_DRAG", "state=commit"],
        Duration::from_secs(5),
        before_client,
    )?;
    let sent = client.wait_line(
        &["RTS_FORMATION_SENT", "op=move"],
        Duration::from_secs(5),
        before_client,
    )?;
    let outcome = client.wait_line(
        &["RTS_FORMATION_OUTCOME", "op=2 "],
        Duration::from_secs(10),
        before_client,
    )?;
    let intake = server.wait_line(
        &["RTS_FORMATION_INTENT", "participant=1 ", "op=2 "],
        Duration::from_secs(10),
        before_server,
    )?;
    let moved = server.wait_line(
        &["RTS_FORMATION_MOVE", "op=move"],
        Duration::from_secs(10),
        before_server,
    )?;
    thread::sleep(Duration::from_millis(500));
    report.artifact(
        "FORMATION_ordered_png",
        &client.screenshot(evidence, "FORMATION-ordered")?,
    );
    report.fact("FORMATION_sent", sent.trim());
    report.fact("FORMATION_intake", intake.trim());
    report.fact("FORMATION_move", moved.trim());

    // The client sends the drag (never slots); the server validates it and plans on its own.
    let start = pair(&commit, "world_start").unwrap_or((f64::NAN, f64::NAN));
    let end = pair(&commit, "world_end").unwrap_or((f64::NAN, f64::NAN));
    let client_text = client.text()[before_client..].to_owned();
    let min = client_text
        .lines()
        .chain(client.text().lines())
        .find(|l| l.contains("RTS_GROUP_MOVE_PLAN"))
        .and_then(|l| pair(l, "playable_min"))
        .unwrap_or((-2000.0, -2000.0));
    let expect = drag_expect(start, end, min, SHAPE_SPACING_CM, own.len(), true);
    let sent_anchor = (
        number(&sent, "x").unwrap_or(f64::NAN),
        number(&sent, "y").unwrap_or(f64::NAN),
    );
    let sent_facing = pair(&sent, "facing").unwrap_or((f64::NAN, f64::NAN));
    let (anchor_error, facing_error) = expect.map_or((f64::INFINITY, f64::INFINITY), |e| {
        (
            distance(sent_anchor, e.anchor),
            distance(sent_facing, e.facing),
        )
    });
    report.verdict(
        "FD-FORMATION-SENT-PROTOCOL-CURRENT",
        field(&sent, "protocol") == Some(network_protocol().as_str())
            && field(&sent, "formation") == Some(formation.as_str())
            && field(&sent, "frontage") == Some("4")
            && anchor_error <= FACING_ANCHOR_TOLERANCE_CM
            && facing_error <= FACING_TOLERANCE
            && distance(sent_facing, step.facing) <= FACING_NOMINAL_TOLERANCE
            && expect.is_some_and(|e| e.frontage == 4)
            && count_lines(&client_text, &["RTS_FORMATION_SENT"]) == 1
            && count_lines(&client_text, &["RTS_COMMAND_INTENT_SENT"]) == 0
            && count_lines(&client_text, &["RTS_GROUP_MOVE_SLOT"]) == 0,
        format!(
            "anchor error {anchor_error:.3} cm, facing {sent_facing:?} error {facing_error:.6}, no client slots or unit intents: {}",
            sent.trim()
        ),
    );
    let move_anchor = pair(&moved, "anchor").unwrap_or((f64::NAN, f64::NAN));
    let move_facing = pair(&moved, "facing").unwrap_or((f64::NAN, f64::NAN));
    let stored_facing = pair(&moved, "stored_facing").unwrap_or((f64::NAN, f64::NAN));
    report.verdict(
        "FD-FORMATION-SERVER-PLAN",
        field(&intake, "accepted") == Some("1")
            && field(&intake, "frontage") == Some("4")
            && pair(&intake, "facing").is_some_and(|f| distance(f, sent_facing) < 1e-5)
            && field(&moved, "formation") == Some(formation.as_str())
            && field(&moved, "members") == Some("20")
            && field(&moved, "unplaced") == Some("0")
            && field(&moved, "rejected") == Some("0")
            && field(&moved, "orders") == Some("20")
            && field(&moved, "explicit_facing") == Some("1")
            && field(&moved, "requested_frontage") == Some("4")
            && field(&moved, "frontage") == Some("4")
            && number(&moved, "checked").is_some_and(|c| c <= 40.0)
            && distance(move_anchor, sent_anchor) < 0.01
            && distance(move_facing, sent_facing) < 1e-5
            && field(&moved, "stored_explicit") == Some("1")
            && field(&moved, "stored_frontage") == Some("4")
            && pair(&moved, "stored_anchor").is_some_and(|a| distance(a, sent_anchor) < 0.01)
            && distance(stored_facing, sent_facing) < 1e-5
            && field(&outcome, "accepted") == Some("1")
            && field(&outcome, "orders") == Some("20"),
        format!("{}; {}", intake.trim(), moved.trim()),
    );

    // Twenty independently admitted member Moves at the server's slots, routed and arrived.
    let server_text = server.text()[before_server..].to_owned();
    let mut slots: Vec<UnitPoints> = server_text
        .lines()
        .filter(|l| {
            l.contains("RTS_FORMATION_SLOT")
                && field(l, "formation") == Some(formation.as_str())
                && field(l, "cause") == Some("move")
        })
        .filter_map(|l| {
            Some((
                field(l, "unit")?.parse().ok()?,
                (number(l, "x")?, number(l, "y")?),
                (number(l, "world_x")?, number(l, "world_y")?),
            ))
        })
        .collect();
    slots.sort_by_key(|s| s.0);
    let slot_units: Vec<u64> = slots.iter().map(|s| s.0).collect();
    let map_points: Vec<(f64, f64)> = slots.iter().map(|s| s.1).collect();
    let (across, along) = frame_extent(&map_points, move_facing);
    let separation = min_separation(&map_points);
    let centre = (
        map_points.iter().map(|p| p.0).sum::<f64>() / map_points.len().max(1) as f64,
        map_points.iter().map(|p| p.1).sum::<f64>() / map_points.len().max(1) as f64,
    );
    report.verdict(
        "FD-FORMATION-SHAPE",
        slot_units == own
            && (separation - SHAPE_SPACING_CM).abs() < 0.5
            && (across - 3.0 * SHAPE_SPACING_CM).abs() < 0.5
            && (along - 4.0 * SHAPE_SPACING_CM).abs() < 0.5
            && distance(centre, move_anchor) < 0.5,
        format!(
            "{} server slots, across {across:.1} (want 450), along {along:.1} (want 600), min separation {separation:.1}, centre {:.2} cm from the anchor",
            slots.len(),
            distance(centre, move_anchor)
        ),
    );
    let received = count_lines(&server_text, &["MP_COMMAND_RECEIVED", " participant=1 "]);
    let ready: Vec<UnitPoints> = server_text
        .lines()
        .filter(|l| l.contains("RTS_NAVIGATION_READY"))
        .filter_map(|l| {
            Some((
                field(l, "unit")?.parse().ok()?,
                triple_xy(l, "requested")?,
                triple_xy(l, "resolved")?,
            ))
        })
        .filter(|r| own.contains(&r.0))
        .collect();
    let routed = slots
        .iter()
        .filter(|s| {
            ready
                .iter()
                .any(|r| r.0 == s.0 && distance(r.1, s.2) < 1.0 && distance(r.1, r.2) <= 150.0)
        })
        .count();
    let tally = client.wait_line(
        &["RTS_GROUP_ORDER_OUTCOME", "expected=20"],
        Duration::from_secs(15),
        before_client,
    )?;
    report.verdict(
        "FD-FORMATION-SERVER-ROUTES",
        received == 0 && routed == own.len() && ready.len() == own.len()
            && field(&tally, "accepted") == Some("20") && field(&tally, "rejected") == Some("0"),
        format!(
            "server-submitted member orders (no client unit intents received: {received}), {routed}/{} routed at the server's slots; {}",
            own.len(),
            tally.trim()
        ),
    );
    let deadline = Instant::now() + Duration::from_secs(40);
    let arrived = |text: &str| {
        own.iter()
            .filter(|unit| {
                text.lines().any(|l| {
                    l.contains("RTS_GAMEPLAY_EVENT kind=OrderChanged")
                        && field(l, "entity").and_then(|v| v.parse::<u64>().ok()) == Some(**unit)
                        && field(l, "previous") == Some("1")
                        && field(l, "reason") == Some("5")
                })
            })
            .count()
    };
    while Instant::now() < deadline && arrived(&server.text()[before_server..]) < own.len() {
        thread::sleep(Duration::from_millis(500));
    }
    thread::sleep(Duration::from_millis(1500));
    let server_text = server.text()[before_server..].to_owned();
    let count = arrived(&server_text);
    let finals: Vec<(f64, f64)> = own
        .iter()
        .filter_map(|unit| server_unit(&server_text, *unit).map(|s| s.0))
        .collect();
    let failed = count_lines(&server_text, &["RTS_NAVIGATION_FAILED"]);
    report.artifact(
        "FORMATION_arrived_png",
        &client.screenshot(evidence, "FORMATION-arrived")?,
    );
    report.verdict(
        "FD-FORMATION-ARRIVED-SPACED",
        count == own.len() && failed == 0 && min_separation(&finals) >= SHAPE_SPACING_CM - 20.0,
        format!(
            "{count}/{} arrived, {failed} navigation failures, final min separation {:.1} cm",
            own.len(),
            min_separation(&finals)
        ),
    );
    let mut assignment: Vec<String> = slots
        .iter()
        .map(|s| format!("{}:{:.3}:{:.3}", s.0, s.1.0, s.1.1))
        .collect();
    assignment.sort();
    normalized.push(format!(
        "FORMATION anchor={:.3},{:.3} facing={:.6},{:.6} frontage={} stored_frontage={} orders={} accepted={} arrived={count} slots={}",
        move_anchor.0,
        move_anchor.1,
        move_facing.0,
        move_facing.1,
        field(&moved, "frontage").unwrap_or("?"),
        field(&moved, "stored_frontage").unwrap_or("?"),
        field(&moved, "orders").unwrap_or("?"),
        field(&tally, "accepted").unwrap_or("?"),
        assignment.join(",")
    ));
    Ok(())
}

/// `--compare-normalized A B`: two runs' `normalized.txt` must agree line by line; numbers within
/// 0.001 for facings and 0.5 cm for every other value, everything else exactly.
fn compare_normalized(a: &str, b: &str) -> Vec<String> {
    let (left, right): (Vec<&str>, Vec<&str>) = (a.lines().collect(), b.lines().collect());
    let mut problems = Vec::new();
    if left.len() != right.len() {
        problems.push(format!("{} lines vs {}", left.len(), right.len()));
    }
    for (l, r) in left.iter().zip(&right) {
        let (lt, rt): (Vec<&str>, Vec<&str>) = (
            l.split_whitespace().collect(),
            r.split_whitespace().collect(),
        );
        if lt.len() != rt.len() {
            problems.push(format!("{l} | {r}"));
            continue;
        }
        for (x, y) in lt.iter().zip(&rt) {
            let key = x.split('=').next().unwrap_or("");
            let tolerance = if key == "facing" {
                FACING_TOLERANCE
            } else {
                FACING_ANCHOR_TOLERANCE_CM
            };
            let split =
                |s: &str| -> Vec<String> { s.split([',', ':', '=']).map(str::to_owned).collect() };
            let (xs, ys) = (split(x), split(y));
            let same = xs.len() == ys.len()
                && xs
                    .iter()
                    .zip(&ys)
                    .all(|(p, q)| match (p.parse::<f64>(), q.parse::<f64>()) {
                        (Ok(p), Ok(q)) => (p - q).abs() <= tolerance,
                        _ => p == q,
                    });
            if !same {
                problems.push(format!("{x} | {y}"));
            }
        }
    }
    problems
}

// ---------------------------------------------------------------------------------------
// `--formation-lifecycle SERVER CLIENT [--evidence DIR] [--port N]`.
//
// FORM-F3 on the same dedicated 1v1 fixture (20 idle units per seat), with the server's
// development casualty fixture armed. Seat 1's client, with real X input: puts one unit in
// group 2 and the army in group 1, forms the army with T, moves the formation (planned on
// the server), stops the group-2 unit on its own while the formation is moving, and lets the
// fixture kill another member. It then checks one bounded regroup of the members still
// following, their arrival and spacing, a second formation move that uses the stored shape,
// release with Y, re-forming, and that leaving the match retires the formation exactly once.
// ---------------------------------------------------------------------------------------

/// `RTS_FORMATION_VIEW` detail of one formation: id, shape, state, members, following,
/// regroups and whether it has a destination.
#[derive(Debug, Default, PartialEq)]
struct FormationView {
    id: u32,
    shape: String,
    state: String,
    members: usize,
    following: usize,
    regroups: u32,
    destination: bool,
    ids: Vec<u64>,
}

fn formation_views(line: &str) -> Vec<FormationView> {
    let Some(detail) = field(line, "detail") else {
        return Vec::new();
    };
    if detail == "none" {
        return Vec::new();
    }
    detail
        .split(';')
        .filter_map(|entry| {
            let parts: Vec<&str> = entry.split(':').collect();
            if parts.len() < 8 {
                return None;
            }
            let mut ids: Vec<u64> = parts[7]
                .trim_start_matches("ids=")
                .split('/')
                .filter_map(|v| v.parse().ok())
                .collect();
            ids.sort_unstable();
            Some(FormationView {
                id: parts[0].parse().ok()?,
                shape: parts[1].to_owned(),
                state: parts[2].to_owned(),
                members: parts[3].split('/').next()?.parse().ok()?,
                following: parts[4].trim_start_matches('f').parse().ok()?,
                regroups: parts[5].trim_start_matches('r').parse().ok()?,
                destination: parts[6] == "dest1",
                ids,
            })
        })
        .collect()
}

/// The client's latest owner-only formation view after `from`.
fn last_formation_view(client: &Game, from: usize) -> Option<String> {
    client.text()[from..]
        .lines()
        .rev()
        .find(|l| l.contains("RTS_FORMATION_VIEW"))
        .map(str::to_owned)
}

fn ids_field(line: &str, key: &str) -> Vec<u64> {
    let mut ids: Vec<u64> = field(line, key)
        .unwrap_or("")
        .split(',')
        .filter_map(|v| v.parse().ok())
        .collect();
    ids.sort_unstable();
    ids
}

/// Per unit: the resolved point of its latest formation slot order and whether it arrived
/// there (an OrderChanged Move -> arrival after that slot was sent).
fn formation_slot_states(text: &str, units: &[u64]) -> Vec<(u64, Option<(f64, f64)>, bool)> {
    let lines: Vec<&str> = text.lines().collect();
    units
        .iter()
        .map(|unit| {
            let id = unit.to_string();
            let Some(last) = lines.iter().rposition(|l| {
                l.contains("RTS_FORMATION_SLOT") && field(l, "unit") == Some(id.as_str())
            }) else {
                return (*unit, None, false);
            };
            let after = &lines[last + 1..];
            let resolved = after
                .iter()
                .find(|l| {
                    l.contains("RTS_NAVIGATION_READY") && field(l, "unit") == Some(id.as_str())
                })
                .and_then(|l| triple_xy(l, "resolved"));
            let arrived = after.iter().any(|l| {
                l.contains("RTS_GAMEPLAY_EVENT kind=OrderChanged")
                    && field(l, "entity") == Some(id.as_str())
                    && field(l, "previous") == Some("1")
                    && field(l, "reason") == Some("5")
            });
            (*unit, resolved, arrived)
        })
        .collect()
}

/// Waits until every unit arrived at its latest formation slot, then checks that each stopped
/// within 30 cm of it and that the survivors keep the spacing.
fn formation_arrival(
    server: &Game,
    from: usize,
    followers: &[u64],
    survivors: &[u64],
) -> (usize, usize, f64, usize) {
    let deadline = Instant::now() + Duration::from_secs(45);
    loop {
        let text = server.text()[from..].to_owned();
        let states = formation_slot_states(&text, followers);
        if states.iter().all(|s| s.2) || Instant::now() >= deadline {
            break;
        }
        thread::sleep(Duration::from_millis(500));
    }
    thread::sleep(Duration::from_millis(1500));
    let text = server.text()[from..].to_owned();
    let states = formation_slot_states(&text, followers);
    let arrived = states.iter().filter(|s| s.2).count();
    let at_slot = states
        .iter()
        .filter(|s| {
            s.1.zip(server_unit(&text, s.0).map(|u| u.0))
                .is_some_and(|(slot, at)| distance(slot, at) <= 30.0)
        })
        .count();
    let finals: Vec<(f64, f64)> = survivors
        .iter()
        .filter_map(|unit| server_unit(&server.text(), *unit).map(|s| s.0))
        .collect();
    let failed = text
        .lines()
        .filter(|l| {
            l.contains("RTS_NAVIGATION_FAILED")
                && field(l, "unit")
                    .and_then(|v| v.parse::<u64>().ok())
                    .is_some_and(|u| survivors.contains(&u))
        })
        .count();
    (arrived, at_slot, min_separation(&finals), failed)
}

#[allow(clippy::too_many_lines)]
fn formation_lifecycle_session(
    options: &DedicatedOptions,
    report: &mut Report,
) -> Result<(), String> {
    let client_options = &options.client;
    let evidence = &client_options.evidence;
    let port = client_options.port;
    let mut args = server_args(port, "1v1");
    args.push(format!("-PolitikaMPUnitScale={GROUP_UNITS_PER_SEAT}"));
    args.push("-PolitikaMPUnitScaleHold".into());
    args.push("-PolitikaMPFormationCasualty".into());
    let mut server = Game::launch_server(&options.server, "server", evidence, &args)?;
    report.fact("server_command", server.command.join(" "));
    server.wait(&["MP_DEDICATED_SERVER_READY"], Duration::from_secs(180))?;
    let awaiting = server.wait(&["MP_MATCH_AWAITING_PARTICIPANTS"], Duration::from_secs(60))?;
    let generation = field(&awaiting, "generation").unwrap_or("0").to_owned();
    let fixture = server.wait(&["MP_UNIT_SCALE_FIXTURE"], Duration::from_secs(5))?;

    let display_a = Display::start(evidence, "client-a")?;
    let mut args_a = client_args(client_options, true, false, false);
    args_a.push(format!(
        "-PolitikaMPGroundProbe={}:{}/{}:{}",
        SHAPE_LINE_POINT.0, SHAPE_LINE_POINT.1, SHAPE_COLUMN_POINT.0, SHAPE_COLUMN_POINT.1
    ));
    let mut client_a = Game::launch(
        &client_options.binary,
        "client-a",
        evidence,
        &args_a,
        Some(&display_a.name()),
    )?;
    report.fact("client_a_command", client_a.command.join(" "));
    server.wait(
        &["MP_PARTICIPANT_BOUND participant=1", "remote=1"],
        Duration::from_secs(90),
    )?;
    let mut client_b = Game::launch(
        &client_options.binary,
        "client-b",
        evidence,
        &client_args(client_options, false, false, false),
        None,
    )?;
    report.fact("client_b_command", client_b.command.join(" "));
    server.wait(
        &["MP_PARTICIPANT_BOUND participant=2", "remote=1"],
        Duration::from_secs(90),
    )?;
    server.wait(
        &["MP_MATCH_ACTIVE", &format!("generation={generation}")],
        Duration::from_secs(30),
    )?;
    client_a.wait(
        &[
            "MP_CLIENT_MATCH_OBSERVED",
            "map=map.test.multiplayer_skirmish",
        ],
        Duration::from_secs(60),
    )?;
    client_a.attach_window()?;
    thread::sleep(Duration::from_secs(6));

    let result = (|| -> Result<(), String> {
        let wait5 = |game: &mut Game, fragments: &[&str], from: usize| {
            game.wait_line(fragments, Duration::from_secs(5), from)
        };
        let owners = entity_owners(&server.text());
        let own: Vec<u64> = owners
            .iter()
            .filter(|(_, o)| *o == 1)
            .map(|(e, _)| *e)
            .collect();
        report.verdict(
            "FORM-FIXTURE-20-PER-SEAT",
            own.len() == GROUP_UNITS_PER_SEAT && field(&fixture, "per_seat") == Some("20"),
            format!("participant 1 units={}; {}", own.len(), fixture.trim()),
        );
        let box_all = |client: &mut Game| -> Result<(usize, String), String> {
            let before = client.offset();
            client.drag_hold(
                (24.0, 24.0),
                (f64::from(WIDTH) - 24.0, f64::from(HEIGHT) - 24.0),
            )?;
            client.release()?;
            let line = client.wait_line(
                &["RTS_SELECTION_SET", "source=box", "op=replace"],
                Duration::from_secs(5),
                before,
            )?;
            Ok((before, line))
        };

        // Setup: the lowest-ID clickable unit alone in group 2, the army in group 1.
        let (before_box, boxed) = box_all(&mut client_a)?;
        report.verdict(
            "FORM-01-BOX-SELECTS-OWN-UNITS",
            selection_ids(&boxed) == own,
            boxed.trim().to_owned(),
        );
        let screens: Vec<(u64, f64, f64)> = client_a.text()[before_box..]
            .lines()
            .filter(|l| l.contains("RTS_SELECTION_SCREEN"))
            .filter_map(|l| {
                Some((
                    field(l, "unit")?.parse().ok()?,
                    number(l, "x")?,
                    number(l, "y")?,
                ))
            })
            .filter(|(_, x, y)| {
                *x > 40.0 && *x < f64::from(WIDTH) - 40.0 && *y > 220.0 && *y < 500.0
            })
            .collect();
        let lone = screens
            .iter()
            .filter(|s| {
                screens
                    .iter()
                    .filter(|o| o.0 != s.0)
                    .all(|o| (s.1 - o.1).hypot(s.2 - o.2) >= 14.0)
            })
            .min_by_key(|s| s.0)
            .copied()
            .ok_or("no clickable own unit clear of its neighbours")?;
        let independent = lone.0;
        report.fact(
            "independent_unit",
            format!("{}@{:.0},{:.0}", lone.0, lone.1, lone.2),
        );
        let at = client_a.offset();
        client_a.click(lone.1, lone.2, "1")?;
        let clicked = wait5(
            &mut client_a,
            &["RTS_SELECTION_SET", "source=click", "count=1 "],
            at,
        )?;
        let at = client_a.offset();
        client_a.modified_key("ctrl", "2")?;
        let group2 = wait5(&mut client_a, &["RTS_CONTROL_GROUP_ASSIGN", "slot=2 "], at)?;
        box_all(&mut client_a)?;
        let at = client_a.offset();
        client_a.modified_key("ctrl", "1")?;
        let group1 = wait5(&mut client_a, &["RTS_CONTROL_GROUP_ASSIGN", "slot=1 "], at)?;
        report.verdict(
            "FORM-SETUP-GROUPS",
            selection_ids(&clicked) == vec![independent]
                && selection_ids(&group2) == vec![independent]
                && selection_ids(&group1) == own,
            format!("{} | {} | {}", clicked.trim(), group2.trim(), group1.trim()),
        );
        report.verdict(
            "FORM-02-GROUPS-CREATE-NO-FORMATION",
            !server.text().contains("RTS_FORMATION_FORMED"),
            "control groups and selection only".to_owned(),
        );

        // 1. T forms the army (Line, the default shape) as one match-owned formation.
        let at = client_a.offset();
        let at_server = server.offset();
        client_a.key("t")?;
        let sent = wait5(&mut client_a, &["RTS_FORMATION_SENT", "op=form"], at)?;
        let formed =
            server.wait_line(&["RTS_FORMATION_FORMED"], Duration::from_secs(5), at_server)?;
        let outcome = wait5(&mut client_a, &["RTS_FORMATION_OUTCOME", "op=0 "], at)?;
        let view = client_a.wait_line(
            &["RTS_FORMATION_VIEW", "source=replicated", "formations=1 "],
            Duration::from_secs(5),
            at,
        )?;
        let formation = field(&formed, "formation").unwrap_or("0").to_owned();
        let views = formation_views(&view);
        report.verdict(
            "FORM-F3-FORM-ADMITTED",
            field(&sent, "members") == Some("20")
                && ids_field(&formed, "ids") == own
                && field(&formed, "shape") == Some("line")
                && field(&formed, "participant") == Some("1")
                && field(&outcome, "accepted") == Some("1")
                && field(&outcome, "members") == Some("20")
                && field(&outcome, "formation") == Some(formation.as_str())
                && views.len() == 1
                && views[0].ids == own
                && views[0].state == "organized"
                && !views[0].destination,
            format!(
                "{} | {} | {} | {}",
                sent.trim(),
                formed.trim(),
                outcome.trim(),
                view.trim()
            ),
        );
        thread::sleep(Duration::from_millis(500));
        let formed_orders = server.text()[at_server..]
            .lines()
            .filter(|l| {
                (l.contains("MP_COMMAND_RECEIVED") && l.contains(" participant=1 "))
                    || l.contains("RTS_ORDER_ADMITTED") && l.contains(" participant=1 ")
            })
            .count();
        report.verdict(
            "FORM-02-FORM-ISSUES-NO-ORDER",
            formed_orders == 0,
            format!("unit commands during Form: {formed_orders}"),
        );
        let leaked = client_b
            .text()
            .lines()
            .filter(|l| l.contains("RTS_FORMATION_VIEW") && !l.contains("formations=0 "))
            .count();
        report.verdict(
            "FORM-F3-OWNER-ONLY",
            leaked == 0,
            format!("opposing client formation views: {leaked}"),
        );
        report.artifact("formed_png", &client_a.screenshot(evidence, "01-formed")?);

        // 2. RMB with exactly the formation selected: one formation request; the server plans
        // the slots on its navmesh and submits one ordinary Move per member.
        let line = wait_screen(&mut client_a, |t| probe_screen(t, 0), "line probe")?;
        let at = client_a.offset();
        let at_server = server.offset();
        client_a.click(line.0, line.1, "3")?;
        let move_sent = wait5(&mut client_a, &["RTS_FORMATION_SENT", "op=move"], at)?;
        let correlation = field(&move_sent, "correlation").unwrap_or("0").to_owned();
        let plan = server.wait_line(
            &["RTS_FORMATION_MOVE", &format!("formation={formation} ")],
            Duration::from_secs(5),
            at_server,
        )?;
        let move_outcome = wait5(
            &mut client_a,
            &[
                "RTS_FORMATION_OUTCOME",
                &format!("correlation={correlation} "),
            ],
            at,
        )?;
        let batch_sent = wait5(
            &mut client_a,
            &["RTS_GROUP_ORDER_SENT", &format!("formation={formation}")],
            at,
        )?;
        let batch = field(&batch_sent, "batch").unwrap_or("0").to_owned();

        // 3. While it moves: recall group 2 (the lone unit) and stop it on its own.
        thread::sleep(Duration::from_millis(400));
        let at_recall = client_a.offset();
        let at_stop_server = server.offset();
        client_a.key("2")?;
        let recalled = wait5(
            &mut client_a,
            &["RTS_SELECTION_SET", "source=group", "count=1 "],
            at_recall,
        )?;
        client_a.key("x")?;
        let independent_line = server.wait_line(
            &[
                "RTS_FORMATION_MEMBER_INDEPENDENT",
                &format!("unit={independent} "),
            ],
            Duration::from_secs(5),
            at_stop_server,
        )?;
        let group_outcome = client_a.wait_line(
            &["RTS_GROUP_ORDER_OUTCOME", &format!("batch={batch} ")],
            Duration::from_secs(15),
            at,
        )?;
        let move_text = server.text()[at_server..].to_owned();
        let slots: Vec<u64> = {
            let mut v: Vec<u64> = move_text
                .lines()
                .filter(|l| l.contains("RTS_FORMATION_SLOT") && l.contains("cause=move"))
                .filter_map(|l| field(l, "unit")?.parse().ok())
                .collect();
            v.sort_unstable();
            v
        };
        let admitted = move_text
            .lines()
            .filter(|l| {
                l.contains("RTS_ORDER_ADMITTED order=Move")
                    && field(l, "correlation") == Some(correlation.as_str())
            })
            .count();
        // Before the lone unit's own Stop, the client sent no unit command at all.
        let unit_intents = server.text()[at_server..at_stop_server]
            .lines()
            .filter(|l| l.contains("MP_COMMAND_RECEIVED") && l.contains(" participant=1 "))
            .count();
        report.verdict(
            "FORM-04-SERVER-PLAN",
            field(&plan, "members") == Some("20")
                && field(&plan, "orders") == Some("20")
                && field(&plan, "unplaced") == Some("0")
                && field(&plan, "shape") == Some("line")
                && number(&plan, "min_separation")
                    .is_some_and(|s| (s - SHAPE_SPACING_CM).abs() < 0.5)
                && slots == own,
            plan.trim().to_owned(),
        );
        report.verdict(
            "FORM-06-MEMBER-ORDERS-ADMITTED-INDIVIDUALLY",
            admitted == 20
                && unit_intents == 0
                && field(&move_outcome, "orders") == Some("20")
                && field(&group_outcome, "expected") == Some("20")
                && field(&group_outcome, "accepted") == Some("20"),
            format!(
                "{admitted} member Moves admitted under correlation {correlation}; client unit intents {unit_intents}; {}",
                group_outcome.trim()
            ),
        );
        let stop_received = server.text()[at_stop_server..].lines().any(|l| {
            l.contains("MP_COMMAND_RECEIVED")
                && l.contains(" order=3 ")
                && field(l, "issuer") == Some(independent.to_string().as_str())
        });
        report.verdict(
            "FORM-07-INDIVIDUAL-STOP-KEEPS-MEMBERSHIP",
            selection_ids(&recalled) == vec![independent]
                && stop_received
                && field(&independent_line, "following") == Some("19"),
            format!("{} | {}", recalled.trim(), independent_line.trim()),
        );

        // 4. The fixture kills the highest-ID member still following; one regroup follows the
        // hysteresis window, for the members still following only.
        let casualty = server.wait_line(
            &[
                "MP_FORMATION_CASUALTY formation=",
                &format!("formation={formation} "),
            ],
            Duration::from_secs(10),
            at_server,
        )?;
        let lost: u64 = field(&casualty, "unit")
            .and_then(|v| v.parse().ok())
            .unwrap_or(0);
        let left = server.wait_line(
            &[
                "RTS_FORMATION_MEMBER_LEFT",
                &format!("unit={lost} "),
                "reason=destroyed",
            ],
            Duration::from_secs(5),
            at_server,
        )?;
        let scheduled = server.wait_line(
            &[
                "RTS_FORMATION_REGROUP_SCHEDULED",
                &format!("formation={formation} "),
            ],
            Duration::from_secs(5),
            at_server,
        )?;
        let regroup = server.wait_line(
            &["RTS_FORMATION_REGROUP ", &format!("formation={formation} ")],
            Duration::from_secs(10),
            at_server,
        )?;
        let followers: Vec<u64> = own
            .iter()
            .copied()
            .filter(|u| *u != independent && *u != lost)
            .collect();
        let survivors: Vec<u64> = own.iter().copied().filter(|u| *u != lost).collect();
        let loss_step: u64 = field(&left, "step")
            .and_then(|v| v.parse().ok())
            .unwrap_or(0);
        let regroup_step: u64 = field(&regroup, "step")
            .and_then(|v| v.parse().ok())
            .unwrap_or(0);
        // DISC-D3: the survivors' Discipline scales the 40-step window; the server logs the
        // scaled delay whenever it differs from 40.
        let delay_line = server.text()[at_server..]
            .lines()
            .find(|l| {
                l.contains("RTS_DISCIPLINE_EFFECT")
                    && l.contains(" effect=regroup_delay ")
                    && l.contains(&format!(" formation={formation} "))
            })
            .map(str::to_owned);
        let regroup_delay: u64 = delay_line.as_deref().map_or(40, |l| {
            let permille = discipline_effect_permille(int_field(l, "value").unwrap_or(-1), -3, 8);
            if int_field(l, "permille") == Some(permille) {
                u64::try_from(((40 * permille + 500) / 1000).max(1)).unwrap_or(0)
            } else {
                0
            }
        });
        report.verdict(
            "FORM-09-LOSS-DEGRADES-AND-SCHEDULES-ONE-REGROUP",
            lost != 0
                && lost != independent
                && lost == *followers.iter().chain([lost].iter()).max().unwrap_or(&0)
                && field(&left, "remaining") == Some("19")
                && field(&scheduled, "due_step").and_then(|v| v.parse::<u64>().ok())
                    == Some(loss_step + regroup_delay)
                && regroup_step == loss_step + regroup_delay
                && delay_line
                    .as_deref()
                    .is_none_or(|l| int_field(l, "steps") == i64::try_from(regroup_delay).ok()),
            format!(
                "{} | {} | {} | {} | regroup delay {regroup_delay} steps ({})",
                casualty.trim(),
                left.trim(),
                scheduled.trim(),
                regroup.trim(),
                delay_line.as_deref().map_or("baseline", str::trim)
            ),
        );
        let regroup_units: Vec<u64> = server.text()[at_server..]
            .lines()
            .filter(|l| l.contains("RTS_FORMATION_SLOT") && l.contains("cause=regroup"))
            .filter_map(|l| field(l, "unit")?.parse().ok())
            .collect();
        report.verdict(
            "FORM-07-REGROUP-LEAVES-INDEPENDENT-MEMBER",
            field(&regroup, "members") == Some("19")
                && field(&regroup, "following") == Some("18")
                && field(&regroup, "planned") == Some("18")
                && field(&regroup, "unplaced") == Some("0")
                && !regroup_units.is_empty()
                && regroup_units.iter().all(|u| followers.contains(u)),
            format!(
                "{} regroup orders for {:?}",
                regroup_units.len(),
                regroup_units
            ),
        );
        let (arrived, at_slot, separation, failed) =
            formation_arrival(&server, at_server, &followers, &survivors);
        thread::sleep(Duration::from_secs(4));
        let after_text = server.text()[at_server..].to_owned();
        let regroups = after_text
            .lines()
            .filter(|l| l.contains("RTS_FORMATION_REGROUP "))
            .count();
        let stopped_followers = after_text
            .lines()
            .filter(|l| {
                l.contains("RTS_ORDER_ADMITTED")
                    && !l.contains("order=Move")
                    && field(l, "unit")
                        .and_then(|v| v.parse::<u64>().ok())
                        .is_some_and(|u| followers.contains(&u))
            })
            .count();
        report.verdict(
            "FORM-08-ARRIVED-SPACED-AFTER-ONE-REGROUP",
            arrived == followers.len()
                && at_slot == followers.len()
                && failed == 0
                && separation >= SHAPE_SPACING_CM - 20.0
                && regroups == 1,
            format!(
                "{arrived}/{} followers arrived, {at_slot} within 30 cm of their slot, {failed} navigation failures, survivors' min separation {separation:.1} cm, regroups {regroups}",
                followers.len()
            ),
        );
        report.verdict(
            "FORM-07-OTHER-MEMBERS-UNAFFECTED",
            stopped_followers == 0,
            format!("non-formation orders admitted for following members: {stopped_followers}"),
        );
        let view = last_formation_view(&client_a, at).unwrap_or_default();
        let views = formation_views(&view);
        report.verdict(
            "FORM-F3-CLIENT-VIEW-AFTER-REGROUP",
            views.len() == 1
                && views[0].members == 19
                && views[0].following == 18
                && views[0].regroups == 1
                && views[0].state == "organized"
                && views[0].destination
                && views[0].ids == survivors,
            view.trim().to_owned(),
        );
        report.artifact(
            "regrouped_png",
            &client_a.screenshot(evidence, "02-regrouped")?,
        );

        // 5. V changes only the local shape; recalling group 1 (19 living) is the whole
        // formation again, and its move keeps the formation's stored Line shape.
        let at = client_a.offset();
        let at_server = server.offset();
        client_a.key("v")?;
        wait5(&mut client_a, &["RTS_GROUP_MOVE_SHAPE", "shape=column"], at)?;
        client_a.key("1")?;
        let recalled = wait5(&mut client_a, &["RTS_SELECTION_SET", "source=group"], at)?;
        thread::sleep(Duration::from_millis(300));
        report.artifact(
            "whole_formation_png",
            &client_a.screenshot(evidence, "03-whole-formation-selected")?,
        );
        let column = wait_screen(&mut client_a, |t| probe_screen(t, 1), "column probe")?;
        client_a.click(column.0, column.1, "3")?;
        let second = server.wait_line(
            &["RTS_FORMATION_MOVE", &format!("formation={formation} ")],
            Duration::from_secs(5),
            at_server,
        )?;
        let (arrived, at_slot, separation, failed) =
            formation_arrival(&server, at_server, &survivors, &survivors);
        report.verdict(
            "FORM-10-SECOND-MOVE-USES-FORMATION-SHAPE",
            selection_ids(&recalled) == survivors
                && field(&second, "shape") == Some("line")
                && field(&second, "members") == Some("19")
                && field(&second, "orders") == Some("19")
                && arrived == survivors.len()
                && at_slot == survivors.len()
                && failed == 0
                && separation >= SHAPE_SPACING_CM - 20.0,
            format!(
                "{} | {arrived}/19 arrived, {at_slot} at slot, {failed} failures, min separation {separation:.1} cm",
                second.trim()
            ),
        );

        // 6. Y releases every member: the formation dissolves, and the next RMB is an ordinary
        // client-planned group move in the local (Column) shape.
        let at = client_a.offset();
        let at_server = server.offset();
        client_a.key("y")?;
        let dissolved = server.wait_line(
            &[
                "RTS_FORMATION_DISSOLVED",
                &format!("formation={formation} "),
                "reason=released",
            ],
            Duration::from_secs(5),
            at_server,
        )?;
        let emptied = client_a.wait_line(
            &["RTS_FORMATION_VIEW", "source=replicated", "formations=0 "],
            Duration::from_secs(5),
            at,
        )?;
        let line = wait_screen(&mut client_a, |t| probe_screen(t, 0), "line probe")?;
        client_a.click(line.0, line.1, "3")?;
        let ordinary = wait5(&mut client_a, &["RTS_GROUP_MOVE_PLAN", "order=0"], at)?;
        let formation_sends = client_a.text()[at..]
            .lines()
            .filter(|l| l.contains("RTS_FORMATION_SENT") && !l.contains("op=release"))
            .count();
        report.verdict(
            "FORM-09-RELEASE-DISSOLVES",
            field(&ordinary, "shape") == Some("column")
                && field(&ordinary, "members") == Some("19")
                && formation_sends == 0,
            format!(
                "{} | {} | {}",
                dissolved.trim(),
                emptied.trim(),
                ordinary.trim()
            ),
        );

        // 7. T again forms a new formation (new ID, the current Column shape).
        thread::sleep(Duration::from_secs(2));
        let at_server = server.offset();
        client_a.key("t")?;
        let reformed =
            server.wait_line(&["RTS_FORMATION_FORMED"], Duration::from_secs(5), at_server)?;
        let reformed_id = field(&reformed, "formation").unwrap_or("0").to_owned();
        report.verdict(
            "FORM-F3-REFORM-NEW-ID",
            reformed_id != formation
                && reformed_id.parse::<u32>().ok() > formation.parse::<u32>().ok()
                && field(&reformed, "shape") == Some("column")
                && field(&reformed, "members") == Some("19"),
            reformed.trim().to_owned(),
        );
        let ended = server.text().lines().any(|l| {
            l.contains("MATCH_LIFECYCLE phase=5")
                || l.contains("RTS_OBJECTIVE_STATE") && l.contains("complete=1")
        });
        report.verdict(
            "FORM-MATCH-STAYS-ACTIVE",
            !ended,
            format!("terminal or captured during the session: {ended}"),
        );
        report.artifact(
            "reformed_png",
            &client_a.screenshot(evidence, "04-reformed")?,
        );

        // 8. Leaving ends the match; the formation is retired exactly once and nothing of it
        // acts afterwards.
        let at_server = server.offset();
        client_a.key("Escape")?;
        let cleared = server.wait_line(
            &["RTS_FORMATIONS_CLEARED"],
            Duration::from_secs(15),
            at_server,
        )?;
        thread::sleep(Duration::from_secs(3));
        let after = server.text()[at_server..].to_owned();
        let clears = after
            .lines()
            .filter(|l| l.contains("RTS_FORMATIONS_CLEARED"))
            .count();
        let cleared_at = after.find("RTS_FORMATIONS_CLEARED").unwrap_or(0);
        let late = after[cleared_at..]
            .lines()
            .skip(1)
            .filter(|l| l.contains("RTS_FORMATION_") && !l.contains("RTS_FORMATIONS_CLEARED"))
            .count();
        report.verdict(
            "FORM-15-TERMINAL-RETIRES-ONCE",
            field(&cleared, "reason") == Some("match_ended")
                && field(&cleared, "count") == Some("1")
                && field(&cleared, "ids") == Some(reformed_id.as_str())
                && clears == 1
                && late == 0,
            format!(
                "{}; clears {clears}; formation lines after the clear {late}",
                cleared.trim()
            ),
        );
        Ok(())
    })();
    launch_options_verdict(
        report,
        "FORM-F3-DEV-LAUNCH-OPTIONS",
        &[&server, &client_a, &client_b],
    );
    report.artifact("server_log", &server.log);
    report.artifact("client_a_log", &client_a.log);
    report.artifact("client_b_log", &client_b.log);
    client_a.stop();
    client_b.stop();
    server.stop();
    drop(display_a);
    result
}

// ---------------------------------------------------------------------------------------
// `--formation-stance SERVER CLIENT [--evidence DIR] [--port N]`.
//
// FORM-F4 on the same dedicated 1v1 fixture (20 idle units per seat), with the server's
// development stance probe armed. Seat 1's client, with real X input: forms the army with T,
// moves the formation into a Line, then presses H (Guard). The probe walks one opposing unit
// to 740 cm from the nearest member (inside acquisition, outside weapon range): the nearest
// members chase it inside their 600 cm leash, kill it and walk back to their posts while the
// rest hold. J cycles adherence. H twice more reaches Hold Fire; the probe walks a second
// opposing unit to 450 cm (inside weapon range) and it fires while the formation holds its
// fire. A last H returns to Hold Ground and the formation fires again. No stance change sends
// a unit order; the opposing client never sees the formation or its stance.
// ---------------------------------------------------------------------------------------

/// The stance and adherence fields that FORM-F4 appends to each `RTS_FORMATION_VIEW` entry.
fn formation_view_stance(line: &str) -> Option<(String, String)> {
    let detail = field(line, "detail")?;
    let entry = detail.split(';').next()?;
    let mut stance = None;
    let mut adherence = None;
    for part in entry.split(':') {
        if let Some(value) = part.strip_prefix("stance=") {
            stance = Some(value.to_owned());
        } else if let Some(value) = part.strip_prefix("adherence=") {
            adherence = Some(value.to_owned());
        }
    }
    Some((stance?, adherence?))
}

/// Shots decided for `shooters` in `text` (one cue per receiving connection; counted once).
fn shots_by(text: &str, shooters: &[u64]) -> usize {
    let mut seen: Vec<String> = text
        .lines()
        .filter(|l| l.contains("MP_SHOT_CUE_SENT"))
        .filter(|l| {
            field(l, "shooter")
                .and_then(|v| v.parse::<u64>().ok())
                .is_some_and(|s| shooters.contains(&s))
        })
        .filter_map(|l| field(l, "sequence").map(str::to_owned))
        .collect();
    seen.sort();
    seen.dedup();
    seen.len()
}

fn unit_field(line: &str) -> u64 {
    field(line, "unit")
        .and_then(|v| v.parse().ok())
        .unwrap_or(0)
}

#[allow(clippy::too_many_lines)]
fn formation_stance_session(options: &DedicatedOptions, report: &mut Report) -> Result<(), String> {
    let client_options = &options.client;
    let evidence = &client_options.evidence;
    let port = client_options.port;
    let mut args = server_args(port, "1v1");
    args.push(format!("-PolitikaMPUnitScale={GROUP_UNITS_PER_SEAT}"));
    args.push("-PolitikaMPUnitScaleHold".into());
    args.push("-PolitikaMPStanceProbe".into());
    let mut server = Game::launch_server(&options.server, "server", evidence, &args)?;
    report.fact("server_command", server.command.join(" "));
    server.wait(&["MP_DEDICATED_SERVER_READY"], Duration::from_secs(180))?;
    let awaiting = server.wait(&["MP_MATCH_AWAITING_PARTICIPANTS"], Duration::from_secs(60))?;
    let generation = field(&awaiting, "generation").unwrap_or("0").to_owned();
    let fixture = server.wait(&["MP_UNIT_SCALE_FIXTURE"], Duration::from_secs(5))?;

    let display_a = Display::start(evidence, "client-a")?;
    let mut args_a = client_args(client_options, true, false, false);
    args_a.push(format!(
        "-PolitikaMPGroundProbe={}:{}/{}:{}",
        SHAPE_LINE_POINT.0, SHAPE_LINE_POINT.1, SHAPE_COLUMN_POINT.0, SHAPE_COLUMN_POINT.1
    ));
    let mut client_a = Game::launch(
        &client_options.binary,
        "client-a",
        evidence,
        &args_a,
        Some(&display_a.name()),
    )?;
    report.fact("client_a_command", client_a.command.join(" "));
    server.wait(
        &["MP_PARTICIPANT_BOUND participant=1", "remote=1"],
        Duration::from_secs(90),
    )?;
    let mut client_b = Game::launch(
        &client_options.binary,
        "client-b",
        evidence,
        &client_args(client_options, false, false, false),
        None,
    )?;
    report.fact("client_b_command", client_b.command.join(" "));
    server.wait(
        &["MP_PARTICIPANT_BOUND participant=2", "remote=1"],
        Duration::from_secs(90),
    )?;
    server.wait(
        &["MP_MATCH_ACTIVE", &format!("generation={generation}")],
        Duration::from_secs(30),
    )?;
    client_a.wait(
        &[
            "MP_CLIENT_MATCH_OBSERVED",
            "map=map.test.multiplayer_skirmish",
        ],
        Duration::from_secs(60),
    )?;
    client_a.attach_window()?;
    thread::sleep(Duration::from_secs(6));

    let result = (|| -> Result<(), String> {
        let wait5 = |game: &mut Game, fragments: &[&str], from: usize| {
            game.wait_line(fragments, Duration::from_secs(5), from)
        };
        let owners = entity_owners(&server.text());
        let own: Vec<u64> = owners
            .iter()
            .filter(|(_, o)| *o == 1)
            .map(|(e, _)| *e)
            .collect();
        report.verdict(
            "FORM-FIXTURE-20-PER-SEAT",
            own.len() == GROUP_UNITS_PER_SEAT && field(&fixture, "per_seat") == Some("20"),
            format!("participant 1 units={}; {}", own.len(), fixture.trim()),
        );
        let own_lines = |text: &str, tag: &str| -> Vec<String> {
            text.lines()
                .filter(|l| l.contains(tag) && own.contains(&unit_field(l)))
                .map(str::to_owned)
                .collect()
        };
        let unit_orders = |text: &str| {
            text.lines()
                .filter(|l| {
                    (l.contains("MP_COMMAND_RECEIVED") || l.contains("RTS_ORDER_ADMITTED"))
                        && l.contains(" participant=1 ")
                })
                .count()
        };

        // 1. Box-select the army and form it (Line); the defaults are Hold Ground / Flexible.
        let before = client_a.offset();
        client_a.drag_hold(
            (24.0, 24.0),
            (f64::from(WIDTH) - 24.0, f64::from(HEIGHT) - 24.0),
        )?;
        client_a.release()?;
        let boxed = client_a.wait_line(
            &["RTS_SELECTION_SET", "source=box", "op=replace"],
            Duration::from_secs(5),
            before,
        )?;
        let at = client_a.offset();
        let at_server = server.offset();
        client_a.key("t")?;
        let formed =
            server.wait_line(&["RTS_FORMATION_FORMED"], Duration::from_secs(5), at_server)?;
        let formation = field(&formed, "formation").unwrap_or("0").to_owned();
        let view = client_a.wait_line(
            &["RTS_FORMATION_VIEW", "source=replicated", "formations=1 "],
            Duration::from_secs(5),
            at,
        )?;
        report.verdict(
            "FORM-F4-DEFAULTS",
            selection_ids(&boxed) == own
                && ids_field(&formed, "ids") == own
                && formation_view_stance(&view)
                    == Some(("hold_ground".to_owned(), "flexible".to_owned())),
            format!("{} | {}", formed.trim(), view.trim()),
        );

        // 2. Move the formation into its Line; each member's slot becomes its post.
        let line = wait_screen(&mut client_a, |t| probe_screen(t, 0), "line probe")?;
        let at_server = server.offset();
        client_a.click(line.0, line.1, "3")?;
        server.wait_line(
            &["RTS_FORMATION_MOVE", &format!("formation={formation} ")],
            Duration::from_secs(5),
            at_server,
        )?;
        let (arrived, at_slot, separation, failed) =
            formation_arrival(&server, at_server, &own, &own);
        report.verdict(
            "FORM-F4-SETUP-LINE-ARRIVED",
            arrived == own.len() && at_slot == own.len() && failed == 0,
            format!(
                "{arrived}/20 arrived, {at_slot} at slot, {failed} failures, min separation {separation:.1} cm"
            ),
        );
        thread::sleep(Duration::from_secs(2));
        let posts: Vec<(u64, (f64, f64))> = own
            .iter()
            .filter_map(|u| server_unit(&server.text(), *u).map(|s| (*u, s.0)))
            .collect();

        // 3. H with the whole formation selected: Guard. A stance change sends no unit order.
        let at = client_a.offset();
        let at_server = server.offset();
        client_a.key("h")?;
        let sent = wait5(&mut client_a, &["RTS_FORMATION_SENT", "op=set_stance"], at)?;
        let set = server.wait_line(
            &[
                "RTS_STANCE_SET",
                &format!("formation={formation} "),
                "stance=guard",
            ],
            Duration::from_secs(5),
            at_server,
        )?;
        let outcome = wait5(&mut client_a, &["RTS_FORMATION_OUTCOME", "op=5 "], at)?;
        let view = client_a.wait_line(
            &["RTS_FORMATION_VIEW", "source=replicated", ":stance=guard"],
            Duration::from_secs(5),
            at,
        )?;
        thread::sleep(Duration::from_millis(500));
        let applied = own_lines(&server.text()[at_server..], "RTS_STANCE_APPLIED")
            .iter()
            .filter(|l| field(l, "stance") == Some("guard") && field(l, "leash") == Some("600"))
            .count();
        let orders = unit_orders(&server.text()[at_server..]);
        report.verdict(
            "FORM-F4-STANCE-IS-NOT-AN-ORDER",
            field(&sent, "formation") == Some(formation.as_str())
                && field(&set, "members") == Some("20")
                && field(&outcome, "accepted") == Some("1")
                && field(&outcome, "orders") == Some("0")
                && applied == own.len()
                && orders == 0,
            format!(
                "{} | {} | {}; members applying guard {applied}; unit orders {orders}",
                set.trim(),
                outcome.trim(),
                view.trim()
            ),
        );
        report.artifact("guard_png", &client_a.screenshot(evidence, "01-guard")?);

        // 4. The probe walks an opposing unit to 740 cm: the nearest members chase it inside the
        // leash, it dies, and they walk back to their posts. Nobody else moves.
        let probe = server.wait_line(
            &["MP_STANCE_PROBE", "phase=guard"],
            Duration::from_secs(5),
            at_server,
        )?;
        let probe_unit = unit_field(&probe);
        let destroyed = server.wait_line(
            &["RTS_UNIT_DESTROYED", &format!("unit={probe_unit}")],
            Duration::from_secs(60),
            at_server,
        );
        let deadline = Instant::now() + Duration::from_secs(30);
        let (engaged, disengaged, returned) = loop {
            let text = server.text()[at_server..].to_owned();
            let engaged: Vec<String> = own_lines(&text, "RTS_STANCE_ENGAGE");
            let disengaged: Vec<String> = own_lines(&text, "RTS_STANCE_DISENGAGE");
            let returned: Vec<String> = own_lines(&text, "RTS_STANCE_RETURN unit=")
                .into_iter()
                .filter(|l| l.contains("phase=arrived") || l.contains("phase=failed"))
                .collect();
            // Settled: every chaser disengaged, and every one walking back reached its post.
            let all_disengaged = !engaged.is_empty()
                && engaged
                    .iter()
                    .all(|e| disengaged.iter().any(|d| unit_field(d) == unit_field(e)));
            let returning = disengaged
                .iter()
                .filter(|l| l.contains(" return=1 "))
                .count();
            if (destroyed.is_ok() && all_disengaged && returned.len() >= returning)
                || Instant::now() >= deadline
            {
                break (engaged, disengaged, returned);
            }
            thread::sleep(Duration::from_millis(500));
        };
        thread::sleep(Duration::from_secs(2));
        let engaged_units: Vec<u64> = {
            let mut v: Vec<u64> = engaged.iter().map(|l| unit_field(l)).collect();
            v.sort_unstable();
            v.dedup();
            v
        };
        report.verdict(
            "FORM-F4-GUARD-ENGAGES",
            field(&probe, "placed") == Some("1")
                && number(&probe, "nearest_member_cm").is_some_and(|d| (d - 740.0).abs() < 5.0)
                && !engaged_units.is_empty()
                && engaged.iter().all(|l| {
                    field(l, "stance") == Some("guard")
                        && field(l, "leash") == Some("600")
                        && field(l, "target").and_then(|v| v.parse::<u64>().ok())
                            == Some(probe_unit)
                })
                && destroyed.is_ok(),
            format!(
                "{} | {} engaged: {engaged_units:?} | {}",
                probe.trim(),
                engaged.len(),
                destroyed
                    .as_deref()
                    .unwrap_or("probe unit not destroyed")
                    .trim()
            ),
        );
        let max_displacement = disengaged
            .iter()
            .filter_map(|l| number(l, "max_displacement"))
            .fold(0.0_f64, f64::max);
        report.verdict(
            "FORM-08-GUARD-LEASH-BOUNDED",
            !disengaged.is_empty()
                && max_displacement > 0.0
                && max_displacement <= 600.0 + 5.0
                && disengaged
                    .iter()
                    .all(|l| field(l, "reason") != Some("route_failed")),
            format!(
                "{} disengagements, max displacement from post {max_displacement:.1} cm (leash 600)",
                disengaged.len()
            ),
        );
        let returning: Vec<u64> = disengaged
            .iter()
            .filter(|l| l.contains(" return=1 "))
            .map(|l| unit_field(l))
            .collect();
        let back: Vec<u64> = returned
            .iter()
            .filter(|l| l.contains("phase=arrived"))
            .map(|l| unit_field(l))
            .collect();
        let at_posts = posts
            .iter()
            .filter(|(u, post)| {
                server_unit(&server.text(), *u).is_some_and(|s| distance(s.0, *post) <= 30.0)
            })
            .count();
        let survivors = posts
            .iter()
            .filter(|(u, _)| server_unit(&server.text(), *u).is_some_and(|s| s.2 > 0))
            .count();
        report.verdict(
            "FORM-F4-GUARD-RETURNS-TO-POST",
            !returning.is_empty()
                && returning.iter().all(|u| back.contains(u))
                && at_posts == survivors,
            format!(
                "returning {returning:?}, arrived {back:?}; {at_posts}/{survivors} living members within 30 cm of their post"
            ),
        );
        let idle_orders = unit_orders(&server.text()[at_server..]);
        let independent = server.text()[at_server..]
            .lines()
            .filter(|l| l.contains("RTS_FORMATION_MEMBER_INDEPENDENT"))
            .count();
        report.verdict(
            "FORM-07-AUTONOMY-IS-NOT-AN-ORDER",
            idle_orders == 0 && independent == 0,
            format!("unit orders during the engagement {idle_orders}; members made independent {independent}"),
        );
        report.artifact(
            "returned_png",
            &client_a.screenshot(evidence, "02-returned")?,
        );

        // 5. J cycles adherence: Maintain halves the leash, Free stops returning, then Flexible.
        let mut cycle = Vec::new();
        for (expected, leash, back) in [
            ("maintain", "300", "1"),
            ("free", "600", "0"),
            ("flexible", "600", "1"),
        ] {
            let at = client_a.offset();
            let at_server = server.offset();
            client_a.key("j")?;
            let view = client_a.wait_line(
                &[
                    "RTS_FORMATION_VIEW",
                    "source=replicated",
                    &format!(":adherence={expected}"),
                ],
                Duration::from_secs(5),
                at,
            )?;
            thread::sleep(Duration::from_millis(400));
            let applied = own_lines(&server.text()[at_server..], "RTS_STANCE_APPLIED")
                .iter()
                .filter(|l| {
                    field(l, "adherence") == Some(expected)
                        && field(l, "leash") == Some(leash)
                        && field(l, "return") == Some(back)
                })
                .count();
            cycle.push((expected, applied, view.trim().to_owned()));
        }
        report.verdict(
            "FORM-F4-ADHERENCE-CYCLES",
            cycle.iter().all(|(_, applied, _)| *applied == survivors),
            format!("{cycle:?}"),
        );

        // 6. H twice: Aggressive, then Hold Fire. The probe walks a second opposing unit into
        // weapon range; it fires at the formation, which holds its fire.
        let at = client_a.offset();
        let at_server = server.offset();
        client_a.key("h")?;
        client_a.wait_line(
            &[
                "RTS_FORMATION_VIEW",
                "source=replicated",
                ":stance=aggressive",
            ],
            Duration::from_secs(5),
            at,
        )?;
        let at = client_a.offset();
        client_a.key("h")?;
        client_a.wait_line(
            &[
                "RTS_FORMATION_VIEW",
                "source=replicated",
                ":stance=hold_fire",
            ],
            Duration::from_secs(5),
            at,
        )?;
        let probe2 = server.wait_line(
            &["MP_STANCE_PROBE", "phase=hold_fire"],
            Duration::from_secs(5),
            at_server,
        )?;
        let probe2_unit = unit_field(&probe2);
        let target = (
            number(&probe2, "x").unwrap_or(0.0),
            number(&probe2, "y").unwrap_or(0.0),
        );
        wait_arrival(
            &mut server,
            probe2_unit,
            target,
            60.0,
            Duration::from_secs(40),
        )?;
        let window = server.offset();
        thread::sleep(Duration::from_secs(5));
        let window_text = server.text()[window..].to_owned();
        let own_shots = shots_by(&window_text, &own);
        let probe_shots = shots_by(&window_text, &[probe2_unit]);
        let moved = own_lines(&window_text, "RTS_STANCE_ENGAGE").len();
        report.verdict(
            "FORM-F4-HOLD-FIRE-HOLDS",
            field(&probe2, "placed") == Some("1")
                && number(&probe2, "nearest_member_cm").is_some_and(|d| (d - 450.0).abs() < 5.0)
                && own_shots == 0
                && probe_shots > 0
                && moved == 0,
            format!(
                "{}; in 5 s the probe fired {probe_shots} shots, the Hold Fire formation {own_shots}, chases {moved}",
                probe2.trim()
            ),
        );
        report.artifact(
            "hold_fire_png",
            &client_a.screenshot(evidence, "03-hold-fire")?,
        );

        // 7. H once more: Hold Ground (the default). The formation fires again, without chasing.
        let at = client_a.offset();
        let at_server = server.offset();
        client_a.key("h")?;
        client_a.wait_line(
            &[
                "RTS_FORMATION_VIEW",
                "source=replicated",
                ":stance=hold_ground",
            ],
            Duration::from_secs(5),
            at,
        )?;
        let killed = server.wait_line(
            &["RTS_UNIT_DESTROYED", &format!("unit={probe2_unit}")],
            Duration::from_secs(30),
            at_server,
        );
        let text = server.text()[at_server..].to_owned();
        let fired = shots_by(&text, &own);
        let chases = own_lines(&text, "RTS_STANCE_ENGAGE").len();
        report.verdict(
            "FORM-F4-HOLD-GROUND-FIRES-AGAIN",
            killed.is_ok() && fired > 0 && chases == 0,
            format!(
                "{}; formation shots {fired}; chases {chases}",
                killed
                    .as_deref()
                    .unwrap_or("probe unit not destroyed")
                    .trim()
            ),
        );

        // 8. The opposing client never receives the formation or its stance.
        let leaked = client_b
            .text()
            .lines()
            .filter(|l| {
                (l.contains("RTS_FORMATION_VIEW") && !l.contains("formations=0 "))
                    || l.contains("RTS_STANCE")
            })
            .count();
        report.verdict(
            "FORM-F4-OWNER-ONLY",
            leaked == 0,
            format!("opposing client formation or stance lines: {leaked}"),
        );
        let ended = server.text().lines().any(|l| {
            l.contains("MATCH_LIFECYCLE phase=5")
                || l.contains("RTS_OBJECTIVE_STATE") && l.contains("complete=1")
        });
        report.verdict(
            "FORM-MATCH-STAYS-ACTIVE",
            !ended,
            format!("terminal or captured during the session: {ended}"),
        );
        let at_server = server.offset();
        client_a.key("Escape")?;
        let cleared = server.wait_line(
            &["RTS_FORMATIONS_CLEARED"],
            Duration::from_secs(15),
            at_server,
        )?;
        report.verdict(
            "FORM-15-TERMINAL-RETIRES-ONCE",
            field(&cleared, "reason") == Some("match_ended")
                && field(&cleared, "count") == Some("1"),
            cleared.trim().to_owned(),
        );
        Ok(())
    })();
    launch_options_verdict(
        report,
        "FORM-F4-DEV-LAUNCH-OPTIONS",
        &[&server, &client_a, &client_b],
    );
    report.artifact("server_log", &server.log);
    report.artifact("client_a_log", &client_a.log);
    report.artifact("client_b_log", &client_b.log);
    client_a.stop();
    client_b.stop();
    server.stop();
    drop(display_a);
    result
}

// ---------------------------------------------------------------------------------------
// `--formation-discipline SERVER CLIENT [--evidence DIR] [--port N]`.
//
// DISC-D1 on the same dedicated 1v1 fixture (20 idle units per seat), with the server's
// development stance probe armed. Seat 1's client, with real X input: puts one lone unit in
// control group 2 and the army in group 1, forms the army with T and moves it into a Line.
// Every unit starts at Discipline 50; members earn 1 point per second up to 80. An individual
// Stop on the group-2 unit makes it independent and it holds its value. H (Guard) makes the
// probe walk an opposing unit next to the formation, so the opposing client receives the
// formation's units for a while: their replicated Discipline must read the baseline there.
// Y releases the formation and every unit fades back to 50, never below it.
// ---------------------------------------------------------------------------------------

/// Integer value of ` key=value`.
fn int_field(line: &str, key: &str) -> Option<i64> {
    field(line, key)?.parse().ok()
}

/// `RTS_DISCIPLINE_PARTICIPANT participant=1` lines in `text`.
fn discipline_participant_samples(text: &str) -> Vec<String> {
    text.lines()
        .filter(|l| l.contains("RTS_DISCIPLINE_PARTICIPANT") && l.contains(" participant=1 "))
        .map(str::to_owned)
        .collect()
}

#[allow(clippy::too_many_lines)]
fn formation_discipline_session(
    options: &DedicatedOptions,
    report: &mut Report,
) -> Result<(), String> {
    let client_options = &options.client;
    let evidence = &client_options.evidence;
    let port = client_options.port;
    let mut args = server_args(port, "1v1");
    args.push(format!("-PolitikaMPUnitScale={GROUP_UNITS_PER_SEAT}"));
    args.push("-PolitikaMPUnitScaleHold".into());
    args.push("-PolitikaMPStanceProbe".into());
    let mut server = Game::launch_server(&options.server, "server", evidence, &args)?;
    report.fact("server_command", server.command.join(" "));
    server.wait(&["MP_DEDICATED_SERVER_READY"], Duration::from_secs(180))?;
    let awaiting = server.wait(&["MP_MATCH_AWAITING_PARTICIPANTS"], Duration::from_secs(60))?;
    let generation = field(&awaiting, "generation").unwrap_or("0").to_owned();
    let fixture = server.wait(&["MP_UNIT_SCALE_FIXTURE"], Duration::from_secs(5))?;

    let display_a = Display::start(evidence, "client-a")?;
    let mut args_a = client_args(client_options, true, false, false);
    args_a.push(format!(
        "-PolitikaMPGroundProbe={}:{}/{}:{}",
        SHAPE_LINE_POINT.0, SHAPE_LINE_POINT.1, SHAPE_COLUMN_POINT.0, SHAPE_COLUMN_POINT.1
    ));
    let mut client_a = Game::launch(
        &client_options.binary,
        "client-a",
        evidence,
        &args_a,
        Some(&display_a.name()),
    )?;
    report.fact("client_a_command", client_a.command.join(" "));
    server.wait(
        &["MP_PARTICIPANT_BOUND participant=1", "remote=1"],
        Duration::from_secs(90),
    )?;
    let mut client_b = Game::launch(
        &client_options.binary,
        "client-b",
        evidence,
        &client_args(client_options, false, false, false),
        None,
    )?;
    report.fact("client_b_command", client_b.command.join(" "));
    server.wait(
        &["MP_PARTICIPANT_BOUND participant=2", "remote=1"],
        Duration::from_secs(90),
    )?;
    server.wait(
        &["MP_MATCH_ACTIVE", &format!("generation={generation}")],
        Duration::from_secs(30),
    )?;
    client_a.wait(
        &[
            "MP_CLIENT_MATCH_OBSERVED",
            "map=map.test.multiplayer_skirmish",
        ],
        Duration::from_secs(60),
    )?;
    client_a.attach_window()?;
    thread::sleep(Duration::from_secs(6));

    let result = (|| -> Result<(), String> {
        let wait5 = |game: &mut Game, fragments: &[&str], from: usize| {
            game.wait_line(fragments, Duration::from_secs(5), from)
        };
        let owners = entity_owners(&server.text());
        let own: Vec<u64> = owners
            .iter()
            .filter(|(_, o)| *o == 1)
            .map(|(e, _)| *e)
            .collect();
        report.verdict(
            "FORM-FIXTURE-20-PER-SEAT",
            own.len() == GROUP_UNITS_PER_SEAT && field(&fixture, "per_seat") == Some("20"),
            format!("participant 1 units={}; {}", own.len(), fixture.trim()),
        );

        // DISC-01: before any formation every own unit is at the baseline.
        let first = server.wait(
            &["RTS_DISCIPLINE_PARTICIPANT", " participant=1 "],
            Duration::from_secs(10),
        )?;
        report.verdict(
            "DISC-01-BASELINE-AT-START",
            int_field(&first, "units") == Some(20)
                && int_field(&first, "members") == Some(0)
                && int_field(&first, "min") == Some(50_000)
                && int_field(&first, "max") == Some(50_000),
            first.trim().to_owned(),
        );

        let box_all = |client: &mut Game| -> Result<(usize, String), String> {
            let before = client.offset();
            client.drag_hold(
                (24.0, 24.0),
                (f64::from(WIDTH) - 24.0, f64::from(HEIGHT) - 24.0),
            )?;
            client.release()?;
            let line = client.wait_line(
                &["RTS_SELECTION_SET", "source=box", "op=replace"],
                Duration::from_secs(5),
                before,
            )?;
            Ok((before, line))
        };

        // Setup: the lowest-ID clickable unit alone in group 2, the army in group 1.
        let (before_box, _) = box_all(&mut client_a)?;
        let screens: Vec<(u64, f64, f64)> = client_a.text()[before_box..]
            .lines()
            .filter(|l| l.contains("RTS_SELECTION_SCREEN"))
            .filter_map(|l| {
                Some((
                    field(l, "unit")?.parse().ok()?,
                    number(l, "x")?,
                    number(l, "y")?,
                ))
            })
            .filter(|(_, x, y)| {
                *x > 40.0 && *x < f64::from(WIDTH) - 40.0 && *y > 220.0 && *y < 500.0
            })
            .collect();
        let lone = screens
            .iter()
            .filter(|s| {
                screens
                    .iter()
                    .filter(|o| o.0 != s.0)
                    .all(|o| (s.1 - o.1).hypot(s.2 - o.2) >= 14.0)
            })
            .min_by_key(|s| s.0)
            .copied()
            .ok_or("no clickable own unit clear of its neighbours")?;
        let independent = lone.0;
        report.fact(
            "independent_unit",
            format!("{}@{:.0},{:.0}", lone.0, lone.1, lone.2),
        );
        let at = client_a.offset();
        client_a.click(lone.1, lone.2, "1")?;
        wait5(
            &mut client_a,
            &["RTS_SELECTION_SET", "source=click", "count=1 "],
            at,
        )?;
        let at = client_a.offset();
        client_a.modified_key("ctrl", "2")?;
        let group2 = wait5(&mut client_a, &["RTS_CONTROL_GROUP_ASSIGN", "slot=2 "], at)?;
        box_all(&mut client_a)?;
        let at = client_a.offset();
        client_a.modified_key("ctrl", "1")?;
        let group1 = wait5(&mut client_a, &["RTS_CONTROL_GROUP_ASSIGN", "slot=1 "], at)?;
        if selection_ids(&group2) != vec![independent] || selection_ids(&group1) != own {
            return Err(format!(
                "control group setup failed: {} | {}",
                group2.trim(),
                group1.trim()
            ));
        }

        // 1. T forms the army (Line). DISC-02: Form changes no value.
        let at_server = server.offset();
        client_a.key("t")?;
        let formed =
            server.wait_line(&["RTS_FORMATION_FORMED"], Duration::from_secs(5), at_server)?;
        let formation = field(&formed, "formation").unwrap_or("0").to_owned();
        let formed_step = int_field(&formed, "step").unwrap_or(0);

        // 2. Move the formation into its Line.
        let line = wait_screen(&mut client_a, |t| probe_screen(t, 0), "line probe")?;
        let at_server = server.offset();
        client_a.click(line.0, line.1, "3")?;
        server.wait_line(
            &["RTS_FORMATION_MOVE", &format!("formation={formation} ")],
            Duration::from_secs(5),
            at_server,
        )?;
        let (arrived, at_slot, separation, failed) =
            formation_arrival(&server, at_server, &own, &own);
        report.verdict(
            "DISC-SETUP-LINE-ARRIVED",
            arrived == own.len() && at_slot == own.len() && failed == 0,
            format!(
                "{arrived}/20 arrived, {at_slot} at slot, {failed} failures, min separation {separation:.1} cm"
            ),
        );
        let early: Vec<String> = server
            .text()
            .lines()
            .filter(|l| l.contains("RTS_DISCIPLINE_BAND") && l.contains(" participant=1 "))
            .filter(|l| int_field(l, "step").is_some_and(|s| s <= formed_step))
            .map(str::to_owned)
            .collect();
        let gained: Vec<String> = server
            .text()
            .lines()
            .filter(|l| {
                l.contains("RTS_DISCIPLINE_SAMPLE")
                    && l.contains(&format!(" formation={formation} "))
            })
            .map(str::to_owned)
            .collect();
        report.verdict(
            "DISC-02-FORM-CHANGES-NOTHING",
            early.is_empty() && !gained.is_empty(),
            format!(
                "formed at step {formed_step}; band crossings at or before Form {}; first sample {}",
                early.len(),
                gained.first().map_or("none", |l| l.trim())
            ),
        );

        // 3. Members earn: between two consecutive formation samples (5 evaluations) the average
        // rises by exactly 5 points while every member earns below the ceiling.
        let sample_at = server.offset();
        let first_sample = server.wait_line(
            &[
                "RTS_DISCIPLINE_SAMPLE",
                &format!(" formation={formation} "),
                "earning=20 ",
            ],
            Duration::from_secs(15),
            sample_at,
        )?;
        let after_first = server.offset();
        let second_sample = server.wait_line(
            &["RTS_DISCIPLINE_SAMPLE", &format!(" formation={formation} ")],
            Duration::from_secs(10),
            after_first,
        )?;
        let rise = int_field(&second_sample, "avg").unwrap_or(0)
            - int_field(&first_sample, "avg").unwrap_or(0);
        report.verdict(
            "DISC-03-MEMBERS-EARN-ONE-POINT-PER-SECOND",
            int_field(&second_sample, "earning") == Some(20)
                && rise == 5_000
                && int_field(&second_sample, "max").is_some_and(|m| m <= 80_000),
            format!("{} | {}", first_sample.trim(), second_sample.trim()),
        );
        report.artifact("earning_png", &client_a.screenshot(evidence, "01-earning")?);

        // 4. Group 2 + X: an individual Stop makes the lone unit independent; it holds.
        let at = client_a.offset();
        let at_server = server.offset();
        client_a.key("2")?;
        wait5(&mut client_a, &["RTS_SELECTION_SET", "count=1 "], at)?;
        client_a.key("x")?;
        let made = server.wait_line(
            &[
                "RTS_FORMATION_MEMBER_INDEPENDENT",
                &format!(" unit={independent} "),
            ],
            Duration::from_secs(5),
            at_server,
        )?;
        let at_hold = server.offset();
        let mut holds = Vec::new();
        for _ in 0..3 {
            let from = server.offset();
            holds.push(server.wait_line(
                &["RTS_DISCIPLINE_HOLDING", &format!(" unit={independent} ")],
                Duration::from_secs(10),
                from,
            )?);
        }
        let held: Vec<i64> = holds.iter().filter_map(|l| int_field(l, "value")).collect();
        let others_rose = server.text()[at_hold..]
            .lines()
            .filter(|l| {
                l.contains("RTS_DISCIPLINE_SAMPLE")
                    && l.contains(&format!(" formation={formation} "))
            })
            .filter_map(|l| int_field(l, "max"))
            .collect::<Vec<_>>();
        report.verdict(
            "DISC-04-INDEPENDENT-MEMBER-HOLDS",
            held.len() == 3
                && held.windows(2).all(|w| w[0] == w[1])
                && others_rose.windows(2).all(|w| w[1] >= w[0])
                && others_rose.last().copied().unwrap_or(0) > held[0],
            format!(
                "{}; held values {held:?}; formation max over the same samples {others_rose:?}",
                made.trim()
            ),
        );
        report.artifact(
            "independent_png",
            &client_a.screenshot(evidence, "02-independent-holds")?,
        );

        // 5. Group 1 + H (Guard): the probe walks an opposing unit next to the formation, so the
        // opposing client receives the formation's units while it lives.
        let at = client_a.offset();
        let at_server = server.offset();
        let at_b = client_b.offset();
        client_a.key("1")?;
        wait5(&mut client_a, &["RTS_SELECTION_SET", "count=20 "], at)?;
        client_a.key("h")?;
        let probe = server.wait_line(
            &["MP_STANCE_PROBE", "phase=guard"],
            Duration::from_secs(5),
            at_server,
        )?;
        let probe_unit = unit_field(&probe);
        let _ = server.wait_line(
            &["RTS_UNIT_DESTROYED", &format!("unit={probe_unit}")],
            Duration::from_secs(60),
            at_server,
        );

        // 6. DISC-03: every following member reaches 80 and never passes it.
        let deadline = Instant::now() + Duration::from_secs(60);
        let ceiling = loop {
            let text = server.text();
            let reached: Vec<u64> = text
                .lines()
                .filter(|l| {
                    l.contains("RTS_DISCIPLINE_BAND")
                        && l.contains(" band=80 ")
                        && l.contains("direction=rising")
                })
                .map(unit_field)
                .filter(|u| own.contains(u))
                .collect();
            let alive: Vec<u64> = own
                .iter()
                .copied()
                .filter(|u| *u != independent && server_unit(&text, *u).is_some_and(|s| s.2 > 0))
                .collect();
            if alive.iter().all(|u| reached.contains(u)) || Instant::now() >= deadline {
                break (reached, alive);
            }
            thread::sleep(Duration::from_millis(500));
        };
        let after_ceiling = server.offset();
        let next = server.wait_line(
            &["RTS_DISCIPLINE_SAMPLE", &format!(" formation={formation} ")],
            Duration::from_secs(10),
            after_ceiling,
        )?;
        let above = server
            .text()
            .lines()
            .filter(|l| {
                l.contains("RTS_DISCIPLINE_BAND")
                    && (l.contains(" band=90 ") || l.contains(" band=100 "))
            })
            .count();
        report.verdict(
            "DISC-03-CEILING-80",
            ceiling.1.iter().all(|u| ceiling.0.contains(u))
                && !ceiling.1.is_empty()
                && above == 0
                && int_field(&next, "max") == Some(80_000),
            format!(
                "{} of {} following members crossed 80 rising; crossings of 90/100 {above}; next sample {}",
                ceiling.0.len(),
                ceiling.1.len(),
                next.trim()
            ),
        );
        report.artifact(
            "ceiling_png",
            &client_a.screenshot(evidence, "03-ceiling-selected")?,
        );

        // 7. DISC-07: the opposing client received the formation's units but only the baseline.
        let seen: Vec<String> = client_b.text()[at_b..]
            .lines()
            .filter(|l| l.contains("MP_CLIENT_SAMPLE") && field(l, "owner") == Some("1"))
            .map(str::to_owned)
            .collect();
        let leaked = seen
            .iter()
            .filter(|l| int_field(l, "resolve") != Some(50_000))
            .count();
        let own_view = client_a
            .text()
            .lines()
            .filter(|l| {
                l.contains("MP_CLIENT_SAMPLE")
                    && field(l, "owner") == Some("1")
                    && int_field(l, "resolve").is_some_and(|v| v > 50_000)
            })
            .count();
        report.verdict(
            "DISC-07-OWNER-ONLY",
            !seen.is_empty() && leaked == 0 && own_view > 0,
            format!(
                "opposing client samples of participant 1 units {}, with a value other than 50000 {leaked}; owner samples above 50000 {own_view}; {}",
                seen.len(),
                probe.trim()
            ),
        );

        // 8. Y releases the formation: every unit fades 0.5 per second back to 50 and stops there.
        let at = client_a.offset();
        let at_server = server.offset();
        client_a.key("y")?;
        let dissolved = server.wait_line(
            &[
                "RTS_FORMATION_DISSOLVED",
                &format!("formation={formation} "),
            ],
            Duration::from_secs(5),
            at_server,
        )?;
        wait5(&mut client_a, &["RTS_FORMATION_VIEW", "formations=0 "], at)?;
        thread::sleep(Duration::from_secs(3));
        let at = client_a.offset();
        client_a.key("2")?;
        wait5(&mut client_a, &["RTS_SELECTION_SET", "count=1 "], at)?;
        report.artifact(
            "fading_png",
            &client_a.screenshot(evidence, "04-fading-selected-unit")?,
        );
        let deadline = Instant::now() + Duration::from_secs(80);
        let samples = loop {
            let samples = discipline_participant_samples(&server.text()[at_server..]);
            let settled = samples.last().is_some_and(|l| {
                int_field(l, "outside") == Some(20) && int_field(l, "max") == Some(50_000)
            });
            if settled || Instant::now() >= deadline {
                break samples;
            }
            thread::sleep(Duration::from_secs(1));
        };
        let maxima: Vec<i64> = samples.iter().filter_map(|l| int_field(l, "max")).collect();
        let minima: Vec<i64> = samples.iter().filter_map(|l| int_field(l, "min")).collect();
        let faded: Vec<u64> = server.text()[at_server..]
            .lines()
            .filter(|l| {
                l.contains("RTS_DISCIPLINE_BAND")
                    && l.contains(" band=50 ")
                    && l.contains("direction=fading")
            })
            .map(unit_field)
            .filter(|u| own.contains(u))
            .collect();
        let survivors = own
            .iter()
            .filter(|u| server_unit(&server.text(), **u).is_some_and(|s| s.2 > 0))
            .count();
        let settle_step = samples
            .iter()
            .find(|l| int_field(l, "max") == Some(50_000))
            .and_then(|l| int_field(l, "step"));
        let release_step = int_field(&dissolved, "step").unwrap_or(0);
        report.verdict(
            "DISC-06-RELEASE-FADES-TO-BASELINE",
            maxima.last() == Some(&50_000)
                && minima.iter().all(|m| *m >= 50_000)
                && maxima.windows(2).all(|w| w[0] - w[1] <= 2_500 && w[1] <= w[0])
                && faded.len() == survivors,
            format!(
                "{}; sample maxima {maxima:?}; minima never below {:?}; {} of {survivors} units landed on 50 fading; all at 50 by step {settle_step:?} ({} steps after release)",
                dissolved.trim(),
                minima.iter().min(),
                faded.len(),
                settle_step.map_or(-1, |s| s - release_step)
            ),
        );
        report.artifact(
            "baseline_png",
            &client_a.screenshot(evidence, "05-baseline")?,
        );

        let ended = server.text().lines().any(|l| {
            l.contains("MATCH_LIFECYCLE phase=5")
                || l.contains("RTS_OBJECTIVE_STATE") && l.contains("complete=1")
        });
        report.verdict(
            "FORM-MATCH-STAYS-ACTIVE",
            !ended,
            format!("terminal or captured during the session: {ended}"),
        );
        Ok(())
    })();
    launch_options_verdict(
        report,
        "DISC-D1-DEV-LAUNCH-OPTIONS",
        &[&server, &client_a, &client_b],
    );
    report.artifact("server_log", &server.log);
    report.artifact("client_a_log", &client_a.log);
    report.artifact("client_b_log", &client_b.log);
    client_a.stop();
    client_b.stop();
    server.stop();
    drop(display_a);
    result
}

// ---------------------------------------------------------------------------------------
// `--discipline-stress SERVER CLIENT [--evidence DIR] [--port N]`.
//
// DISC-D2 on the dedicated 1v1 fixture (20 idle units per seat). The server runs with the
// stance probe, the discipline casualty fixture and participant 2 on the solo-tolerant
// (Atlantic Concord) doctrine. Seat 1's client, with real X input: sends one lone unit
// (group 2) far behind the army and forms the other 19 (group 1) into a Line. The lone soldier
// fades 0.5 per second to the isolation floor of 30 and holds there. H (Guard) makes the probe
// walk one opposing unit away from its comrades: as a Concord soldier it stays at 50. J sets the
// formation's adherence, which makes the fixture kill an end member and its three nearest
// fellow members on consecutive steps: members within 800 cm lose 5 per casualty, at most 15 in
// the window, members beyond do not. Finally the lone soldier walks back beside its comrades
// and recovers to 50. The opposing client receives only the default value throughout.
// ---------------------------------------------------------------------------------------

/// The lone soldier's destination, far behind the army's Line (ground probe index 2).
const DISC_LONE_POINT: (f64, f64) = (-1700.0, -500.0);

#[allow(clippy::too_many_lines)]
fn discipline_stress_session(
    options: &DedicatedOptions,
    report: &mut Report,
) -> Result<(), String> {
    let client_options = &options.client;
    let evidence = &client_options.evidence;
    let port = client_options.port;
    let mut args = server_args(port, "1v1");
    args.push(format!("-PolitikaMPUnitScale={GROUP_UNITS_PER_SEAT}"));
    args.push("-PolitikaMPUnitScaleHold".into());
    args.push("-PolitikaMPStanceProbe".into());
    args.push("-PolitikaMPDisciplineCasualties".into());
    args.push("-PolitikaMPDisciplineDoctrine=2:solo".into());
    let mut server = Game::launch_server(&options.server, "server", evidence, &args)?;
    report.fact("server_command", server.command.join(" "));
    server.wait(&["MP_DEDICATED_SERVER_READY"], Duration::from_secs(180))?;
    let awaiting = server.wait(&["MP_MATCH_AWAITING_PARTICIPANTS"], Duration::from_secs(60))?;
    let generation = field(&awaiting, "generation").unwrap_or("0").to_owned();
    let fixture = server.wait(&["MP_UNIT_SCALE_FIXTURE"], Duration::from_secs(5))?;

    let display_a = Display::start(evidence, "client-a")?;
    let mut args_a = client_args(client_options, true, false, false);
    args_a.push(format!(
        "-PolitikaMPGroundProbe={}:{}/{}:{}/{}:{}",
        SHAPE_LINE_POINT.0,
        SHAPE_LINE_POINT.1,
        SHAPE_COLUMN_POINT.0,
        SHAPE_COLUMN_POINT.1,
        DISC_LONE_POINT.0,
        DISC_LONE_POINT.1
    ));
    let mut client_a = Game::launch(
        &client_options.binary,
        "client-a",
        evidence,
        &args_a,
        Some(&display_a.name()),
    )?;
    report.fact("client_a_command", client_a.command.join(" "));
    server.wait(
        &["MP_PARTICIPANT_BOUND participant=1", "remote=1"],
        Duration::from_secs(90),
    )?;
    let mut client_b = Game::launch(
        &client_options.binary,
        "client-b",
        evidence,
        &client_args(client_options, false, false, false),
        None,
    )?;
    report.fact("client_b_command", client_b.command.join(" "));
    server.wait(
        &["MP_PARTICIPANT_BOUND participant=2", "remote=1"],
        Duration::from_secs(90),
    )?;
    server.wait(
        &["MP_MATCH_ACTIVE", &format!("generation={generation}")],
        Duration::from_secs(30),
    )?;
    client_a.wait(
        &[
            "MP_CLIENT_MATCH_OBSERVED",
            "map=map.test.multiplayer_skirmish",
        ],
        Duration::from_secs(60),
    )?;
    client_a.attach_window()?;
    thread::sleep(Duration::from_secs(6));

    let result = (|| -> Result<(), String> {
        let wait5 = |game: &mut Game, fragments: &[&str], from: usize| {
            game.wait_line(fragments, Duration::from_secs(5), from)
        };
        let owners = entity_owners(&server.text());
        let own: Vec<u64> = owners
            .iter()
            .filter(|(_, o)| *o == 1)
            .map(|(e, _)| *e)
            .collect();
        report.verdict(
            "FORM-FIXTURE-20-PER-SEAT",
            own.len() == GROUP_UNITS_PER_SEAT && field(&fixture, "per_seat") == Some("20"),
            format!("participant 1 units={}; {}", own.len(), fixture.trim()),
        );
        let first = server.wait(
            &["RTS_DISCIPLINE_PARTICIPANT", " participant=1 "],
            Duration::from_secs(10),
        )?;
        report.verdict(
            "DISC-01-BASELINE-AT-START",
            int_field(&first, "units") == Some(20)
                && int_field(&first, "min") == Some(50_000)
                && int_field(&first, "max") == Some(50_000),
            first.trim().to_owned(),
        );

        let box_all = |client: &mut Game| -> Result<(usize, String), String> {
            let before = client.offset();
            client.drag_hold(
                (24.0, 24.0),
                (f64::from(WIDTH) - 24.0, f64::from(HEIGHT) - 24.0),
            )?;
            client.release()?;
            let line = client.wait_line(
                &["RTS_SELECTION_SET", "source=box", "op=replace"],
                Duration::from_secs(5),
                before,
            )?;
            Ok((before, line))
        };

        // Setup: the lowest-ID clickable unit alone in group 2, the other 19 in group 1.
        let (before_box, _) = box_all(&mut client_a)?;
        let screens: Vec<(u64, f64, f64)> = client_a.text()[before_box..]
            .lines()
            .filter(|l| l.contains("RTS_SELECTION_SCREEN"))
            .filter_map(|l| {
                Some((
                    field(l, "unit")?.parse().ok()?,
                    number(l, "x")?,
                    number(l, "y")?,
                ))
            })
            .filter(|(_, x, y)| {
                *x > 40.0 && *x < f64::from(WIDTH) - 40.0 && *y > 220.0 && *y < 500.0
            })
            .collect();
        let lone = screens
            .iter()
            .filter(|s| {
                screens
                    .iter()
                    .filter(|o| o.0 != s.0)
                    .all(|o| (s.1 - o.1).hypot(s.2 - o.2) >= 14.0)
            })
            .min_by_key(|s| s.0)
            .copied()
            .ok_or("no clickable own unit clear of its neighbours")?;
        let lone_id = lone.0;
        report.fact(
            "lone_unit",
            format!("{}@{:.0},{:.0}", lone.0, lone.1, lone.2),
        );
        let at = client_a.offset();
        client_a.click(lone.1, lone.2, "1")?;
        wait5(
            &mut client_a,
            &["RTS_SELECTION_SET", "source=click", "count=1 "],
            at,
        )?;
        let at = client_a.offset();
        client_a.modified_key("ctrl", "2")?;
        let group2 = wait5(&mut client_a, &["RTS_CONTROL_GROUP_ASSIGN", "slot=2 "], at)?;
        box_all(&mut client_a)?;
        let at = client_a.offset();
        client_a.modified_click(lone.1, lone.2, "shift")?;
        wait5(
            &mut client_a,
            &["RTS_SELECTION_SET", "source=click", "op=toggle"],
            at,
        )?;
        let at = client_a.offset();
        client_a.modified_key("ctrl", "1")?;
        let group1 = wait5(&mut client_a, &["RTS_CONTROL_GROUP_ASSIGN", "slot=1 "], at)?;
        let army: Vec<u64> = own.iter().copied().filter(|u| *u != lone_id).collect();
        if selection_ids(&group2) != vec![lone_id] || selection_ids(&group1) != army {
            return Err(format!(
                "control group setup failed: {} | {}",
                group2.trim(),
                group1.trim()
            ));
        }

        // 1. Group 2: send the lone soldier far behind where the Line will stand.
        let lone_point = wait_screen(&mut client_a, |t| probe_screen(t, 2), "lone point")?;
        let at = client_a.offset();
        client_a.key("2")?;
        wait5(&mut client_a, &["RTS_SELECTION_SET", "count=1 "], at)?;
        let lone_sent = server.offset();
        client_a.click(lone_point.0, lone_point.1, "3")?;

        // 2. Group 1: T forms the 19 and a click moves them into a Line.
        let at = client_a.offset();
        client_a.key("1")?;
        wait5(&mut client_a, &["RTS_SELECTION_SET", "count=19 "], at)?;
        let at_server = server.offset();
        client_a.key("t")?;
        let formed =
            server.wait_line(&["RTS_FORMATION_FORMED"], Duration::from_secs(5), at_server)?;
        let formation = field(&formed, "formation").unwrap_or("0").to_owned();
        let line = wait_screen(&mut client_a, |t| probe_screen(t, 0), "line probe")?;
        let at_server = server.offset();
        client_a.click(line.0, line.1, "3")?;
        server.wait_line(
            &["RTS_FORMATION_MOVE", &format!("formation={formation} ")],
            Duration::from_secs(5),
            at_server,
        )?;
        let (arrived, at_slot, separation, failed) =
            formation_arrival(&server, at_server, &army, &army);
        report.verdict(
            "DISC-SETUP-LINE-ARRIVED",
            arrived == army.len() && at_slot == army.len() && failed == 0,
            format!(
                "{arrived}/19 arrived, {at_slot} at slot, {failed} failures, min separation {separation:.1} cm"
            ),
        );

        // 3. DISC-11: the lone soldier fades 2.5 points per 100-step sample to 30 and holds.
        let lone_tag = format!(" unit={lone_id} ");
        let isolated_samples = |text: &str| -> Vec<(i64, i64, String)> {
            text.lines()
                .filter(|l| l.contains("RTS_DISCIPLINE_ISOLATED") && l.contains(&lone_tag))
                .filter_map(|l| Some((int_field(l, "step")?, int_field(l, "value")?, l.to_owned())))
                .collect()
        };
        let deadline = Instant::now() + Duration::from_secs(90);
        let lone_series = loop {
            let series = isolated_samples(&server.text()[lone_sent..]);
            if series.iter().filter(|s| s.1 == 30_000).count() >= 2 || Instant::now() >= deadline {
                break series;
            }
            thread::sleep(Duration::from_secs(1));
        };
        let values: Vec<i64> = lone_series.iter().map(|s| s.1).collect();
        let steady_fade = lone_series.windows(2).all(|w| {
            w[1].0 - w[0].0 == 100
                && (w[0].1 - w[1].1 == 2_500
                    || (w[0].1 > 30_000 && w[1].1 == 30_000)
                    || (w[0].1 == 30_000 && w[1].1 == 30_000))
        });
        let standard = lone_series.iter().all(|s| {
            field(&s.2, "exempt") == Some("0") && field(&s.2, "doctrine") == Some("standard")
        });
        report.verdict(
            "DISC-11-LONE-SOLDIER-FADES-TO-30",
            values.len() >= 3
                && steady_fade
                && standard
                && values.iter().all(|v| *v >= 30_000)
                && values.iter().filter(|v| **v == 30_000).count() >= 2
                && values.first().is_some_and(|v| *v < 50_000),
            format!(
                "unit {lone_id}: samples (step, value) {:?}",
                lone_series.iter().map(|s| (s.0, s.1)).collect::<Vec<_>>()
            ),
        );
        let at = client_a.offset();
        client_a.key("2")?;
        wait5(&mut client_a, &["RTS_SELECTION_SET", "count=1 "], at)?;
        thread::sleep(Duration::from_millis(600));
        report.artifact(
            "lone_png",
            &client_a.screenshot(evidence, "01-lone-soldier-at-30")?,
        );

        // 4. Group 1 + H (Guard): the probe walks one opposing unit away from its comrades. With
        // the solo-tolerant doctrine it is listed as alone but exempt, and stays at 50.
        let at = client_a.offset();
        let at_server = server.offset();
        let at_b = client_b.offset();
        client_a.key("1")?;
        wait5(&mut client_a, &["RTS_SELECTION_SET", "count=19 "], at)?;
        client_a.key("h")?;
        let probe = server.wait_line(
            &["MP_STANCE_PROBE", "phase=guard"],
            Duration::from_secs(5),
            at_server,
        )?;
        let probe_unit = unit_field(&probe);
        let _ = server.wait_line(
            &["RTS_UNIT_DESTROYED", &format!("unit={probe_unit}")],
            Duration::from_secs(60),
            at_server,
        );
        let exempt: Vec<String> = server.text()[at_server..]
            .lines()
            .filter(|l| {
                l.contains("RTS_DISCIPLINE_ISOLATED") && l.contains(&format!(" unit={probe_unit} "))
            })
            .map(str::to_owned)
            .collect();
        let opponent_min = discipline_participant_samples_of(&server.text(), 2)
            .iter()
            .filter_map(|l| int_field(l, "min"))
            .min();
        let override_line = server
            .text()
            .lines()
            .find(|l| l.contains("RTS_DISCIPLINE_DOCTRINE_OVERRIDE"))
            .unwrap_or("")
            .trim()
            .to_owned();
        report.verdict(
            "DISC-11-CONCORD-EXEMPT",
            !exempt.is_empty()
                && exempt.iter().all(|l| {
                    field(l, "doctrine") == Some("solo")
                        && field(l, "exempt") == Some("1")
                        && int_field(l, "value") == Some(50_000)
                })
                && opponent_min == Some(50_000)
                && field(&override_line, "participant") == Some("2"),
            format!(
                "{override_line}; probe unit {probe_unit} alone in {} samples: {}; participant 2 minimum over the run {opponent_min:?}",
                exempt.len(),
                exempt.first().map_or("none", |l| l.trim())
            ),
        );
        // Guard chasers walk back to their posts before the casualties.
        thread::sleep(Duration::from_secs(8));

        // 5. J: the fixture kills an end member and its three nearest fellow members.
        let at_server = server.offset();
        client_a.key("j")?;
        let armed = server.wait_line(
            &[
                "MP_DISCIPLINE_CASUALTIES_ARMED",
                &format!("formation={formation} "),
            ],
            Duration::from_secs(5),
            at_server,
        )?;
        let victims: Vec<u64> = field(&armed, "victims")
            .unwrap_or("")
            .split(',')
            .filter_map(|v| v.parse().ok())
            .collect();
        let destroyed_since = |text: &str| -> Vec<u64> {
            text.lines()
                .filter(|l| l.contains("RTS_UNIT_DESTROYED"))
                .map(unit_field)
                .collect()
        };
        let deadline = Instant::now() + Duration::from_secs(10);
        while !victims
            .iter()
            .all(|v| destroyed_since(&server.text()[at_server..]).contains(v))
        {
            if Instant::now() >= deadline {
                return Err(format!("not every casualty fell: {}", armed.trim()));
            }
            thread::sleep(Duration::from_millis(200));
        }
        let fallen_before = destroyed_since(&server.text()[..at_server]);
        thread::sleep(Duration::from_millis(500));
        let text = server.text()[at_server..].to_owned();
        let destroyed_steps: Vec<(u64, i64)> = text
            .lines()
            .filter(|l| l.contains("MP_DISCIPLINE_CASUALTY "))
            .filter_map(|l| Some((unit_field(l), int_field(l, "step")?)))
            .collect();
        let shocks: Vec<String> = text
            .lines()
            .filter(|l| l.contains("RTS_DISCIPLINE_SHOCK"))
            .filter(|l| {
                field(l, "casualty")
                    .and_then(|v| v.parse::<u64>().ok())
                    .is_some_and(|c| victims.contains(&c))
            })
            .map(str::to_owned)
            .collect();
        let in_range: Vec<&String> = shocks
            .iter()
            .filter(|l| field(l, "in_range") == Some("1"))
            .collect();
        let out_range: Vec<&String> = shocks
            .iter()
            .filter(|l| field(l, "in_range") == Some("0"))
            .collect();
        let range_ok = in_range
            .iter()
            .all(|l| number(l, "distance").is_some_and(|d| d <= 800.0))
            && out_range.iter().all(|l| {
                number(l, "distance").is_some_and(|d| d > 800.0)
                    && int_field(l, "applied") == Some(0)
                    && int_field(l, "before") == int_field(l, "after")
            });
        // Every surviving member is listed once per casualty: 19 members, minus the casualties so far.
        let mut per_casualty = Vec::new();
        let mut listed_ok = victims.len() == 4;
        for (index, victim) in victims.iter().enumerate() {
            let listed: Vec<u64> = shocks
                .iter()
                .filter(|l| field(l, "casualty") == Some(&victim.to_string()))
                .map(|l| unit_field(l))
                .collect();
            let expected: Vec<u64> = army
                .iter()
                .copied()
                .filter(|u| !victims[..=index].contains(u) && !fallen_before.contains(u))
                .collect();
            let mut sorted = listed.clone();
            sorted.sort_unstable();
            listed_ok &= sorted == expected;
            per_casualty.push(listed.len());
        }
        report.verdict(
            "DISC-10-SHOCK-REACHES-ONLY-MEMBERS-IN-RANGE",
            range_ok && listed_ok && !in_range.is_empty() && !out_range.is_empty(),
            format!(
                "{armed}; casualties (unit, step) {destroyed_steps:?}; members listed per casualty {per_casualty:?}; in range {} (max {:.1} cm), out of range {} (min {:.1} cm)",
                in_range.len(),
                in_range.iter().filter_map(|l| number(l, "distance")).fold(0.0, f64::max),
                out_range.len(),
                out_range
                    .iter()
                    .filter_map(|l| number(l, "distance"))
                    .fold(f64::INFINITY, f64::min),
                armed = armed.trim()
            ),
        );
        // Per member: what each casualty in range cost it, in order.
        let mut costs: Vec<(u64, Vec<i64>)> = Vec::new();
        let mut amounts_ok = true;
        for line in &in_range {
            let unit = unit_field(line);
            let applied = int_field(line, "applied").unwrap_or(-1);
            amounts_ok &= (0..=5_000).contains(&applied)
                && int_field(line, "after")
                    == int_field(line, "before").map(|b| (b - applied).max(0));
            match costs.iter_mut().find(|c| c.0 == unit) {
                Some(entry) => entry.1.push(applied),
                None => costs.push((unit, vec![applied])),
            }
        }
        let capped: Vec<&(u64, Vec<i64>)> = costs
            .iter()
            .filter(|c| c.1 == vec![5_000, 5_000, 5_000, 0])
            .collect();
        let within_cap = costs.iter().all(|c| c.1.iter().sum::<i64>() <= 15_000);
        let full_until_cap = costs.iter().all(|c| {
            let mut taken = 0;
            c.1.iter().all(|a| {
                let expected = (15_000 - taken).clamp(0, 5_000);
                taken += expected;
                *a == expected
            })
        });
        report.verdict(
            "DISC-10-SHOCK-5-CAPPED-AT-15",
            amounts_ok && within_cap && full_until_cap && !capped.is_empty(),
            format!(
                "per member costs {costs:?}; members that hit the cap on the fourth casualty {}",
                capped.len()
            ),
        );
        report.artifact(
            "shocked_png",
            &client_a.screenshot(evidence, "02-formation-after-casualties")?,
        );

        // 6. Group 2: the lone soldier walks back to the Line and recovers to 50 beside it.
        let at = client_a.offset();
        client_a.key("2")?;
        wait5(&mut client_a, &["RTS_SELECTION_SET", "count=1 "], at)?;
        let back = wait_screen(&mut client_a, |t| probe_screen(t, 0), "line probe")?;
        let at_server = server.offset();
        let last_isolated = isolated_samples(&server.text()).last().map(|s| s.0);
        client_a.click(back.0, back.1, "3")?;
        let landed = server.wait_line(
            &[
                "RTS_DISCIPLINE_BAND",
                &lone_tag,
                "band=50 ",
                "direction=rising",
            ],
            Duration::from_secs(75),
            at_server,
        );
        thread::sleep(Duration::from_secs(1));
        let after_back = isolated_samples(&server.text()[at_server..]);
        let recovered = client_a
            .text()
            .lines()
            .filter(|l| {
                l.contains("MP_CLIENT_SAMPLE") && l.contains(&format!(" entity={lone_id} "))
            })
            .filter_map(|l| int_field(l, "resolve"))
            .collect::<Vec<_>>();
        let tail: Vec<i64> = recovered
            .iter()
            .copied()
            .skip_while(|v| *v != 30_000)
            .collect();
        let landed_step = landed.as_ref().ok().and_then(|l| int_field(l, "step"));
        report.verdict(
            "DISC-11-RECOVERS-BESIDE-COMRADES",
            landed_step.is_some()
                && tail.windows(2).all(|w| w[1] >= w[0])
                && tail.last() == Some(&50_000)
                && after_back.len() <= 3,
            format!(
                "{}; last isolated sample at step {last_isolated:?}; isolated samples after the order {} (walking back); owner client values from 30 on, non-decreasing to {:?}",
                landed.as_ref().map_or_else(|e| e.clone(), |l| l.trim().to_owned()),
                after_back.len(),
                tail.last()
            ),
        );
        report.artifact(
            "recovered_png",
            &client_a.screenshot(evidence, "03-lone-soldier-recovered")?,
        );

        // 7. DISC-07: the opposing client received participant 1's units with the default only,
        // although several of them were below 50 on the server.
        let seen: Vec<String> = client_b.text()[at_b..]
            .lines()
            .filter(|l| l.contains("MP_CLIENT_SAMPLE") && field(l, "owner") == Some("1"))
            .map(str::to_owned)
            .collect();
        let leaked = seen
            .iter()
            .filter(|l| int_field(l, "resolve") != Some(50_000))
            .count();
        let own_below = client_a
            .text()
            .lines()
            .filter(|l| {
                l.contains("MP_CLIENT_SAMPLE")
                    && field(l, "owner") == Some("1")
                    && int_field(l, "resolve").is_some_and(|v| v < 50_000)
            })
            .count();
        report.verdict(
            "DISC-07-OWNER-ONLY",
            !seen.is_empty() && leaked == 0 && own_below > 0,
            format!(
                "opposing client samples of participant 1 units {}, with a value other than 50000 {leaked}; owner samples below 50000 {own_below}",
                seen.len()
            ),
        );
        let ended = server.text().lines().any(|l| {
            l.contains("MATCH_LIFECYCLE phase=5")
                || l.contains("RTS_OBJECTIVE_STATE") && l.contains("complete=1")
        });
        report.verdict(
            "FORM-MATCH-STAYS-ACTIVE",
            !ended,
            format!("terminal or captured during the session: {ended}"),
        );
        Ok(())
    })();
    launch_options_verdict(
        report,
        "DISC-D2-DEV-LAUNCH-OPTIONS",
        &[&server, &client_a, &client_b],
    );
    report.artifact("server_log", &server.log);
    report.artifact("client_a_log", &client_a.log);
    report.artifact("client_b_log", &client_b.log);
    client_a.stop();
    client_b.stop();
    server.stop();
    drop(display_a);
    result
}

// `--discipline-effects SERVER CLIENT [--evidence DIR] [--port N]`.
//
// DISC-D3 on the dedicated 1v1 fixture (20 idle units per seat). The server runs with the stance
// probe, the discipline casualty fixture and participant 2 on the solo-tolerant (Atlantic Concord)
// doctrine, so every opposing soldier stays at the baseline and shows unmodified numbers. Seat 1's
// client, with real X input, forms its 20 units into a Line and lets them earn Discipline 80. H
// (Guard) makes the probe walk one opposing unit close: the members fire at it on a shorter
// interval, take less damage from its hits, chase it on their leash and walk back to their posts
// faster. J sets the adherence, which makes the fixture kill four members: the formation closes its
// ranks after a delay shortened by the survivors' Discipline. R reloads the members that fired,
// faster than the baseline. Every effect is checked against the documented formula on the server
// log and against the opposing side's baseline behaviour.
// ---------------------------------------------------------------------------------------

/// The DISC-D3 multiplier in permille for Discipline `value` (milli-points):
/// 1000 + F * numerator / denominator with F = (D - 50) / 50 in permille, truncating as the server does.
fn discipline_effect_permille(value: i64, numerator: i64, denominator: i64) -> i64 {
    let factor = (value.clamp(0, 100_000) - 50_000) * 1000 / 50_000;
    1000 + factor * numerator / denominator
}

/// `RTS_DISCIPLINE_EFFECT` lines of one effect in `text`.
fn discipline_effects_of(text: &str, effect: &str) -> Vec<String> {
    let tag = format!(" effect={effect} ");
    text.lines()
        .filter(|l| l.contains("RTS_DISCIPLINE_EFFECT") && l.contains(&tag))
        .map(str::to_owned)
        .collect()
}

#[allow(clippy::too_many_lines)]
fn discipline_effects_session(
    options: &DedicatedOptions,
    report: &mut Report,
) -> Result<(), String> {
    let client_options = &options.client;
    let evidence = &client_options.evidence;
    let port = client_options.port;
    let mut args = server_args(port, "1v1");
    args.push(format!("-PolitikaMPUnitScale={GROUP_UNITS_PER_SEAT}"));
    args.push("-PolitikaMPUnitScaleHold".into());
    args.push("-PolitikaMPStanceProbe".into());
    args.push("-PolitikaMPDisciplineCasualties".into());
    args.push("-PolitikaMPDisciplineDoctrine=2:solo".into());
    let mut server = Game::launch_server(&options.server, "server", evidence, &args)?;
    report.fact("server_command", server.command.join(" "));
    server.wait(&["MP_DEDICATED_SERVER_READY"], Duration::from_secs(180))?;
    let awaiting = server.wait(&["MP_MATCH_AWAITING_PARTICIPANTS"], Duration::from_secs(60))?;
    let generation = field(&awaiting, "generation").unwrap_or("0").to_owned();
    let fixture = server.wait(&["MP_UNIT_SCALE_FIXTURE"], Duration::from_secs(5))?;

    let display_a = Display::start(evidence, "client-a")?;
    let mut args_a = client_args(client_options, true, false, false);
    args_a.push(format!(
        "-PolitikaMPGroundProbe={}:{}/{}:{}/{}:{}",
        SHAPE_LINE_POINT.0,
        SHAPE_LINE_POINT.1,
        SHAPE_COLUMN_POINT.0,
        SHAPE_COLUMN_POINT.1,
        DISC_LONE_POINT.0,
        DISC_LONE_POINT.1
    ));
    let mut client_a = Game::launch(
        &client_options.binary,
        "client-a",
        evidence,
        &args_a,
        Some(&display_a.name()),
    )?;
    report.fact("client_a_command", client_a.command.join(" "));
    server.wait(
        &["MP_PARTICIPANT_BOUND participant=1", "remote=1"],
        Duration::from_secs(90),
    )?;
    let mut client_b = Game::launch(
        &client_options.binary,
        "client-b",
        evidence,
        &client_args(client_options, false, false, false),
        None,
    )?;
    report.fact("client_b_command", client_b.command.join(" "));
    server.wait(
        &["MP_PARTICIPANT_BOUND participant=2", "remote=1"],
        Duration::from_secs(90),
    )?;
    server.wait(
        &["MP_MATCH_ACTIVE", &format!("generation={generation}")],
        Duration::from_secs(30),
    )?;
    client_a.wait(
        &[
            "MP_CLIENT_MATCH_OBSERVED",
            "map=map.test.multiplayer_skirmish",
        ],
        Duration::from_secs(60),
    )?;
    client_a.attach_window()?;
    thread::sleep(Duration::from_secs(6));

    let result = (|| -> Result<(), String> {
        let wait5 = |game: &mut Game, fragments: &[&str], from: usize| {
            game.wait_line(fragments, Duration::from_secs(5), from)
        };
        let owners = entity_owners(&server.text());
        let own: Vec<u64> = owners
            .iter()
            .filter(|(_, o)| *o == 1)
            .map(|(e, _)| *e)
            .collect();
        report.verdict(
            "FORM-FIXTURE-20-PER-SEAT",
            own.len() == GROUP_UNITS_PER_SEAT && field(&fixture, "per_seat") == Some("20"),
            format!("participant 1 units={}; {}", own.len(), fixture.trim()),
        );
        let at_b = client_b.offset();

        // Setup: all 20 in group 1, T forms them and a click moves them into a Line.
        let before = client_a.offset();
        client_a.drag_hold(
            (24.0, 24.0),
            (f64::from(WIDTH) - 24.0, f64::from(HEIGHT) - 24.0),
        )?;
        client_a.release()?;
        client_a.wait_line(
            &["RTS_SELECTION_SET", "source=box", "op=replace"],
            Duration::from_secs(5),
            before,
        )?;
        let at = client_a.offset();
        client_a.modified_key("ctrl", "1")?;
        let group1 = wait5(&mut client_a, &["RTS_CONTROL_GROUP_ASSIGN", "slot=1 "], at)?;
        if selection_ids(&group1) != own {
            return Err(format!("control group setup failed: {}", group1.trim()));
        }
        let at_server = server.offset();
        client_a.key("t")?;
        let formed =
            server.wait_line(&["RTS_FORMATION_FORMED"], Duration::from_secs(5), at_server)?;
        let formation = field(&formed, "formation").unwrap_or("0").to_owned();
        let formation_tag = format!(" formation={formation} ");
        let line = wait_screen(&mut client_a, |t| probe_screen(t, 0), "line probe")?;
        let at_server = server.offset();
        client_a.click(line.0, line.1, "3")?;
        server.wait_line(
            &["RTS_FORMATION_MOVE", &formation_tag],
            Duration::from_secs(5),
            at_server,
        )?;
        let (arrived, at_slot, separation, failed) =
            formation_arrival(&server, at_server, &own, &own);
        report.verdict(
            "DISC-SETUP-LINE-ARRIVED",
            arrived == own.len() && at_slot == own.len() && failed == 0,
            format!(
                "{arrived}/20 arrived, {at_slot} at slot, {failed} failures, min separation {separation:.1} cm"
            ),
        );

        // The Line earns Discipline 80 (30 evaluations of service).
        let earned = server.wait_line(
            &["RTS_DISCIPLINE_SAMPLE", &formation_tag, "min=80000 "],
            Duration::from_secs(90),
            at_server,
        );
        report.verdict(
            "DISC-D3-SETUP-EARNED-80",
            earned.as_ref().is_ok_and(|l| {
                int_field(l, "members") == Some(20) && int_field(l, "max") == Some(80_000)
            }),
            earned
                .as_ref()
                .map_or_else(|e| e.clone(), |l| l.trim().to_owned()),
        );
        earned?;
        let at = client_a.offset();
        client_a.key("1")?;
        wait5(&mut client_a, &["RTS_SELECTION_SET", "count=20 "], at)?;
        thread::sleep(Duration::from_millis(600));
        report.artifact(
            "earned_png",
            &client_a.screenshot(evidence, "01-formation-at-80")?,
        );

        // 1. H (Guard): the probe walks one opposing unit close; members engage it, chase it on
        // their leash, kill it and walk back to their posts.
        let probe_from = server.offset();
        client_a.key("h")?;
        let probe = server.wait_line(
            &["MP_STANCE_PROBE", "phase=guard", "placed=1"],
            Duration::from_secs(5),
            probe_from,
        )?;
        let probe_unit = unit_field(&probe);
        server.wait_line(
            &["RTS_UNIT_DESTROYED", &format!("unit={probe_unit}")],
            Duration::from_secs(60),
            probe_from,
        )?;
        // Wait until every chaser that started back has arrived (or given up).
        let deadline = Instant::now() + Duration::from_secs(20);
        loop {
            let text = server.text()[probe_from..].to_owned();
            let started = text
                .lines()
                .filter(|l| l.contains("RTS_STANCE_RETURN") && l.contains("phase=start"))
                .count();
            let ended = text
                .lines()
                .filter(|l| {
                    l.contains("RTS_STANCE_RETURN")
                        && (l.contains("phase=arrived") || l.contains("phase=failed"))
                })
                .count();
            if (started > 0 && ended >= started) || Instant::now() >= deadline {
                break;
            }
            thread::sleep(Duration::from_millis(500));
        }
        thread::sleep(Duration::from_secs(1));
        let probe_text = server.text()[probe_from..].to_owned();
        report.artifact(
            "engaged_png",
            &client_a.screenshot(evidence, "02-after-probe")?,
        );

        // DISC-D3 defense: every hit on a member is scaled once by 1000 + F * (-1/5); a member's
        // health loss is the sum of its scaled hits (the fraction is carried). The probe, at the
        // baseline, loses whole base hits.
        let hits: Vec<String> = discipline_effects_of(&probe_text, "damage_taken")
            .into_iter()
            .filter(|l| field(l, "participant") == Some("1"))
            .collect();
        let formula_ok = hits.iter().all(|l| {
            let value = int_field(l, "value").unwrap_or(-1);
            let permille = int_field(l, "permille").unwrap_or(-1);
            let base = int_field(l, "base_milli").unwrap_or(-1);
            permille == discipline_effect_permille(value, -1, 5)
                && int_field(l, "applied_milli") == Some(base * permille / 1000)
        });
        let mut per_member: Vec<(u64, i64, i64)> = Vec::new();
        for line in &hits {
            let unit = unit_field(line);
            let applied = int_field(line, "applied_milli").unwrap_or(0);
            match per_member.iter_mut().find(|m| m.0 == unit) {
                Some(entry) => entry.1 += applied,
                None => per_member.push((unit, applied, 0)),
            }
        }
        for member in &mut per_member {
            member.2 = probe_text
                .lines()
                .filter(|l| {
                    l.contains("RTS_HEALTH_CHANGED") && l.contains(&format!(" unit={} ", member.0))
                })
                .filter_map(|l| Some(int_field(l, "before")? - int_field(l, "after")?))
                .sum();
        }
        let health_ok = per_member.iter().all(|m| m.2 == m.1 / 1000);
        let base_hit = hits.first().and_then(|l| int_field(l, "base_milli"));
        let probe_losses: Vec<i64> = probe_text
            .lines()
            .filter(|l| {
                l.contains("RTS_HEALTH_CHANGED") && l.contains(&format!(" unit={probe_unit} "))
            })
            .filter_map(|l| Some(int_field(l, "before")? - int_field(l, "after")?))
            .collect();
        let member_base = discipline_effects_of(&probe_text, "fire_interval")
            .first()
            .and_then(|l| int_field(l, "base_steps"));
        let shooters: Vec<u64> = {
            let mut ids: Vec<u64> = hits
                .iter()
                .filter_map(|l| field(l, "shooter")?.parse().ok())
                .collect();
            ids.sort_unstable();
            ids.dedup();
            ids
        };
        let probe_whole = !probe_losses.is_empty()
            && !probe_text.lines().any(|l| {
                l.contains("RTS_DISCIPLINE_EFFECT") && l.contains(&format!(" unit={probe_unit} "))
            });
        let near_80 = hits
            .iter()
            .filter(|l| int_field(l, "value").is_some_and(|v| v >= 79_000))
            .count();
        report.verdict(
            "DISC-D3-DEFENSE-SCALES-HITS",
            !hits.is_empty() && formula_ok && health_ok && near_80 > 0 && probe_whole,
            format!(
                "{} hits on members from {shooters:?} (probe unit {probe_unit}), {near_80} at a value of 79 or more; first {}; per member (unit, applied milli, health lost) {per_member:?}; the probe at the baseline lost {probe_losses:?} (base hit {base_hit:?} milli, unscaled)",
                hits.len(),
                hits.first().map_or("none", |l| l.trim())
            ),
        );

        // DISC-D3 fire interval: the weapon's whole-step interval scaled by 1000 + F * (-3/20),
        // with the fraction carried shot to shot; no member fires before its interval is over.
        let shots: Vec<String> = discipline_effects_of(&probe_text, "fire_interval")
            .into_iter()
            .filter(|l| field(l, "participant") == Some("1"))
            .collect();
        let mut chain_ok = true;
        let mut never_early = true;
        let mut short_gaps = 0;
        // Per member: (step, scaled steps, carry) of each shot.
        type Shots = Vec<(i64, i64, i64)>;
        let mut by_unit: Vec<(u64, Shots)> = Vec::new();
        for line in &shots {
            let unit = unit_field(line);
            let step = int_field(line, "step").unwrap_or(-1);
            let steps = int_field(line, "steps").unwrap_or(-1);
            let carry = int_field(line, "carry").unwrap_or(-1);
            let base = int_field(line, "base_steps").unwrap_or(-1);
            let value = int_field(line, "value").unwrap_or(-1);
            let permille = int_field(line, "permille").unwrap_or(-1);
            let entry = match by_unit.iter_mut().position(|u| u.0 == unit) {
                Some(index) => &mut by_unit[index],
                None => {
                    by_unit.push((unit, Vec::new()));
                    by_unit.last_mut().ok_or("unit list")?
                }
            };
            let previous = entry.1.last().copied();
            let owed = previous.map_or(0, |p| p.2);
            chain_ok &= permille == discipline_effect_permille(value, -3, 20)
                && steps * 1000 + carry == base * permille + owed
                && (0..1000).contains(&carry);
            if let Some((previous_step, previous_steps, _)) = previous {
                never_early &= step - previous_step >= previous_steps;
                if step - previous_step == previous_steps && previous_steps < base {
                    short_gaps += 1;
                }
            }
            entry.1.push((step, steps, carry));
        }
        // The probe's own cadence at the baseline, from the steps of its hits on members.
        let mut probe_steps: Vec<i64> = hits.iter().filter_map(|l| int_field(l, "step")).collect();
        probe_steps.dedup();
        let probe_gaps: Vec<i64> = probe_steps.windows(2).map(|w| w[1] - w[0]).collect();
        let shortest = shots.iter().filter_map(|l| int_field(l, "steps")).min();
        report.verdict(
            "DISC-D3-FIRE-INTERVAL-SHORTER",
            !shots.is_empty() && chain_ok && never_early && short_gaps > 0,
            format!(
                "{} member shots by {} members; member weapon base interval {member_base:?} steps, shortest scaled interval {shortest:?}; {short_gaps} consecutive shots exactly one scaled interval apart below the base; the probe at the baseline fired at gaps {probe_gaps:?}; first {}",
                shots.len(),
                by_unit.len(),
                shots.first().map_or("none", |l| l.trim())
            ),
        );

        // DISC-D3 return speed: 1000 + F / 3; measured from the logged start and arrival
        // (the unit moves on every step between them except the arrival step).
        let returns: Vec<String> = discipline_effects_of(&probe_text, "return_speed");
        let mut measured: Vec<(u64, f64, f64, f64)> = Vec::new();
        let mut return_ok = !returns.is_empty();
        for line in &returns {
            let unit = unit_field(line);
            let value = int_field(line, "value").unwrap_or(-1);
            let permille = int_field(line, "permille").unwrap_or(-1);
            let base = number(line, "base_speed").unwrap_or(0.0);
            let speed = number(line, "speed").unwrap_or(0.0);
            return_ok &= permille == discipline_effect_permille(value, 1, 3)
                && (speed - base * permille as f64 / 1000.0).abs() <= 0.2;
            let start_step = int_field(line, "step").unwrap_or(-1);
            let tag = format!(" unit={unit} ");
            let start = probe_text.lines().find(|l| {
                l.contains("RTS_STANCE_RETURN")
                    && l.contains(&tag)
                    && l.contains("phase=start")
                    && int_field(l, "step") == Some(start_step)
            });
            let arrival = probe_text.lines().find(|l| {
                l.contains("RTS_STANCE_RETURN")
                    && l.contains(&tag)
                    && l.contains("phase=arrived")
                    && int_field(l, "step").is_some_and(|s| s > start_step)
            });
            if let (Some(start), Some(arrival)) = (start, arrival) {
                let moving_steps = int_field(arrival, "step").unwrap_or(0) - start_step - 1;
                let covered = number(start, "displacement").unwrap_or(0.0)
                    - number(arrival, "displacement").unwrap_or(0.0);
                if moving_steps > 0 {
                    #[allow(clippy::cast_precision_loss)]
                    let per_step = covered / moving_steps as f64;
                    measured.push((unit, per_step, speed * 0.05, base * 0.05));
                }
            }
        }
        let paced = measured
            .iter()
            .all(|m| (m.1 - m.2).abs() <= m.2 * 0.03 && m.1 > m.3 * 1.05);
        report.verdict(
            "DISC-D3-RETURN-FASTER",
            return_ok && !measured.is_empty() && paced,
            format!(
                "{} returns logged; measured (unit, cm per step, scaled, baseline) {measured:?}; first {}",
                returns.len(),
                returns.first().map_or("none", |l| l.trim())
            ),
        );

        // 2. J: the fixture kills four members. The regroup delay follows the survivors' average
        // after the casualty shock: round(40 * (1000 + F * (-3/8)) / 1000) steps.
        let at_server = server.offset();
        client_a.key("j")?;
        let armed = server.wait_line(
            &["MP_DISCIPLINE_CASUALTIES_ARMED", &formation_tag],
            Duration::from_secs(5),
            at_server,
        )?;
        let regroup = server.wait_line(
            &["RTS_FORMATION_REGROUP ", &formation_tag],
            Duration::from_secs(15),
            at_server,
        );
        thread::sleep(Duration::from_millis(500));
        let casualty_text = server.text()[at_server..].to_owned();
        let delays = discipline_effects_of(&casualty_text, "regroup_delay");
        let delay = delays.iter().find(|l| l.contains(&formation_tag));
        let scheduled = casualty_text
            .lines()
            .find(|l| l.contains("RTS_FORMATION_REGROUP_SCHEDULED") && l.contains(&formation_tag));
        let delay_ok = delay.is_some_and(|l| {
            let value = int_field(l, "value").unwrap_or(-1);
            let permille = int_field(l, "permille").unwrap_or(-1);
            let steps = int_field(l, "steps").unwrap_or(-1);
            permille == discipline_effect_permille(value, -3, 8)
                && steps == ((40 * permille + 500) / 1000).max(1)
                && int_field(l, "due") == int_field(l, "step").map(|s| s + steps)
                && steps < 40
                && scheduled.is_some_and(|s| int_field(s, "due_step") == int_field(l, "due"))
                && regroup
                    .as_ref()
                    .is_ok_and(|r| int_field(r, "step") == int_field(l, "due"))
        });
        report.verdict(
            "DISC-D3-REGROUP-SOONER",
            delay_ok,
            format!(
                "{}; {}; {}; {}",
                armed.trim(),
                delay.map_or("no regroup_delay effect", |l| l.trim()),
                scheduled.map_or("no regroup scheduled", str::trim),
                regroup
                    .as_ref()
                    .map_or_else(|e| e.clone(), |l| l.trim().to_owned())
            ),
        );
        report.artifact(
            "regrouped_png",
            &client_a.screenshot(evidence, "03-after-casualties")?,
        );

        // 3. R: the members that fired reload; 3 s scaled by 1000 + F * (-3/20).
        let at = client_a.offset();
        client_a.key("1")?;
        wait5(&mut client_a, &["RTS_SELECTION_SET", "count="], at)?;
        let at_server = server.offset();
        client_a.key("r")?;
        thread::sleep(Duration::from_secs(5));
        let reload_text = server.text()[at_server..].to_owned();
        let reloads = discipline_effects_of(&reload_text, "reload_time");
        let mut reload_measured: Vec<(u64, f64, i64, i64)> = Vec::new();
        let mut reload_ok = !reloads.is_empty();
        for line in &reloads {
            let unit = unit_field(line);
            let value = int_field(line, "value").unwrap_or(-1);
            let permille = int_field(line, "permille").unwrap_or(-1);
            let base = number(line, "base_seconds").unwrap_or(0.0);
            let seconds = number(line, "seconds").unwrap_or(0.0);
            reload_ok &= permille == discipline_effect_permille(value, -3, 20)
                && (seconds - base * permille as f64 / 1000.0).abs() <= 0.002;
            let tag = format!(" unit={unit} ");
            let begin = reload_text
                .lines()
                .find(|l| l.contains("INFANTRY_RELOAD_BEGIN") && l.contains(&tag))
                .and_then(|l| int_field(l, "step"));
            let complete = reload_text
                .lines()
                .find(|l| l.contains("INFANTRY_RELOAD_COMPLETE") && l.contains(&tag))
                .and_then(|l| int_field(l, "step"));
            #[allow(clippy::cast_possible_truncation)]
            let expected = (seconds / 0.05).ceil() as i64;
            #[allow(clippy::cast_possible_truncation)]
            let baseline = (base / 0.05).ceil() as i64;
            match (begin, complete) {
                (Some(b), Some(c)) => {
                    reload_ok &=
                        (expected - 1..=expected).contains(&(c - b)) && c - b < baseline - 1;
                    reload_measured.push((unit, seconds, c - b, baseline));
                }
                _ => reload_ok = false,
            }
        }
        report.verdict(
            "DISC-D3-RELOAD-SHORTER",
            reload_ok,
            format!(
                "{} reloads; (unit, scaled seconds, measured steps, baseline steps) {reload_measured:?}; first {}",
                reloads.len(),
                reloads.first().map_or("none", |l| l.trim())
            ),
        );

        // The opposing side stayed at the baseline: no effect line names one of its units, and
        // every effect logged for participant 1 comes from a value away from the baseline.
        let all_effects: Vec<String> = server
            .text()
            .lines()
            .filter(|l| l.contains("RTS_DISCIPLINE_EFFECT"))
            .map(str::to_owned)
            .collect();
        let opposing = all_effects
            .iter()
            .filter(|l| field(l, "participant") == Some("2"))
            .count();
        let at_baseline = all_effects
            .iter()
            .filter(|l| int_field(l, "value") == Some(50_000))
            .count();
        let opponent_min = discipline_participant_samples_of(&server.text(), 2)
            .iter()
            .filter_map(|l| int_field(l, "min"))
            .min();
        report.verdict(
            "DISC-D3-BASELINE-NEUTRAL",
            !all_effects.is_empty() && opposing == 0 && at_baseline == 0 && opponent_min == Some(50_000),
            format!(
                "{} effect lines, naming participant 2: {opposing}, at value 50000: {at_baseline}; participant 2 minimum over the run {opponent_min:?}",
                all_effects.len()
            ),
        );

        // DISC-07: the opposing client still receives the default value only.
        let seen: Vec<String> = client_b.text()[at_b..]
            .lines()
            .filter(|l| l.contains("MP_CLIENT_SAMPLE") && field(l, "owner") == Some("1"))
            .map(str::to_owned)
            .collect();
        let leaked = seen
            .iter()
            .filter(|l| int_field(l, "resolve") != Some(50_000))
            .count();
        let own_above = client_a
            .text()
            .lines()
            .filter(|l| {
                l.contains("MP_CLIENT_SAMPLE")
                    && field(l, "owner") == Some("1")
                    && int_field(l, "resolve").is_some_and(|v| v > 50_000)
            })
            .count();
        report.verdict(
            "DISC-07-OWNER-ONLY",
            !seen.is_empty() && leaked == 0 && own_above > 0,
            format!(
                "opposing client samples of participant 1 units {}, with a value other than 50000 {leaked}; owner samples above 50000 {own_above}",
                seen.len()
            ),
        );
        let ended = server.text().lines().any(|l| {
            l.contains("MATCH_LIFECYCLE phase=5")
                || l.contains("RTS_OBJECTIVE_STATE") && l.contains("complete=1")
        });
        report.verdict(
            "FORM-MATCH-STAYS-ACTIVE",
            !ended,
            format!("terminal or captured during the session: {ended}"),
        );
        Ok(())
    })();
    launch_options_verdict(
        report,
        "DISC-D3-DEV-LAUNCH-OPTIONS",
        &[&server, &client_a, &client_b],
    );
    report.artifact("server_log", &server.log);
    report.artifact("client_a_log", &client_a.log);
    report.artifact("client_b_log", &client_b.log);
    client_a.stop();
    client_b.stop();
    server.stop();
    drop(display_a);
    result
}

// ---------------------------------------------------------------------------------------
// `--discipline-rout SERVER CLIENT [--evidence DIR] [--port N]`.
//
// DISC-D4 on the dedicated 1v1 fixture (20 idle units per seat). The server runs with the
// discipline casualty fixture, the rout seed (10 points) and participant 2 on the solo-tolerant
// doctrine as a baseline reference. Seat 1's client, with real X input, forms its 20 units into a
// Line. J sets the adherence: the fixture sets the members to 10 and kills four at one end, and the
// ordinary casualty shock takes the members near them to 0. Those break in the step they reach 0,
// leave the formation, flee to seat 1's start without firing, and refuse a real group order (only
// their owner's client sees them routed). Thirty evaluations later each rallies at 15 and takes
// orders again.
// ---------------------------------------------------------------------------------------

/// Units destroyed in `text`.
fn destroyed_units(text: &str) -> Vec<u64> {
    text.lines()
        .filter(|l| l.contains("RTS_UNIT_DESTROYED"))
        .map(unit_field)
        .collect()
}

/// `RTS_DISCIPLINE_ROUT` lines of one phase in `text`.
fn rout_lines(text: &str, phase: &str) -> Vec<String> {
    let tag = format!(" phase={phase} ");
    text.lines()
        .filter(|l| l.contains("RTS_DISCIPLINE_ROUT") && l.contains(&tag))
        .map(str::to_owned)
        .collect()
}

#[allow(clippy::too_many_lines)]
fn discipline_rout_session(options: &DedicatedOptions, report: &mut Report) -> Result<(), String> {
    let client_options = &options.client;
    let evidence = &client_options.evidence;
    let port = client_options.port;
    let mut args = server_args(port, "1v1");
    args.push(format!("-PolitikaMPUnitScale={GROUP_UNITS_PER_SEAT}"));
    args.push("-PolitikaMPUnitScaleHold".into());
    args.push("-PolitikaMPDisciplineCasualties".into());
    args.push("-PolitikaMPDisciplineRoutSeed=10".into());
    args.push("-PolitikaMPDisciplineRoutWitness".into());
    args.push("-PolitikaMPDisciplineDoctrine=2:solo".into());
    let mut server = Game::launch_server(&options.server, "server", evidence, &args)?;
    report.fact("server_command", server.command.join(" "));
    server.wait(&["MP_DEDICATED_SERVER_READY"], Duration::from_secs(180))?;
    let awaiting = server.wait(&["MP_MATCH_AWAITING_PARTICIPANTS"], Duration::from_secs(60))?;
    let generation = field(&awaiting, "generation").unwrap_or("0").to_owned();
    let fixture = server.wait(&["MP_UNIT_SCALE_FIXTURE"], Duration::from_secs(5))?;

    let display_a = Display::start(evidence, "client-a")?;
    let mut args_a = client_args(client_options, true, false, false);
    args_a.push(format!(
        "-PolitikaMPGroundProbe={}:{}/{}:{}",
        SHAPE_LINE_POINT.0, SHAPE_LINE_POINT.1, SHAPE_COLUMN_POINT.0, SHAPE_COLUMN_POINT.1
    ));
    let mut client_a = Game::launch(
        &client_options.binary,
        "client-a",
        evidence,
        &args_a,
        Some(&display_a.name()),
    )?;
    report.fact("client_a_command", client_a.command.join(" "));
    server.wait(
        &["MP_PARTICIPANT_BOUND participant=1", "remote=1"],
        Duration::from_secs(90),
    )?;
    let mut client_b = Game::launch(
        &client_options.binary,
        "client-b",
        evidence,
        &client_args(client_options, false, false, false),
        None,
    )?;
    report.fact("client_b_command", client_b.command.join(" "));
    server.wait(
        &["MP_PARTICIPANT_BOUND participant=2", "remote=1"],
        Duration::from_secs(90),
    )?;
    server.wait(
        &["MP_MATCH_ACTIVE", &format!("generation={generation}")],
        Duration::from_secs(30),
    )?;
    client_a.wait(
        &[
            "MP_CLIENT_MATCH_OBSERVED",
            "map=map.test.multiplayer_skirmish",
        ],
        Duration::from_secs(60),
    )?;
    client_a.attach_window()?;
    thread::sleep(Duration::from_secs(6));

    let result = (|| -> Result<(), String> {
        let wait5 = |game: &mut Game, fragments: &[&str], from: usize| {
            game.wait_line(fragments, Duration::from_secs(5), from)
        };
        let owners = entity_owners(&server.text());
        let own: Vec<u64> = owners
            .iter()
            .filter(|(_, o)| *o == 1)
            .map(|(e, _)| *e)
            .collect();
        report.verdict(
            "FORM-FIXTURE-20-PER-SEAT",
            own.len() == GROUP_UNITS_PER_SEAT && field(&fixture, "per_seat") == Some("20"),
            format!("participant 1 units={}; {}", own.len(), fixture.trim()),
        );
        let at_b = client_b.offset();

        // Setup: all 20 in group 1, T forms them and a click moves them into a Line.
        let before = client_a.offset();
        client_a.drag_hold(
            (24.0, 24.0),
            (f64::from(WIDTH) - 24.0, f64::from(HEIGHT) - 24.0),
        )?;
        client_a.release()?;
        client_a.wait_line(
            &["RTS_SELECTION_SET", "source=box", "op=replace"],
            Duration::from_secs(5),
            before,
        )?;
        let at = client_a.offset();
        client_a.modified_key("ctrl", "1")?;
        let group1 = wait5(&mut client_a, &["RTS_CONTROL_GROUP_ASSIGN", "slot=1 "], at)?;
        if selection_ids(&group1) != own {
            return Err(format!("control group setup failed: {}", group1.trim()));
        }
        let at_server = server.offset();
        client_a.key("t")?;
        let formed =
            server.wait_line(&["RTS_FORMATION_FORMED"], Duration::from_secs(5), at_server)?;
        let formation = field(&formed, "formation").unwrap_or("0").to_owned();
        let formation_tag = format!(" formation={formation} ");
        let line = wait_screen(&mut client_a, |t| probe_screen(t, 0), "line probe")?;
        let at_server = server.offset();
        client_a.click(line.0, line.1, "3")?;
        server.wait_line(
            &["RTS_FORMATION_MOVE", &formation_tag],
            Duration::from_secs(5),
            at_server,
        )?;
        let (arrived, at_slot, separation, failed) =
            formation_arrival(&server, at_server, &own, &own);
        report.verdict(
            "DISC-SETUP-LINE-ARRIVED",
            arrived == own.len() && at_slot == own.len() && failed == 0,
            format!(
                "{arrived}/20 arrived, {at_slot} at slot, {failed} failures, min separation {separation:.1} cm"
            ),
        );

        // 1. J: the seed sets the members to 10, then four casualties at one end.
        let rout_from = server.offset();
        client_a.key("j")?;
        let seeded = server.wait_line(
            &["MP_DISCIPLINE_ROUT_SEED", &formation_tag],
            Duration::from_secs(5),
            rout_from,
        )?;
        let armed = server.wait_line(
            &["MP_DISCIPLINE_CASUALTIES_ARMED", &formation_tag],
            Duration::from_secs(5),
            rout_from,
        )?;
        let deadline = Instant::now() + Duration::from_secs(10);
        while Instant::now() < deadline
            && server.text()[rout_from..]
                .lines()
                .filter(|l| l.contains("MP_DISCIPLINE_CASUALTY "))
                .count()
                < 4
        {
            thread::sleep(Duration::from_millis(200));
        }
        thread::sleep(Duration::from_secs(1));
        let break_text = server.text()[rout_from..].to_owned();
        let breaks = rout_lines(&break_text, "break");
        let broken: Vec<u64> = breaks.iter().map(|l| unit_field(l)).collect();
        let shocks: Vec<&str> = break_text
            .lines()
            .filter(|l| l.contains("RTS_DISCIPLINE_SHOCK") && l.contains("in_range=1"))
            .collect();
        // DISC-20: a member breaks exactly in the step a shock takes it to 0, at value 0.
        let to_zero: Vec<(u64, i64)> = shocks
            .iter()
            .filter(|l| {
                int_field(l, "after") == Some(0) && int_field(l, "before").is_some_and(|b| b > 0)
            })
            .map(|l| (unit_field(l), int_field(l, "step").unwrap_or(-1)))
            .collect();
        let matched = to_zero.iter().all(|(unit, step)| {
            breaks
                .iter()
                .any(|b| unit_field(b) == *unit && int_field(b, "step") == Some(*step))
        }) && breaks.iter().all(|b| {
            int_field(b, "value") == Some(0)
                && to_zero
                    .iter()
                    .any(|(u, s)| *u == unit_field(b) && int_field(b, "step") == Some(*s))
        });
        let victims: Vec<u64> = field(&armed, "victims")
            .unwrap_or("")
            .split(',')
            .filter_map(|v| v.parse().ok())
            .collect();
        let survivors_unbroken = own
            .iter()
            .filter(|u| !victims.contains(u) && !broken.contains(u))
            .count();
        report.verdict(
            "DISC-20-BREAK-AT-ZERO",
            !breaks.is_empty() && matched && to_zero.len() == breaks.len() && survivors_unbroken > 0,
            format!(
                "{}; {}; {} shocks in range, {} to zero; {} broke {broken:?}; {survivors_unbroken} survivors beyond the shock stayed in the line",
                seeded.trim(),
                armed.trim(),
                shocks.len(),
                to_zero.len(),
                breaks.len()
            ),
        );
        // DISC-21: each broken member left its formation as routed in the same step and took no
        // later shock.
        let left_ok = breaks.iter().all(|b| {
            let unit = unit_field(b);
            let step = int_field(b, "step");
            break_text.lines().any(|l| {
                l.contains("RTS_FORMATION_MEMBER_LEFT")
                    && unit_field(l) == unit
                    && l.contains("reason=routed")
                    && int_field(l, "step") == step
            }) && !break_text.lines().any(|l| {
                l.contains("RTS_DISCIPLINE_SHOCK")
                    && unit_field(l) == unit
                    && int_field(l, "step") > step
            })
        });
        report.verdict(
            "DISC-21-LEAVES-FORMATION",
            !breaks.is_empty() && left_ok,
            format!(
                "{} breaks; first {}",
                breaks.len(),
                breaks.first().map_or("none", |l| l.trim())
            ),
        );
        report.artifact("routed_png", &client_a.screenshot(evidence, "01-routed")?);

        // 2. A real group order: 1 selects group 1 (the routed and the rest), a right-click moves it.
        let at = client_a.offset();
        client_a.key("1")?;
        wait5(&mut client_a, &["RTS_SELECTION_SET", "count="], at)?;
        thread::sleep(Duration::from_millis(600));
        report.artifact(
            "routed_hud_png",
            &client_a.screenshot(evidence, "02-group-with-routed")?,
        );
        let column = wait_screen(&mut client_a, |t| probe_screen(t, 1), "column probe")?;
        let order_from = server.offset();
        let at = client_a.offset();
        client_a.click(column.0, column.1, "3")?;
        let outcome = client_a.wait_line(&["RTS_GROUP_ORDER_OUTCOME"], Duration::from_secs(10), at);
        thread::sleep(Duration::from_millis(500));
        let order_text = server.text()[order_from..].to_owned();
        // A fixture victim can break from an earlier casualty and then fall itself.
        let fallen = destroyed_units(&server.text()[rout_from..order_from]);
        let still_routed: Vec<u64> = broken
            .iter()
            .copied()
            .filter(|u| {
                !fallen.contains(u)
                    && !rout_lines(&server.text()[rout_from..order_from], "rally")
                        .iter()
                        .any(|l| unit_field(l) == *u)
            })
            .collect();
        let refused: Vec<u64> = order_text
            .lines()
            .filter(|l| l.contains("RTS_ORDER_REJECTED") && int_field(l, "outcome") == Some(19))
            .map(unit_field)
            .collect();
        let admitted_routed = order_text
            .lines()
            .filter(|l| l.contains("RTS_ORDER_ADMITTED") && broken.contains(&unit_field(l)))
            .count();
        let mut refused_sorted = refused.clone();
        refused_sorted.sort_unstable();
        let mut expected = still_routed.clone();
        expected.sort_unstable();
        report.verdict(
            "DISC-24-ORDERS-REFUSED",
            !expected.is_empty()
                && refused_sorted == expected
                && admitted_routed == 0
                && outcome.as_ref().is_ok_and(|l| {
                    int_field(l, "rejected") == i64::try_from(expected.len()).ok()
                        && int_field(l, "dominant_rejection") == Some(19)
                }),
            format!(
                "refused as routed {refused_sorted:?}, routed at the order {expected:?}, routed admitted {admitted_routed}; client {}",
                outcome.as_ref().map_or_else(|e| e.clone(), |l| l.trim().to_owned())
            ),
        );
        report.artifact(
            "refused_png",
            &client_a.screenshot(evidence, "03-order-refused")?,
        );

        // 3. Rally: 30 evaluations after each break, at 15, on an evaluation step. Units that fell
        // while routed are reported and left out.
        let deadline = Instant::now() + Duration::from_secs(45);
        loop {
            let text = server.text()[rout_from..].to_owned();
            let fallen = destroyed_units(&text);
            let living = broken.iter().filter(|u| !fallen.contains(u)).count();
            if rout_lines(&text, "rally").len() >= living || Instant::now() >= deadline {
                break;
            }
            thread::sleep(Duration::from_millis(500));
        }
        let rout_text = server.text()[rout_from..].to_owned();
        let fallen = destroyed_units(&rout_text);
        let breaks: Vec<String> = breaks
            .into_iter()
            .filter(|b| !fallen.contains(&unit_field(b)))
            .collect();
        let fell_routed: Vec<u64> = broken
            .iter()
            .copied()
            .filter(|u| fallen.contains(u))
            .collect();
        let broken: Vec<u64> = breaks.iter().map(|l| unit_field(l)).collect();
        let rallies = rout_lines(&rout_text, "rally");
        let flights = rout_lines(&rout_text, "flight_end");
        // DISC-22: each fled along a route towards seat 1's start and got closer to it.
        let fled_ok = breaks.iter().all(|b| {
            let unit = unit_field(b);
            let start = number(b, "rally_distance").unwrap_or(0.0);
            let end = flights
                .iter()
                .find(|f| unit_field(f) == unit)
                .and_then(|f| number(f, "rally_distance"))
                .or_else(|| {
                    rallies
                        .iter()
                        .find(|r| unit_field(r) == unit)
                        .and_then(|r| number(r, "rally_distance"))
                });
            int_field(b, "route") == Some(1)
                && field(b, "order") == Some("1")
                && end.is_some_and(|e| e < start - 100.0 || e <= 30.0)
        });
        let rally_points: Vec<&str> = breaks.iter().filter_map(|b| field(b, "rally")).collect();
        report.verdict(
            "DISC-22-FLEES-TO-START",
            !breaks.is_empty() && fled_ok && rally_points.windows(2).all(|w| w[0] == w[1]),
            format!(
                "{} living routed units; fell while routed {fell_routed:?}; rally points {rally_points:?}; flights ended {}; first {}",
                breaks.len(),
                flights.len(),
                flights.first().map_or("none", |l| l.trim())
            ),
        );
        // DISC-23: between break and rally no routed unit fired or hit anything (below the
        // baseline every shot logs its fire interval), and it took no order (DISC-24).
        let quiet = breaks.iter().all(|b| {
            let unit = unit_field(b);
            let from = int_field(b, "step").unwrap_or(0);
            let to = rallies
                .iter()
                .find(|r| unit_field(r) == unit)
                .and_then(|r| int_field(r, "step"))
                .unwrap_or(i64::MAX);
            let shooter = format!(" shooter={unit} ");
            !rout_text.lines().any(|l| {
                let step = int_field(l, "step").unwrap_or(-1);
                step > from
                    && step < to
                    && ((l.contains("RTS_DISCIPLINE_EFFECT")
                        && l.contains("effect=fire_interval")
                        && unit_field(l) == unit)
                        || l.contains(&shooter))
            })
        });
        report.verdict(
            "DISC-23-ROUTED-DO-NOT-FIGHT",
            !breaks.is_empty() && quiet && admitted_routed == 0,
            format!("no shot, hit or admitted order from a routed unit between its break and its rally: {quiet}"),
        );
        // The first evaluation is the break step itself when it falls on one (the break comes
        // first in the step), else the next; the rally is 29 evaluations after that.
        let rally_ok = !breaks.is_empty()
            && rallies.len() == breaks.len()
            && breaks.iter().all(|b| {
                let step = int_field(b, "step").unwrap_or(-1);
                let expected = (step + 19) / 20 * 20 + 29 * 20;
                rallies.iter().any(|r| {
                    unit_field(r) == unit_field(b)
                        && int_field(r, "value") == Some(15_000)
                        && int_field(r, "step") == Some(expected)
                        && int_field(r, "routed_steps") == Some(expected - step)
                })
            });
        report.verdict(
            "DISC-26-RALLY-AT-15",
            rally_ok,
            format!(
                "{} rallies for {} breaks; first {}",
                rallies.len(),
                breaks.len(),
                rallies.first().map_or("none", |l| l.trim())
            ),
        );

        // 4. After the rally the same real group order is accepted by every unit.
        let at = client_a.offset();
        client_a.key("1")?;
        wait5(&mut client_a, &["RTS_SELECTION_SET", "count="], at)?;
        let line = wait_screen(&mut client_a, |t| probe_screen(t, 0), "line probe")?;
        let order_from = server.offset();
        let at = client_a.offset();
        client_a.click(line.0, line.1, "3")?;
        let outcome = client_a.wait_line(&["RTS_GROUP_ORDER_OUTCOME"], Duration::from_secs(10), at);
        thread::sleep(Duration::from_millis(500));
        let after_text = server.text()[order_from..].to_owned();
        let readmitted = broken
            .iter()
            .filter(|u| {
                after_text
                    .lines()
                    .any(|l| l.contains("RTS_ORDER_ADMITTED") && unit_field(l) == **u)
            })
            .count();
        report.verdict(
            "DISC-26-ORDERS-AFTER-RALLY",
            readmitted == broken.len()
                && outcome
                    .as_ref()
                    .is_ok_and(|l| int_field(l, "rejected") == Some(0)),
            format!(
                "{readmitted}/{} rallied units admitted; client {}",
                broken.len(),
                outcome
                    .as_ref()
                    .map_or_else(|e| e.clone(), |l| l.trim().to_owned())
            ),
        );
        report.artifact("rallied_png", &client_a.screenshot(evidence, "04-rallied")?);

        // Owner-only: client A saw its units routed; client B never did, nor any other value.
        let own_routed = client_a
            .text()
            .lines()
            .filter(|l| {
                l.contains("MP_CLIENT_SAMPLE")
                    && field(l, "owner") == Some("1")
                    && int_field(l, "routed") == Some(1)
            })
            .count();
        let seen: Vec<String> = client_b.text()[at_b..]
            .lines()
            .filter(|l| l.contains("MP_CLIENT_SAMPLE") && field(l, "owner") == Some("1"))
            .map(str::to_owned)
            .collect();
        let leaked = seen
            .iter()
            .filter(|l| {
                int_field(l, "routed") != Some(0) || int_field(l, "resolve") != Some(50_000)
            })
            .count();
        let seen_routed = seen
            .iter()
            .filter(|l| {
                field(l, "entity")
                    .and_then(|v| v.parse::<u64>().ok())
                    .is_some_and(|e| broken.contains(&e))
            })
            .count();
        let witness = server
            .text()
            .lines()
            .find(|l| l.contains("MP_DISCIPLINE_ROUT_WITNESS"))
            .map(str::to_owned);
        report.verdict(
            "DISC-07-OWNER-ONLY-ROUT",
            own_routed > 0 && seen_routed > 0 && leaked == 0,
            format!(
                "owner samples routed {own_routed}; opposing client samples of participant 1 units {} ({seen_routed} of units that routed), routed or not at 50000 {leaked}; {}",
                seen.len(),
                witness.as_deref().map_or("no witness line", str::trim)
            ),
        );
        let opponent_breaks = rout_lines(&server.text(), "break")
            .iter()
            .filter(|l| field(l, "participant") == Some("2"))
            .count();
        report.verdict(
            "DISC-D4-BASELINE-NO-ROUT",
            opponent_breaks == 0,
            format!("breaks of participant 2: {opponent_breaks}"),
        );
        let ended = server.text().lines().any(|l| {
            l.contains("MATCH_LIFECYCLE phase=5")
                || l.contains("RTS_OBJECTIVE_STATE") && l.contains("complete=1")
        });
        report.verdict(
            "FORM-MATCH-STAYS-ACTIVE",
            !ended,
            format!("terminal or captured during the session: {ended}"),
        );
        Ok(())
    })();
    launch_options_verdict(
        report,
        "DISC-D4-DEV-LAUNCH-OPTIONS",
        &[&server, &client_a, &client_b],
    );
    report.artifact("server_log", &server.log);
    report.artifact("client_a_log", &client_a.log);
    report.artifact("client_b_log", &client_b.log);
    client_a.stop();
    client_b.stop();
    server.stop();
    drop(display_a);
    result
}

/// `RTS_DISCIPLINE_PARTICIPANT` lines of one participant in `text`.
fn discipline_participant_samples_of(text: &str, participant: u32) -> Vec<String> {
    let tag = format!(" participant={participant} ");
    text.lines()
        .filter(|l| l.contains("RTS_DISCIPLINE_PARTICIPANT") && l.contains(&tag))
        .map(str::to_owned)
        .collect()
}

// ECON-E1: treasury, income and training by real input on a packaged dedicated server.
// `--economy SERVER CLIENT [--evidence DIR] [--port N]` plays the default fixture through money,
// training, a refused purchase, a refund, a commanded new unit, a match end with an item in
// training and the next generation. `--economy-limits` gives both seats 15 units and $100,000 to
// reach the queue limit and the unit limit.
const ECONOMY_START_CENTS: i64 = 100_000;
const ECONOMY_PRICE_CENTS: i64 = 36_000;
const ECONOMY_TRAIN_STEPS: u64 = 200;
const ECONOMY_INCOME_CENTS: i64 = 2_000;
const ECONOMY_INCOME_STEPS: u64 = 200;
const ECONOMY_LIMITS_CENTS: i64 = 10_000_000;

fn econ_int(line: &str, key: &str) -> i64 {
    field(line, key).and_then(|v| v.parse().ok()).unwrap_or(-1)
}

/// The server's money lines for one participant in one generation, in log order.
fn economy_lines(text: &str, participant: u32, generation: &str) -> Vec<String> {
    let tag = format!("participant={participant} ");
    let generation = format!("generation={generation} ");
    text.lines()
        .filter(|l| l.contains("RTS_ECONOMY_TX") && l.contains(&tag) && l.contains(&generation))
        .map(str::to_owned)
        .collect()
}

/// Replays the money lines from the start: every balance follows from the one before it.
fn economy_replay(lines: &[String], start: i64) -> Result<(), String> {
    let mut available = start;
    for line in lines {
        let amount = econ_int(line, "amount");
        match field(line, "kind") {
            Some("income") | Some("refund") | Some("sale") => available += amount,
            Some("train") | Some("food") => available -= amount,
            other => return Err(format!("unknown kind {other:?}: {}", line.trim())),
        }
        if econ_int(line, "available") != available || field(line, "reconciles") != Some("1") {
            return Err(format!("expected available {available}: {}", line.trim()));
        }
    }
    Ok(())
}

/// Income steps are exact multiples of the interval with none missing, each paying the fixed sum.
fn economy_income_cadence(lines: &[String]) -> Result<usize, String> {
    let mut count = 0_u64;
    for line in lines.iter().filter(|l| field(l, "kind") == Some("income")) {
        count += 1;
        let step = econ_int(line, "step");
        if step != (count * ECONOMY_INCOME_STEPS) as i64
            || econ_int(line, "amount") != ECONOMY_INCOME_CENTS
        {
            return Err(format!("income {count} off its cadence: {}", line.trim()));
        }
    }
    Ok(count as usize)
}

fn economy_key_outcome(
    client: &mut Game,
    server: &mut Game,
    modifier: Option<&str>,
    server_fragments: &[&str],
) -> Result<(String, String), String> {
    let (client_mark, server_mark) = (client.offset(), server.offset());
    match modifier {
        Some(held) => client.modified_key(held, "b")?,
        None => client.key("b")?,
    }
    let on_server = server.wait_line(server_fragments, Duration::from_secs(5), server_mark)?;
    let on_client = client.wait_line(
        &["RTS_ECONOMY_OUTCOME participant=1"],
        Duration::from_secs(5),
        client_mark,
    )?;
    Ok((on_server, on_client))
}

fn economy_start_session(
    options: &DedicatedOptions,
    extra: &[String],
) -> Result<(Game, Game, Game, Display, String), String> {
    economy_start_session_with(options, extra, &[])
}

/// As `economy_start_session`, with extra switches for the rendered client.
fn economy_start_session_with(
    options: &DedicatedOptions,
    extra: &[String],
    client_a_extra: &[String],
) -> Result<(Game, Game, Game, Display, String), String> {
    let client_options = &options.client;
    let evidence = &client_options.evidence;
    let mut args = server_args(client_options.port, "1v1");
    if extra.iter().any(|a| a == STARTING_HARVESTERS) {
        args.retain(|a| a != NO_STARTING_HARVESTERS);
    }
    args.extend_from_slice(extra);
    let mut server = Game::launch_server(&options.server, "server", evidence, &args)?;
    server.wait(&["MP_DEDICATED_SERVER_READY"], Duration::from_secs(180))?;
    let awaiting = server.wait(&["MP_MATCH_AWAITING_PARTICIPANTS"], Duration::from_secs(60))?;
    let generation = field(&awaiting, "generation").unwrap_or("0").to_owned();
    let display_a = Display::start(evidence, "client-a")?;
    let mut args_a = client_args(client_options, true, false, false);
    args_a.extend_from_slice(client_a_extra);
    let mut client_a = Game::launch(
        &client_options.binary,
        "client-a",
        evidence,
        &args_a,
        Some(&display_a.name()),
    )?;
    server.wait(
        &["MP_PARTICIPANT_BOUND participant=1", "remote=1"],
        Duration::from_secs(90),
    )?;
    let client_b = Game::launch(
        &client_options.binary,
        "client-b",
        evidence,
        &client_args(client_options, false, false, false),
        None,
    )?;
    server.wait(
        &["MP_PARTICIPANT_BOUND participant=2", "remote=1"],
        Duration::from_secs(90),
    )?;
    server.wait(
        &["MP_MATCH_ACTIVE", &format!("generation={generation}")],
        Duration::from_secs(30),
    )?;
    client_a.wait(
        &[
            "MP_CLIENT_MATCH_OBSERVED",
            "map=map.test.multiplayer_skirmish",
        ],
        Duration::from_secs(60),
    )?;
    client_a.attach_window()?;
    thread::sleep(Duration::from_secs(6));
    Ok((server, client_a, client_b, display_a, generation))
}

fn economy_session(options: &DedicatedOptions, report: &mut Report) -> Result<(), String> {
    let evidence = options.client.evidence.clone();
    let (mut server, mut client_a, mut client_b, _display, generation) =
        economy_start_session(options, &[])?;
    report.fact("server_command", server.command.join(" "));
    report.fact("client_a_command", client_a.command.join(" "));
    report.fact("client_b_command", client_b.command.join(" "));

    let result = (|| -> Result<(), String> {
        // Accounts and structures.
        let text = server.text();
        let starts: Vec<&str> = text
            .lines()
            .filter(|l| {
                l.contains("RTS_ECONOMY_START") && l.contains(&format!("generation={generation}"))
            })
            .collect();
        report.verdict(
            "ECON-P01-ACCOUNTS",
            starts.len() == 2
                && starts.iter().all(|l| {
                    econ_int(l, "available") == ECONOMY_START_CENTS
                        && field(l, "producer") == Some("1")
                })
                && field(starts[0], "participant") == Some("1")
                && field(starts[1], "participant") == Some("2"),
            starts.join(" | "),
        );
        let (text_a, text_b) = (client_a.text(), client_b.text());
        let structures = |t: &str| -> Vec<String> {
            t.lines()
                .filter(|l| l.contains("RTS_STRUCTURE_RELEVANT"))
                .filter_map(|l| field(l, "owner").map(str::to_owned))
                .collect()
        };
        report.verdict(
            "ECON-P02-STRUCTURE-OWN-TEAM-ONLY",
            structures(&text_a) == ["1", "1"] && structures(&text_b) == ["2", "2"],
            format!(
                "client A received {:?}, client B received {:?}",
                structures(&text_a),
                structures(&text_b)
            ),
        );
        zoom_out(&client_a, 5)?;
        thread::sleep(Duration::from_millis(800));
        report.artifact(
            "client_a_start_png",
            &client_a.screenshot(&evidence, "01-start")?,
        );

        // Two purchases are affordable; the third is not.
        let (first, first_client) = economy_key_outcome(
            &mut client_a,
            &mut server,
            None,
            &["RTS_ECONOMY_TX participant=1 kind=train"],
        )?;
        let (second, _) = economy_key_outcome(
            &mut client_a,
            &mut server,
            None,
            &["RTS_ECONOMY_TX participant=1 kind=train"],
        )?;
        report.verdict(
            "ECON-P03-TRAIN-DEBITS-ONCE",
            econ_int(&first, "amount") == ECONOMY_PRICE_CENTS
                && econ_int(&first, "queued") == 1
                && econ_int(&second, "queued") == 2
                && econ_int(&second, "spent") - econ_int(&first, "spent") == ECONOMY_PRICE_CENTS
                // Scheduled income may land between the two purchases.
                && econ_int(&first, "available") + econ_int(&second, "credited")
                    - econ_int(&first, "credited")
                    - econ_int(&second, "available")
                    == ECONOMY_PRICE_CENTS
                && field(&first_client, "accepted") == Some("1")
                && econ_int(&first_client, "amount") == ECONOMY_PRICE_CENTS,
            format!(
                "{} | {} | client {}",
                first.trim(),
                second.trim(),
                first_client.trim()
            ),
        );
        thread::sleep(Duration::from_millis(600));
        report.artifact(
            "client_a_training_png",
            &client_a.screenshot(&evidence, "02-training")?,
        );

        let money_before = economy_lines(&server.text(), 1, &generation).len();
        let (refused, refused_client) = economy_key_outcome(
            &mut client_a,
            &mut server,
            None,
            &[
                "RTS_ECONOMY_REJECT participant=1",
                "rejection=insufficient_funds",
            ],
        )?;
        let spends_after = economy_lines(&server.text(), 1, &generation)
            .iter()
            .skip(money_before)
            .filter(|l| field(l, "kind") != Some("income"))
            .count();
        report.verdict(
            "ECON-P04-INSUFFICIENT-FUNDS",
            field(&refused_client, "accepted") == Some("0")
                && field(&refused_client, "rejection") == Some("8")
                && spends_after == 0,
            format!(
                "{} | client {}; {spends_after} money lines other than income",
                refused.trim(),
                refused_client.trim()
            ),
        );
        thread::sleep(Duration::from_millis(400));
        report.artifact(
            "client_a_refused_png",
            &client_a.screenshot(&evidence, "03-refused")?,
        );

        // Shift+B refunds the newest entry, once.
        let (refund, refund_client) = economy_key_outcome(
            &mut client_a,
            &mut server,
            Some("shift"),
            &["RTS_ECONOMY_TX participant=1 kind=refund"],
        )?;
        report.verdict(
            "ECON-P05-CANCEL-REFUNDS",
            econ_int(&refund, "amount") == ECONOMY_PRICE_CENTS
                && econ_int(&refund, "queued") == 1
                && field(&refund, "transaction").is_some()
                && field(&refund_client, "accepted") == Some("1"),
            format!("{} | client {}", refund.trim(), refund_client.trim()),
        );

        // The first entry appears exactly its training time after admission, as an ordinary unit.
        let first_transaction = field(&first, "transaction").unwrap_or("0").to_owned();
        let spawn = server.wait(
            &[
                "RTS_PRODUCTION_SPAWN participant=1",
                &format!("transaction={first_transaction} "),
            ],
            Duration::from_secs(20),
        )?;
        let entity = econ_int(&spawn, "entity") as u64;
        let delta = econ_int(&spawn, "step") - econ_int(&first, "step");
        report.verdict(
            "ECON-P06-TRAINING-TIME",
            delta == ECONOMY_TRAIN_STEPS as i64,
            format!(
                "admitted at step {}, appeared {delta} steps later: {}",
                econ_int(&first, "step"),
                spawn.trim()
            ),
        );
        let owners_seen = client_a.wait(
            &[
                "MP_CLIENT_UNIT_RELEVANT",
                &format!("entity={entity} "),
                "owner=1",
            ],
            Duration::from_secs(5),
        )?;
        thread::sleep(Duration::from_secs(1));
        let b_saw = client_b.text().lines().any(|l| {
            l.contains("MP_CLIENT_UNIT_RELEVANT") && l.contains(&format!("entity={entity} "))
        });
        report.verdict(
            "ECON-P07-NEW-UNIT-REPLICATES-BY-VISION",
            !b_saw,
            format!("{}; client B received it: {b_saw}", owners_seen.trim()),
        );
        report.artifact(
            "client_a_spawned_png",
            &client_a.screenshot(&evidence, "04-spawned")?,
        );
        let moved = order_move(&mut client_a, &mut server, entity, (640.0, 360.0))?;
        report.verdict("ECON-P08-NEW-UNIT-TAKES-ORDERS", true, moved.trim());

        // A match end with an item in training: the capture goes on while training runs.
        let owned: Vec<u64> = entity_owners(&server.text())
            .iter()
            .filter(|(_, owner)| *owner == 1)
            .map(|(id, _)| *id)
            .collect();
        let unit = *owned.first().ok_or("no authored unit for participant 1")?;
        select_entity(&mut client_a, "MP_CLIENT_SAMPLE", unit)?;
        thread::sleep(Duration::from_millis(600));
        let objective = wait_screen(
            &mut client_a,
            |t| objective_screen(t, "MP_CLIENT_OBJECTIVE"),
            "objective on client A",
        )?;
        let at_move = server.offset();
        client_a.click(objective.0, objective.1, "3")?;
        server.wait_line(
            &[
                "RTS_ORDER_ADMITTED order=Move",
                &format!("unit={unit}"),
                "participant=1",
            ],
            Duration::from_secs(5),
            at_move,
        )?;
        let progress_deadline = Instant::now() + Duration::from_secs(120);
        loop {
            let halfway = server
                .text()
                .lines()
                .rev()
                .find(|l| l.contains("MP_SERVER_OBJECTIVE"))
                .is_some_and(|l| econ_int(l, "progress_milli") >= 500);
            if halfway {
                break;
            }
            if Instant::now() > progress_deadline {
                return Err("the objective did not reach half progress".into());
            }
            thread::sleep(Duration::from_millis(200));
        }
        let (in_flight, _) = economy_key_outcome(
            &mut client_a,
            &mut server,
            None,
            &["RTS_ECONOMY_TX participant=1 kind=train"],
        )?;
        let results = server.wait(
            &[
                "MATCH_RESULTS outcome=objective_secured",
                &format!("generation={generation}"),
            ],
            Duration::from_secs(60),
        )?;
        let cleared = server.wait(
            &["RTS_ECONOMY_CLEARED participant=1", "reason=match_ended"],
            Duration::from_secs(5),
        )?;
        let at_end = server.offset();
        report.verdict(
            "ECON-P09-MATCH-END-DROPS-QUEUE",
            field(&cleared, "dropped_queue") == Some("1")
                && field(&cleared, "reconciles") == Some("1"),
            format!(
                "{} | {} | {}",
                in_flight.trim(),
                results.trim(),
                cleared.trim()
            ),
        );
        let late_client = client_a.offset();
        client_a.key("b")?;
        let late_outcome = client_a.wait_line(
            &["RTS_ECONOMY_OUTCOME participant=1"],
            Duration::from_secs(5),
            late_client,
        )?;
        thread::sleep(Duration::from_secs(ECONOMY_TRAIN_STEPS / 20 + 2));
        let after = server.text();
        let tail = &after[at_end.min(after.len())..];
        let moved_after = tail
            .lines()
            .filter(|l| l.contains("RTS_ECONOMY_TX") || l.contains("RTS_PRODUCTION_SPAWN"))
            .count();
        report.verdict(
            "ECON-P10-NOTHING-AFTER-THE-END",
            moved_after == 0
                && field(&late_outcome, "accepted") == Some("0")
                && field(&late_outcome, "rejection") == Some("4"),
            format!(
                "{moved_after} money or spawn lines after the end; late request: {}",
                late_outcome.trim()
            ),
        );

        // The ledger explains itself from the server's lines alone.
        let lines_1 = economy_lines(&after, 1, &generation);
        let lines_2 = economy_lines(&after, 2, &generation);
        let replay = economy_replay(&lines_1, ECONOMY_START_CENTS)
            .and(economy_replay(&lines_2, ECONOMY_START_CENTS));
        let cadence_1 = economy_income_cadence(&lines_1);
        let cadence_2 = economy_income_cadence(&lines_2);
        report.verdict(
            "ECON-P11-LEDGER-REPLAYS",
            replay.is_ok(),
            format!(
                "{} lines for participant 1, {} for participant 2: {replay:?}",
                lines_1.len(),
                lines_2.len()
            ),
        );
        report.verdict(
            "ECON-P12-INCOME-CADENCE",
            cadence_1.as_ref().is_ok_and(|n| *n >= 1) && cadence_1 == cadence_2,
            format!("{cadence_1:?} payments to participant 1, {cadence_2:?} to participant 2"),
        );

        // Each client's own view matches the server, and neither learns the other's.
        let view_of = |t: &str, participant: &str| -> Vec<String> {
            t.lines()
                .filter(|l| {
                    l.contains("RTS_ECONOMY_VIEW") && field(l, "participant") == Some(participant)
                })
                .map(str::to_owned)
                .collect()
        };
        let (text_a, text_b) = (client_a.text(), client_b.text());
        let views_a = view_of(&text_a, "1");
        let views_b = view_of(&text_b, "2");
        let last_a = views_a.last().cloned().unwrap_or_default();
        let last_b = views_b.last().cloned().unwrap_or_default();
        let final_1 = lines_1.last().map_or(-1, |l| econ_int(l, "available"));
        let final_2 = lines_2.last().map_or(-1, |l| econ_int(l, "available"));
        report.verdict(
            "ECON-P13-VIEW-MATCHES-SERVER",
            econ_int(&last_a, "available") == final_1 && econ_int(&last_b, "available") == final_2,
            format!(
                "server {final_1} and {final_2}; A {} | B {}",
                last_a.trim(),
                last_b.trim()
            ),
        );
        let a_leaks = view_of(&text_a, "2").len();
        let b_leaks = view_of(&text_b, "1").len();
        let b_spent = views_b.iter().filter(|l| econ_int(l, "spent") != 0).count();
        report.verdict(
            "ECON-P14-OWNER-ONLY-VIEW",
            a_leaks == 0 && b_leaks == 0 && !views_b.is_empty() && b_spent == 0,
            format!("A's copies of B's view {a_leaks}, B's copies of A's view {b_leaks}; {} B views, {b_spent} with spending", views_b.len()),
        );

        // Repeatability: the normalized sequence, without timing or income interleaving.
        let trace: String = lines_1
            .iter()
            .filter(|l| field(l, "kind") != Some("income"))
            .map(|l| {
                format!(
                    "{} {} queued={}\n",
                    field(l, "kind").unwrap_or("?"),
                    econ_int(l, "amount"),
                    econ_int(l, "queued")
                )
            })
            .chain([format!("training_steps={delta}\n")])
            .collect();
        fs::write(evidence.join("economy-trace.txt"), &trace).map_err(|e| e.to_string())?;
        report.artifact("economy_trace", &evidence.join("economy-trace.txt"));
        report.artifact(
            "client_a_results_png",
            &client_a.screenshot(&evidence, "05-results")?,
        );

        // The next generation starts clean.
        let at_reset = server.offset();
        client_a.hover(640.0, 360.0)?;
        client_a.key("Escape")?;
        client_b.stop();
        server.wait_line(
            &["MP_DEDICATED_SESSION_RESET", "travel=1"],
            Duration::from_secs(60),
            at_reset,
        )?;
        let next = server.wait_line(
            &["MP_MATCH_AWAITING_PARTICIPANTS"],
            Duration::from_secs(90),
            at_reset,
        )?;
        let generation2 = field(&next, "generation").unwrap_or("0").to_owned();
        let text = server.text();
        let tail = &text[at_reset.min(text.len())..];
        let starts2: Vec<&str> = tail
            .lines()
            .filter(|l| {
                l.contains("RTS_ECONOMY_START") && l.contains(&format!("generation={generation2}"))
            })
            .collect();
        report.verdict(
            "ECON-P15-NEXT-GENERATION-FRESH",
            generation2 != generation
                && starts2.len() == 2
                && starts2
                    .iter()
                    .all(|l| econ_int(l, "available") == ECONOMY_START_CENTS)
                && !tail.lines().any(|l| l.contains("RTS_ECONOMY_TX")),
            format!(
                "generation {generation} then {generation2}: {}",
                starts2.join(" | ")
            ),
        );
        Ok(())
    })();
    launch_options_verdict(
        report,
        "ECON-P00-LAUNCH-OPTIONS",
        &[&server, &client_a, &client_b],
    );
    client_a.stop();
    client_b.stop();
    server.stop();
    result
}

// `--power SERVER CLIENT [--evidence DIR] [--port N]` (ECON-E2) plays one generator outage with
// real input: two purchases, the outage the fixture starts after the first, a refused purchase, a
// refund, the recovery, the delayed unit, a purchase after the recovery, a match end and the next
// generation. Generators cannot be damaged yet, so only the development fixture causes an outage.
const POWER_OUTAGE_OFFSET_STEPS: i64 = 60;
const POWER_OUTAGE_STEPS: i64 = 120;
const POWER_SUPPLIED_KW: i64 = 100;
const POWER_REQUIRED_KW: i64 = 50;

fn power_session(options: &DedicatedOptions, report: &mut Report) -> Result<(), String> {
    let evidence = options.client.evidence.clone();
    let fixture =
        format!("-PolitikaMPPowerOutage={POWER_OUTAGE_OFFSET_STEPS}:{POWER_OUTAGE_STEPS}");
    let (mut server, mut client_a, mut client_b, _display, generation) =
        economy_start_session(options, &[fixture])?;
    report.fact("server_command", server.command.join(" "));
    report.fact("client_a_command", client_a.command.join(" "));
    report.fact("client_b_command", client_b.command.join(" "));

    let result = (|| -> Result<(), String> {
        // Every building has a positive load and the base starts powered.
        let text = server.text();
        let starts: Vec<&str> = text
            .lines()
            .filter(|l| {
                l.contains("RTS_POWER_START") && l.contains(&format!("generation={generation}"))
            })
            .collect();
        report.verdict(
            "PWR-P01-BASE-STARTS-POWERED",
            starts.len() == 2
                && starts.iter().all(|l| {
                    econ_int(l, "supplied_kw") == POWER_SUPPLIED_KW
                        && econ_int(l, "required_kw") == POWER_REQUIRED_KW
                        && econ_int(l, "generator_load_kw") > 0
                        && econ_int(l, "producer_load_kw") > 0
                        && field(l, "producer_powered") == Some("1")
                }),
            starts.join(" | "),
        );
        let (text_a, text_b) = (client_a.text(), client_b.text());
        let buildings = |t: &str| -> Vec<String> {
            let mut found: Vec<String> = t
                .lines()
                .filter(|l| l.contains("RTS_STRUCTURE_RELEVANT"))
                .map(|l| {
                    format!(
                        "{}:{}",
                        field(l, "kind").unwrap_or("?"),
                        field(l, "owner").unwrap_or("?")
                    )
                })
                .collect();
            found.sort();
            found
        };
        report.verdict(
            "PWR-P02-BUILDINGS-OWN-TEAM-ONLY",
            buildings(&text_a) == ["generator:1", "producer:1"]
                && buildings(&text_b) == ["generator:2", "producer:2"],
            format!(
                "client A received {:?}, client B received {:?}",
                buildings(&text_a),
                buildings(&text_b)
            ),
        );
        zoom_out(&client_a, 5)?;
        thread::sleep(Duration::from_millis(800));
        report.artifact(
            "client_a_start_png",
            &client_a.screenshot(&evidence, "01-start")?,
        );

        // Two purchases while powered; the first also starts the fixture's clock.
        let (first, _) = economy_key_outcome(
            &mut client_a,
            &mut server,
            None,
            &["RTS_ECONOMY_TX participant=1 kind=train"],
        )?;
        let admitted = econ_int(&first, "step");
        thread::sleep(Duration::from_millis(400));
        let (second, _) = economy_key_outcome(
            &mut client_a,
            &mut server,
            None,
            &["RTS_ECONOMY_TX participant=1 kind=train"],
        )?;

        let off = server.wait(
            &["RTS_POWER_STATE participant=1", "producer_powered=0"],
            Duration::from_secs(20),
        )?;
        let off_step = econ_int(&off, "step");
        let off_remaining = econ_int(&off, "head_remaining");
        report.verdict(
            "PWR-P03-OUTAGE-PAUSES-THE-QUEUE",
            off_step == admitted + POWER_OUTAGE_OFFSET_STEPS
                && econ_int(&off, "supplied_kw") == 0
                && econ_int(&off, "required_kw") == POWER_REQUIRED_KW
                && econ_int(&off, "queued") == 2,
            format!("{} | {} | {}", first.trim(), second.trim(), off.trim()),
        );
        let offline_view = client_a.wait(
            &["RTS_ECONOMY_VIEW participant=1", "powered=0"],
            Duration::from_secs(5),
        )?;
        thread::sleep(Duration::from_millis(600));
        report.artifact(
            "client_a_outage_png",
            &client_a.screenshot(&evidence, "02-outage")?,
        );

        // Purchases are refused with a reason and no charge, and Cancel still refunds.
        let (refused, refused_client) = economy_key_outcome(
            &mut client_a,
            &mut server,
            None,
            &["RTS_ECONOMY_REJECT participant=1", "op=0"],
        )?;
        report.verdict(
            "PWR-P04-UNPOWERED-REFUSES-TRAINING",
            field(&refused, "rejection") == Some("producer_unpowered")
                && field(&refused_client, "accepted") == Some("0")
                && econ_int(&refused_client, "amount") == 0,
            format!("{} | client {}", refused.trim(), refused_client.trim()),
        );
        thread::sleep(Duration::from_millis(400));
        report.artifact(
            "client_a_refused_png",
            &client_a.screenshot(&evidence, "03-refused")?,
        );
        let (refund, _) = economy_key_outcome(
            &mut client_a,
            &mut server,
            Some("shift"),
            &["RTS_ECONOMY_TX participant=1 kind=refund"],
        )?;
        report.verdict(
            "PWR-P05-CANCEL-REFUNDS-IN-AN-OUTAGE",
            econ_int(&refund, "amount") == ECONOMY_PRICE_CENTS
                && econ_int(&refund, "queued") == 1
                && econ_int(&refund, "step") < off_step + POWER_OUTAGE_STEPS,
            refund.trim(),
        );

        // Recovery resumes the same entry, and the unit is late by exactly the outage.
        let on = server.wait(
            &["RTS_POWER_STATE participant=1", "producer_powered=1"],
            Duration::from_secs(20),
        )?;
        report.verdict(
            "PWR-P06-RECOVERY-RESUMES-THE-ENTRY",
            econ_int(&on, "step") == off_step + POWER_OUTAGE_STEPS
                && econ_int(&on, "supplied_kw") == POWER_SUPPLIED_KW
                && econ_int(&on, "queued") == 1
                && econ_int(&on, "head_remaining") == off_remaining,
            format!("off {} | on {}", off.trim(), on.trim()),
        );
        let spawn = server.wait(
            &["RTS_PRODUCTION_SPAWN participant=1"],
            Duration::from_secs(30),
        )?;
        let delay = econ_int(&spawn, "step") - admitted;
        report.verdict(
            "PWR-P07-UNIT-LATE-BY-THE-OUTAGE",
            delay == ECONOMY_TRAIN_STEPS as i64 + POWER_OUTAGE_STEPS,
            format!(
                "admitted at step {admitted}, appeared {delay} steps later: {}",
                spawn.trim()
            ),
        );
        let entity: u64 = field(&spawn, "entity")
            .and_then(|v| v.parse().ok())
            .ok_or("spawn line has no entity")?;
        client_a.wait(
            &["MP_CLIENT_UNIT_RELEVANT", &format!("entity={entity} ")],
            Duration::from_secs(10),
        )?;
        thread::sleep(Duration::from_millis(600));
        report.artifact(
            "client_a_recovered_png",
            &client_a.screenshot(&evidence, "04-recovered")?,
        );

        let (again, _) = economy_key_outcome(
            &mut client_a,
            &mut server,
            None,
            &["RTS_ECONOMY_TX participant=1 kind=train"],
        )?;
        report.verdict(
            "PWR-P08-TRAINING-WORKS-AFTER-RECOVERY",
            econ_int(&again, "amount") == ECONOMY_PRICE_CENTS
                && econ_int(&again, "queued") == 1
                && field(&again, "reconciles") == Some("1"),
            again.trim(),
        );

        // The outage touched only this seat, moved no money and reached only its owner.
        let text = server.text();
        let enemy_changes = text
            .lines()
            .filter(|l| l.contains("RTS_POWER_STATE participant=2"))
            .count();
        let (text_a, text_b) = (client_a.text(), client_b.text());
        let a_leaks = text_a
            .lines()
            .filter(|l| l.contains("RTS_ECONOMY_VIEW participant=2"))
            .count();
        let b_leaks = text_b
            .lines()
            .filter(|l| l.contains("RTS_ECONOMY_VIEW participant=1"))
            .count();
        let b_offline = text_b
            .lines()
            .filter(|l| l.contains("RTS_ECONOMY_VIEW") && l.contains("powered=0"))
            .count();
        let a_back = text_a
            .lines()
            .rev()
            .find(|l| l.contains("RTS_ECONOMY_VIEW participant=1"))
            .unwrap_or_default();
        report.verdict(
            "PWR-P09-OWNER-ONLY-OUTAGE",
            enemy_changes == 0
                && a_leaks == 0
                && b_leaks == 0
                && b_offline == 0
                && econ_int(&offline_view, "supplied_kw") == 0
                && econ_int(a_back, "supplied_kw") == POWER_SUPPLIED_KW
                && field(a_back, "powered") == Some("1"),
            format!(
                "enemy power changes {enemy_changes}; A's copies of B's view {a_leaks}, B's copies of A's view {b_leaks}, B offline views {b_offline}; {} | {}",
                offline_view.trim(),
                a_back.trim()
            ),
        );
        let lines_1 = economy_lines(&text, 1, &generation);
        let replay = economy_replay(&lines_1, ECONOMY_START_CENTS);
        let cadence = economy_income_cadence(&lines_1);
        let trains = lines_1
            .iter()
            .filter(|l| field(l, "kind") == Some("train"))
            .count();
        let refunds = lines_1
            .iter()
            .filter(|l| field(l, "kind") == Some("refund"))
            .count();
        report.verdict(
            "PWR-P10-NO-SECOND-CHARGE",
            replay.is_ok() && cadence.is_ok() && trains == 3 && refunds == 1,
            format!(
                "{trains} purchases and {refunds} refund; replay {replay:?}; income through the outage {cadence:?}"
            ),
        );

        // Repeatability: the normalized sequence, without timing or income interleaving.
        let mut trace: String = lines_1
            .iter()
            .filter(|l| field(l, "kind") != Some("income"))
            .map(|l| {
                format!(
                    "{} {} queued={}\n",
                    field(l, "kind").unwrap_or("?"),
                    econ_int(l, "amount"),
                    econ_int(l, "queued")
                )
            })
            .collect();
        trace.push_str(&format!(
            "off_offset={} head_remaining={off_remaining} on_offset={} delay={delay}\n",
            off_step - admitted,
            econ_int(&on, "step") - admitted
        ));
        fs::write(evidence.join("power-trace.txt"), &trace).map_err(|e| e.to_string())?;
        report.artifact("power_trace", &evidence.join("power-trace.txt"));

        // A match end, then a next generation that owes nothing to the outage.
        let owned: Vec<u64> = entity_owners(&server.text())
            .iter()
            .filter(|(_, owner)| *owner == 1)
            .map(|(id, _)| *id)
            .collect();
        let unit = *owned.first().ok_or("no authored unit for participant 1")?;
        select_entity(&mut client_a, "MP_CLIENT_SAMPLE", unit)?;
        thread::sleep(Duration::from_millis(600));
        let objective = wait_screen(
            &mut client_a,
            |t| objective_screen(t, "MP_CLIENT_OBJECTIVE"),
            "objective on client A",
        )?;
        let at_move = server.offset();
        client_a.click(objective.0, objective.1, "3")?;
        server.wait_line(
            &[
                "RTS_ORDER_ADMITTED order=Move",
                &format!("unit={unit}"),
                "participant=1",
            ],
            Duration::from_secs(5),
            at_move,
        )?;
        server.wait(
            &[
                "MATCH_RESULTS outcome=objective_secured",
                &format!("generation={generation}"),
            ],
            Duration::from_secs(180),
        )?;
        let cleared = server.wait(
            &["RTS_ECONOMY_CLEARED participant=1", "reason=match_ended"],
            Duration::from_secs(5),
        )?;
        let at_reset = server.offset();
        client_a.hover(640.0, 360.0)?;
        client_a.key("Escape")?;
        client_b.stop();
        server.wait_line(
            &["MP_DEDICATED_SESSION_RESET", "travel=1"],
            Duration::from_secs(60),
            at_reset,
        )?;
        let next = server.wait_line(
            &["MP_MATCH_AWAITING_PARTICIPANTS"],
            Duration::from_secs(90),
            at_reset,
        )?;
        let generation2 = field(&next, "generation").unwrap_or("0").to_owned();
        let text = server.text();
        let tail = &text[at_reset.min(text.len())..];
        let starts2: Vec<&str> = tail
            .lines()
            .filter(|l| {
                l.contains("RTS_POWER_START") && l.contains(&format!("generation={generation2}"))
            })
            .collect();
        report.verdict(
            "PWR-P11-NEXT-GENERATION-FRESH",
            generation2 != generation
                && field(&cleared, "reconciles") == Some("1")
                && starts2.len() == 2
                && starts2.iter().all(|l| {
                    econ_int(l, "supplied_kw") == POWER_SUPPLIED_KW
                        && field(l, "producer_powered") == Some("1")
                })
                && !tail.lines().any(|l| l.contains("RTS_POWER_STATE")),
            format!(
                "generation {generation} then {generation2}: {} | {}",
                cleared.trim(),
                starts2.join(" | ")
            ),
        );
        Ok(())
    })();
    launch_options_verdict(
        report,
        "PWR-P00-LAUNCH-OPTIONS",
        &[&server, &client_a, &client_b],
    );
    client_a.stop();
    client_b.stop();
    server.stop();
    result
}

// `--gold SERVER CLIENT [--evidence DIR] [--port N]` (ECON-E4) plays the whole route with real
// input. Seat 1's source holds a load and a part load. K trains a harvester, which drives its first
// trip with no further input. L sells the lot. A right-click parks the harvester, a right-click on
// the cache sends it back to work, and it empties the source and stops. L sells what is left, K is
// refused, and then a match end and the next generation.
const GOLD_FIXTURE_SOURCE: i64 = 40;
const GOLD_NORMAL_SOURCE: i64 = 500;
const GOLD_LOAD: i64 = 25;
const GOLD_LAST_LOAD: i64 = 15;
const GOLD_HARVESTER_CENTS: i64 = 30_000;
const GOLD_HARVESTER_STEPS: i64 = 300;
const GOLD_LOAD_STEPS: i64 = 60;
const GOLD_UNLOAD_STEPS: i64 = 40;
// Seat 1's source and producer on the multiplayer test map, and open ground between them. Seat 2
// mirrors the first two.
const GOLD_SOURCE_AT: (f64, f64) = (-826.0, -1580.4);
const GOLD_DEPOT_AT: (f64, f64) = (-1740.2, -937.0);
const GOLD_PARK_AT: (f64, f64) = (-1200.0, -1000.0);
const GOLD_PICKUP_RADIUS_CM: f64 = 250.0;
const GOLD_DEPOT_RADIUS_CM: f64 = 350.0;
const GOLD_HARVESTER_CM_PER_STEP: f64 = 280.0 * 0.05;

/// A participant's gold lines in one generation: every line follows from the last, and all of the
/// source's gold is accounted for. Returns the number of lines.
fn harvest_replay(
    text: &str,
    participant: u32,
    generation: &str,
    resource: &str,
    start: i64,
) -> Result<usize, String> {
    let tag = format!("RTS_HARVEST_TX participant={participant} ");
    let generation = format!("generation={generation} ");
    let wanted = format!("resource={resource} ");
    // The ration reserve also moves with upkeep and purchases; food_replay follows it.
    let gold = resource == "gold";
    let (mut source, mut carried, mut held, mut delivered, mut sold, mut lost) =
        (start, 0_i64, 0_i64, 0_i64, 0_i64, 0_i64);
    let mut count = 0;
    for line in text
        .lines()
        .filter(|l| l.contains(&tag) && l.contains(&generation) && l.contains(&wanted))
    {
        let amount = econ_int(line, "amount");
        match field(line, "kind") {
            Some("load") => {
                source -= amount;
                carried += amount;
            }
            Some("deliver") => {
                carried -= amount;
                held += amount;
                delivered += amount;
            }
            Some("sale") => {
                held -= amount;
                sold += amount;
                if econ_int(line, "gross") - econ_int(line, "fee") <= 0 {
                    return Err(format!("a sale paid nothing: {}", line.trim()));
                }
            }
            Some("lost") => {
                carried -= amount;
                lost += amount;
            }
            Some("harvester") | Some("loading") | Some("detached") | Some("waiting")
            | Some("resumed") | Some("retarget") => {
                if amount != 0 {
                    return Err(format!("this kind moves no gold: {}", line.trim()));
                }
            }
            other => return Err(format!("unknown kind {other:?}: {}", line.trim())),
        }
        let expected = [
            ("source", source),
            ("carried", carried),
            ("held", held),
            ("delivered", delivered),
            ("sold", sold),
            ("lost", lost),
        ];
        if expected
            .iter()
            .filter(|(key, _)| gold || !matches!(*key, "held" | "sold"))
            .any(|(key, value)| econ_int(line, key) != *value)
            || field(line, "reconciles") != Some("1")
            || source < 0
            || carried < 0
            || held < 0
        {
            return Err(format!("expected {expected:?}: {}", line.trim()));
        }
        count += 1;
    }
    Ok(count)
}

/// The latest position a rendered client has sampled for an entity.
fn client_unit_position(text: &str, entity: u64) -> Option<(f64, f64)> {
    text.lines()
        .rev()
        .filter(|l| l.contains("MP_CLIENT_SAMPLE"))
        .find(|l| field(l, "entity").and_then(|v| v.parse::<u64>().ok()) == Some(entity))
        .and_then(|l| Some((number(l, "x")?, number(l, "y")?)))
}

fn gold_session(options: &DedicatedOptions, report: &mut Report) -> Result<(), String> {
    let evidence = options.client.evidence.clone();
    let (mut server, mut client_a, mut client_b, _display, generation) =
        economy_start_session_with(
            options,
            &[
                format!("-PolitikaMPEconomySourceGold=1:{GOLD_FIXTURE_SOURCE}"),
                "-PolitikaMPEconomySourceFood=1:0".to_owned(),
            ],
            &[format!(
                "-PolitikaMPGroundProbe={}:{}/{}:{}/{}:{}",
                GOLD_SOURCE_AT.0,
                GOLD_SOURCE_AT.1,
                GOLD_DEPOT_AT.0,
                GOLD_DEPOT_AT.1,
                GOLD_PARK_AT.0,
                GOLD_PARK_AT.1
            )],
        )?;
    report.fact("server_command", server.command.join(" "));
    report.fact("client_a_command", client_a.command.join(" "));
    report.fact("client_b_command", client_b.command.join(" "));

    let result = (|| -> Result<(), String> {
        let text = server.text();
        let starts: Vec<&str> = text
            .lines()
            .filter(|l| {
                l.contains("RTS_GOLD_START") && l.contains(&format!("generation={generation}"))
            })
            .collect();
        let at = |line: &str, x: f64, y: f64| {
            (number(line, "x").unwrap_or(f64::MAX) - x).abs() < 1.0
                && (number(line, "y").unwrap_or(f64::MAX) - y).abs() < 1.0
        };
        report.verdict(
            "GOLD-P01-SOURCES-AT-THE-START",
            starts.len() == 2
                && field(starts[0], "participant") == Some("1")
                && econ_int(starts[0], "source") == GOLD_FIXTURE_SOURCE
                && at(starts[0], GOLD_SOURCE_AT.0, GOLD_SOURCE_AT.1)
                && field(starts[1], "participant") == Some("2")
                && econ_int(starts[1], "source") == GOLD_NORMAL_SOURCE
                && at(starts[1], -GOLD_SOURCE_AT.0, -GOLD_SOURCE_AT.1),
            starts.join(" | "),
        );
        let sources = |t: &str| -> Vec<String> {
            t.lines()
                .filter(|l| l.contains("RTS_GOLD_SOURCE_RELEVANT"))
                .filter_map(|l| field(l, "owner").map(str::to_owned))
                .collect()
        };
        client_a.wait(&["RTS_GOLD_SOURCE_RELEVANT"], Duration::from_secs(10))?;
        client_b.wait(&["RTS_GOLD_SOURCE_RELEVANT"], Duration::from_secs(10))?;
        let (text_a, text_b) = (client_a.text(), client_b.text());
        report.verdict(
            "GOLD-P02-SOURCE-OWN-TEAM-ONLY",
            sources(&text_a) == ["1"] && sources(&text_b) == ["2"],
            format!(
                "client A received {:?}, client B received {:?}",
                sources(&text_a),
                sources(&text_b)
            ),
        );
        zoom_out(&client_a, 5)?;
        thread::sleep(Duration::from_millis(800));
        report.artifact(
            "client_a_start_png",
            &client_a.screenshot(&evidence, "01-start")?,
        );

        // Nothing to sell yet.
        let (client_mark, server_mark) = (client_a.offset(), server.offset());
        client_a.key("l")?;
        let refused = server.wait_line(
            &["RTS_ECONOMY_REJECT participant=1", "op=3"],
            Duration::from_secs(5),
            server_mark,
        )?;
        let refused_client = client_a.wait_line(
            &["RTS_ECONOMY_OUTCOME participant=1", "op=3"],
            Duration::from_secs(5),
            client_mark,
        )?;
        report.verdict(
            "GOLD-P03-NOTHING-TO-SELL-YET",
            field(&refused, "rejection") == Some("nothing_to_sell")
                && field(&refused_client, "accepted") == Some("0")
                && econ_int(&refused_client, "amount") == 0
                && !server.text().contains("RTS_HARVEST_TX participant=1"),
            format!("{} | client {}", refused.trim(), refused_client.trim()),
        );

        // K trains a harvester; nothing else is pressed until it has delivered.
        let (client_mark, server_mark) = (client_a.offset(), server.offset());
        client_a.key("k")?;
        let bought = server.wait_line(
            &["RTS_ECONOMY_TX participant=1 kind=train"],
            Duration::from_secs(5),
            server_mark,
        )?;
        client_a.wait_line(
            &["RTS_ECONOMY_OUTCOME participant=1", "op=0", "accepted=1"],
            Duration::from_secs(5),
            client_mark,
        )?;
        let spawned = server.wait(
            &["RTS_PRODUCTION_SPAWN participant=1", "recipe=1"],
            Duration::from_secs(30),
        )?;
        let harvester = econ_int(&spawned, "entity") as u64;
        report.verdict(
            "GOLD-P04-K-TRAINS-A-HARVESTER",
            econ_int(&bought, "amount") == GOLD_HARVESTER_CENTS
                && field(&bought, "reconciles") == Some("1")
                && econ_int(&spawned, "step") - econ_int(&bought, "step") == GOLD_HARVESTER_STEPS,
            format!("{} | {}", bought.trim(), spawned.trim()),
        );
        let unit_tag = format!("unit={harvester} ");

        // Its first trip, with no further input.
        let loaded = server.wait(
            &["RTS_HARVEST_TX participant=1 kind=load ", &unit_tag],
            Duration::from_secs(30),
        )?;
        let loaded_view = client_a.wait(
            &[
                "RTS_ECONOMY_VIEW participant=1",
                &format!("gold_carried={GOLD_LOAD} "),
            ],
            Duration::from_secs(5),
        );
        thread::sleep(Duration::from_millis(500));
        report.artifact(
            "client_a_carrying_png",
            &client_a.screenshot(&evidence, "02-carrying")?,
        );
        let delivered = server.wait(
            &["RTS_HARVEST_TX participant=1 kind=deliver", &unit_tag],
            Duration::from_secs(30),
        )?;
        let delivered_view = client_a.wait(
            &[
                "RTS_ECONOMY_VIEW participant=1",
                &format!("gold_held={GOLD_LOAD} "),
                "gold_carried=0 ",
            ],
            Duration::from_secs(5),
        );
        // It really drove: from the output to the source and back, at its speed, with the full
        // loading and unloading times.
        let gap = ((GOLD_SOURCE_AT.0 - GOLD_DEPOT_AT.0).powi(2)
            + (GOLD_SOURCE_AT.1 - GOLD_DEPOT_AT.1).powi(2))
        .sqrt()
            - GOLD_PICKUP_RADIUS_CM
            - GOLD_DEPOT_RADIUS_CM;
        let home = econ_int(&delivered, "step") - econ_int(&loaded, "step");
        let least_home = (gap / GOLD_HARVESTER_CM_PER_STEP) as i64 + GOLD_UNLOAD_STEPS;
        report.verdict(
            "GOLD-P05-THE-FIRST-TRIP-DRIVES-ITSELF",
            econ_int(&loaded, "amount") == GOLD_LOAD
                && econ_int(&loaded, "source") == GOLD_FIXTURE_SOURCE - GOLD_LOAD
                && econ_int(&loaded, "held") == 0
                && econ_int(&loaded, "step") - econ_int(&spawned, "step") >= GOLD_LOAD_STEPS
                && econ_int(&delivered, "amount") == GOLD_LOAD
                && econ_int(&delivered, "held") == GOLD_LOAD
                && econ_int(&delivered, "carried") == 0
                && home >= least_home
                && loaded_view.is_ok()
                && delivered_view.is_ok(),
            format!(
                "{home} steps home, at least {least_home} | {} | {} | views {loaded_view:?} {delivered_view:?}",
                loaded.trim(),
                delivered.trim()
            ),
        );

        // L sells the lot, at the quote.
        let sell =
            |client: &mut Game, server: &mut Game| -> Result<(String, String, String), String> {
                let (client_mark, server_mark) = (client.offset(), server.offset());
                client.key("l")?;
                let money = server.wait_line(
                    &["RTS_ECONOMY_TX participant=1 kind=sale"],
                    Duration::from_secs(5),
                    server_mark,
                )?;
                let gold = server.wait_line(
                    &["RTS_HARVEST_TX participant=1 kind=sale"],
                    Duration::from_secs(5),
                    server_mark,
                )?;
                let outcome = client.wait_line(
                    &["RTS_ECONOMY_OUTCOME participant=1", "op=3", "accepted=1"],
                    Duration::from_secs(5),
                    client_mark,
                )?;
                Ok((money, gold, outcome))
            };
        let (money_1, gold_1, outcome_1) = sell(&mut client_a, &mut server)?;
        report.verdict(
            "GOLD-P06-A-LOT-AT-THE-QUOTE",
            econ_int(&gold_1, "amount") == 25
                && econ_int(&gold_1, "gross") == 6_250
                && econ_int(&gold_1, "fee") == 188
                && econ_int(&money_1, "amount") == 6_062
                && econ_int(&gold_1, "held") == 0
                && econ_int(&outcome_1, "amount") == 6_062
                && econ_int(&outcome_1, "gold_sold") == 25,
            format!(
                "{} | {} | client {}",
                money_1.trim(),
                gold_1.trim(),
                outcome_1.trim()
            ),
        );
        report.artifact(
            "client_a_sold_png",
            &client_a.screenshot(&evidence, "03-sold")?,
        );

        // A right-click parks it during its next load, before any gold has moved, and it stays there.
        // It is selected while it stands at the source, because a sampled position trails a moving unit.
        let park_screen = wait_screen(
            &mut client_a,
            |t| probe_screen(t, 2),
            "the parking point on client A",
        )?;
        let source_screen = wait_screen(
            &mut client_a,
            |t| probe_screen(t, 0),
            "the gold source on client A",
        )?;
        let server_mark = server.offset();
        server.wait(
            &[
                "RTS_HARVEST_TX participant=1 kind=loading",
                &unit_tag,
                "source=15 ",
            ],
            Duration::from_secs(20),
        )?;
        thread::sleep(Duration::from_millis(900));
        let mut selected = false;
        for (dx, dy) in [(0.0, 0.0), (0.0, -8.0), (0.0, -16.0), (4.0, -4.0)] {
            let mark = client_a.offset();
            client_a.click(source_screen.0 + dx, source_screen.1 + dy, "1")?;
            if client_a
                .wait_line(
                    &["RTS_SELECTION player=", &format!("unit={harvester}")],
                    Duration::from_millis(400),
                    mark,
                )
                .is_ok()
            {
                selected = true;
                break;
            }
        }
        if !selected {
            return Err("could not select the harvester at the source".to_owned());
        }
        let move_mark = server.offset();
        client_a.click(park_screen.0, park_screen.1, "3")?;
        server.wait_line(
            &[
                "RTS_ORDER_ADMITTED order=Move",
                &format!("unit={harvester}"),
                "participant=1",
            ],
            Duration::from_secs(5),
            move_mark,
        )?;
        let detached = server.wait_line(
            &["RTS_HARVEST_TX participant=1 kind=detached", &unit_tag],
            Duration::from_secs(5),
            server_mark,
        )?;
        let start = Instant::now();
        let mut parked = false;
        while start.elapsed() < Duration::from_secs(30) && !parked {
            parked = client_unit_position(&client_a.text(), harvester).is_some_and(|(x, y)| {
                ((x - GOLD_PARK_AT.0).powi(2) + (y - GOLD_PARK_AT.1).powi(2)).sqrt() < 60.0
            });
            thread::sleep(Duration::from_millis(250));
        }
        thread::sleep(Duration::from_secs(4));
        let text = server.text();
        let while_parked: Vec<&str> = text[server_mark.min(text.len())..]
            .lines()
            .filter(|l| l.contains("RTS_HARVEST_TX participant=1"))
            .filter(|l| !matches!(field(l, "kind"), Some("detached") | Some("loading")))
            .collect();
        report.verdict(
            "GOLD-P07-A-PLAYER-ORDER-PARKS-IT",
            econ_int(&detached, "source") == GOLD_LAST_LOAD && parked && while_parked.is_empty(),
            format!(
                "parked {parked}, {} gold lines while parked | {}",
                while_parked.len(),
                detached.trim()
            ),
        );
        report.artifact(
            "client_a_parked_png",
            &client_a.screenshot(&evidence, "04-parked")?,
        );

        // A right-click on the cache sends it back to work. The last load is what is left, and the
        // routine ends by itself with the source empty.
        let server_mark = server.offset();
        order_move(&mut client_a, &mut server, harvester, source_screen)?;
        let resumed = server.wait_line(
            &["RTS_HARVEST_TX participant=1 kind=resumed", &unit_tag],
            Duration::from_secs(30),
            server_mark,
        )?;
        let last = server.wait_line(
            &["RTS_HARVEST_TX participant=1 kind=load ", &unit_tag],
            Duration::from_secs(20),
            server_mark,
        )?;
        let last_home = server.wait_line(
            &["RTS_HARVEST_TX participant=1 kind=deliver", &unit_tag],
            Duration::from_secs(30),
            server_mark,
        )?;
        let stopped = server.wait_line(
            &["RTS_HARVEST_TX participant=1 kind=waiting", &unit_tag],
            Duration::from_secs(5),
            server_mark,
        )?;
        report.verdict(
            "GOLD-P08-THE-CACHE-SENDS-IT-BACK",
            econ_int(&resumed, "source") == GOLD_LAST_LOAD
                && econ_int(&last, "amount") == GOLD_LAST_LOAD
                && econ_int(&last, "source") == 0
                && econ_int(&last, "step") - econ_int(&resumed, "step") == GOLD_LOAD_STEPS
                && econ_int(&last_home, "amount") == GOLD_LAST_LOAD
                && econ_int(&last_home, "held") == GOLD_LAST_LOAD
                && econ_int(&stopped, "step") == econ_int(&last_home, "step"),
            format!(
                "{} | {} | {} | {}",
                resumed.trim(),
                last.trim(),
                last_home.trim(),
                stopped.trim()
            ),
        );
        thread::sleep(Duration::from_millis(600));
        report.artifact(
            "client_a_empty_png",
            &client_a.screenshot(&evidence, "05-source-empty")?,
        );

        // Less than a lot sells what is left, then nothing, and an empty cache refuses K.
        let (money_2, gold_2, outcome_2) = sell(&mut client_a, &mut server)?;
        let (client_mark, server_mark) = (client_a.offset(), server.offset());
        client_a.key("l")?;
        let nothing = server.wait_line(
            &["RTS_ECONOMY_REJECT participant=1", "op=3"],
            Duration::from_secs(5),
            server_mark,
        )?;
        client_a.wait_line(
            &["RTS_ECONOMY_OUTCOME participant=1", "op=3", "accepted=0"],
            Duration::from_secs(5),
            client_mark,
        )?;
        let (client_mark, server_mark) = (client_a.offset(), server.offset());
        client_a.key("k")?;
        let no_source = server.wait_line(
            &["RTS_ECONOMY_REJECT participant=1", "op=0"],
            Duration::from_secs(5),
            server_mark,
        )?;
        client_a.wait_line(
            &["RTS_ECONOMY_OUTCOME participant=1", "op=0", "accepted=0"],
            Duration::from_secs(5),
            client_mark,
        )?;
        report.verdict(
            "GOLD-P09-WHAT-IS-LEFT-THEN-NOTHING",
            econ_int(&gold_2, "amount") == GOLD_LAST_LOAD
                && econ_int(&gold_2, "gross") == 3_750
                && econ_int(&gold_2, "fee") == 113
                && econ_int(&money_2, "amount") == 3_637
                && econ_int(&gold_2, "held") == 0
                && econ_int(&outcome_2, "gold_sold") == GOLD_LAST_LOAD
                && field(&nothing, "rejection") == Some("nothing_to_sell")
                && field(&no_source, "rejection") == Some("no_resource_source"),
            format!(
                "{} | {} | {} | {}",
                money_2.trim(),
                gold_2.trim(),
                nothing.trim(),
                no_source.trim()
            ),
        );

        // Only seat 1 moved, and each client saw only its own gold.
        let text = server.text();
        let (text_a, text_b) = (client_a.text(), client_b.text());
        let enemy_lines = text
            .lines()
            .filter(|l| l.contains("RTS_HARVEST_TX participant=2"))
            .count();
        let a_leaks = text_a
            .lines()
            .filter(|l| l.contains("RTS_ECONOMY_VIEW participant=2"))
            .count();
        let b_leaks = text_b
            .lines()
            .filter(|l| l.contains("RTS_ECONOMY_VIEW participant=1"))
            .count();
        let b_gold = text_b
            .lines()
            .filter(|l| {
                l.contains("RTS_ECONOMY_VIEW")
                    && (econ_int(l, "gold_held") != 0
                        || econ_int(l, "gold_carried") != 0
                        || econ_int(l, "harvesters") != 0)
            })
            .count();
        report.verdict(
            "GOLD-P10-OWNER-ONLY",
            enemy_lines == 0 && a_leaks == 0 && b_leaks == 0 && b_gold == 0,
            format!("enemy gold lines {enemy_lines}; A's copies of B's view {a_leaks}, B's copies of A's view {b_leaks}, B gold views {b_gold}"),
        );
        let replay = harvest_replay(&text, 1, &generation, "gold", GOLD_FIXTURE_SOURCE);
        report.verdict(
            "GOLD-P11-GOLD-REPLAYS",
            replay == Ok(12),
            format!("{replay:?}: harvester, loading, load, deliver, sale, loading, detached, resumed, load, deliver, waiting, sale"),
        );
        let lines_1 = economy_lines(&text, 1, &generation);
        let money = economy_replay(&lines_1, ECONOMY_START_CENTS);
        let sales = lines_1
            .iter()
            .filter(|l| field(l, "kind") == Some("sale"))
            .count();
        report.verdict(
            "GOLD-P12-MONEY-REPLAYS",
            money.is_ok() && sales == 2,
            format!("{} money lines, {sales} sales; {money:?}", lines_1.len()),
        );

        // Repeatability: the normalized sequence, without timing or income interleaving.
        let mut trace = String::new();
        for line in text.lines().filter(|l| {
            l.contains("RTS_HARVEST_TX participant=1")
                && l.contains(&format!("generation={generation} "))
        }) {
            trace.push_str(&format!(
                "gold {} amount={} gross={} fee={} source={} carried={} held={} delivered={} sold={}\n",
                field(line, "kind").unwrap_or("?"),
                econ_int(line, "amount"),
                econ_int(line, "gross"),
                econ_int(line, "fee"),
                econ_int(line, "source"),
                econ_int(line, "carried"),
                econ_int(line, "held"),
                econ_int(line, "delivered"),
                econ_int(line, "sold"),
            ));
        }
        for line in lines_1
            .iter()
            .filter(|l| field(l, "kind") != Some("income"))
        {
            trace.push_str(&format!(
                "money {} amount={} queued={}\n",
                field(line, "kind").unwrap_or("?"),
                econ_int(line, "amount"),
                econ_int(line, "queued"),
            ));
        }
        fs::write(evidence.join("gold-trace.txt"), &trace).map_err(|e| e.to_string())?;
        report.artifact("gold_trace", &evidence.join("gold-trace.txt"));

        // A match end with a harvester alive, taken by a soldier, then a next generation that owes
        // nothing to this one.
        let owned: Vec<u64> = entity_owners(&server.text())
            .iter()
            .filter(|(id, owner)| *owner == 1 && *id != harvester)
            .map(|(id, _)| *id)
            .collect();
        let unit = *owned.first().ok_or("no authored unit for participant 1")?;
        select_entity(&mut client_a, "MP_CLIENT_SAMPLE", unit)?;
        thread::sleep(Duration::from_millis(600));
        let objective = wait_screen(
            &mut client_a,
            |t| objective_screen(t, "MP_CLIENT_OBJECTIVE"),
            "objective on client A",
        )?;
        let at_move = server.offset();
        client_a.click(objective.0, objective.1, "3")?;
        server.wait_line(
            &[
                "RTS_ORDER_ADMITTED order=Move",
                &format!("unit={unit}"),
                "participant=1",
            ],
            Duration::from_secs(5),
            at_move,
        )?;
        server.wait(
            &[
                "MATCH_RESULTS outcome=objective_secured",
                &format!("generation={generation}"),
            ],
            Duration::from_secs(180),
        )?;
        let cleared = server.wait(
            &["RTS_ECONOMY_CLEARED participant=1", "reason=match_ended"],
            Duration::from_secs(5),
        )?;
        let at_reset = server.offset();
        client_a.hover(640.0, 360.0)?;
        client_a.key("Escape")?;
        client_b.stop();
        server.wait_line(
            &["MP_DEDICATED_SESSION_RESET", "travel=1"],
            Duration::from_secs(60),
            at_reset,
        )?;
        let next = server.wait_line(
            &["MP_MATCH_AWAITING_PARTICIPANTS"],
            Duration::from_secs(90),
            at_reset,
        )?;
        let generation2 = field(&next, "generation").unwrap_or("0").to_owned();
        let text = server.text();
        let tail = &text[at_reset.min(text.len())..];
        let starts2: Vec<&str> = tail
            .lines()
            .filter(|l| {
                l.contains("RTS_GOLD_START") && l.contains(&format!("generation={generation2}"))
            })
            .collect();
        report.verdict(
            "GOLD-P13-NEXT-GENERATION-FRESH",
            generation2 != generation
                && field(&cleared, "reconciles") == Some("1")
                && field(&cleared, "gold_reconciles") == Some("1")
                && econ_int(&cleared, "sale_credited") == 6_062 + 3_637
                && starts2.len() == 2
                && econ_int(starts2[0], "source") == GOLD_FIXTURE_SOURCE
                && econ_int(starts2[1], "source") == GOLD_NORMAL_SOURCE
                && !tail.lines().any(|l| l.contains("RTS_HARVEST_TX")),
            format!(
                "generation {generation} then {generation2}: {} | {}",
                cleared.trim(),
                starts2.join(" | ")
            ),
        );
        Ok(())
    })();
    launch_options_verdict(
        report,
        "GOLD-P00-LAUNCH-OPTIONS",
        &[&server, &client_a, &client_b],
    );
    client_a.stop();
    client_b.stop();
    server.stop();
    result
}

// `--harvest SERVER CLIENT [--evidence DIR] [--port N]` (ECON-E5) plays the normal start with real
// input. Each seat has a free harvester and two caches. It hauls gold with no input at all, a trained
// rifleman's and the harvester's owner-only fields reach their owner, a right-click sends it to the
// food cache, the food goes into the reserve until seat 1's small cache is empty, and it moves on to
// the gold by itself. A box over the base takes only soldiers. Then a match end and the next generation.
const HARVEST_FOOD_FIXTURE: i64 = 60;
const HARVEST_FOOD_NORMAL: i64 = 600;
const HARVEST_FOOD_LOAD: i64 = 30;
const HARVEST_FOOD_AT: (f64, f64) = (-1774.1, 180.5);

fn harvest_session(options: &DedicatedOptions, report: &mut Report) -> Result<(), String> {
    let evidence = options.client.evidence.clone();
    let (mut server, mut client_a, mut client_b, _display, generation) =
        economy_start_session_with(
            options,
            &[
                STARTING_HARVESTERS.to_owned(),
                format!("-PolitikaMPEconomySourceFood=1:{HARVEST_FOOD_FIXTURE}"),
            ],
            &[format!(
                "-PolitikaMPGroundProbe={}:{}/{}:{}",
                GOLD_SOURCE_AT.0, GOLD_SOURCE_AT.1, HARVEST_FOOD_AT.0, HARVEST_FOOD_AT.1
            )],
        )?;
    report.fact("server_command", server.command.join(" "));
    report.fact("client_a_command", client_a.command.join(" "));
    report.fact("client_b_command", client_b.command.join(" "));

    let result = (|| -> Result<(), String> {
        let gen_tag = format!("generation={generation}");
        let text = server.text();
        let lines_of = |tag: &str| -> Vec<String> {
            text.lines()
                .filter(|l| l.contains(tag) && l.contains(&gen_tag))
                .map(str::to_owned)
                .collect()
        };
        let at = |line: &str, x: f64, y: f64| {
            (number(line, "x").unwrap_or(f64::MAX) - x).abs() < 1.0
                && (number(line, "y").unwrap_or(f64::MAX) - y).abs() < 1.0
        };
        let (gold, food, free, money) = (
            lines_of("RTS_GOLD_START"),
            lines_of("RTS_FOOD_SOURCE_START"),
            lines_of("RTS_HARVESTER_START"),
            lines_of("RTS_ECONOMY_START"),
        );
        report.verdict(
            "HARV-P01-CACHES-AND-A-FREE-HARVESTER",
            gold.len() == 2
                && food.len() == 2
                && free.len() == 2
                && field(&free[0], "participant") == Some("1")
                && field(&free[1], "participant") == Some("2")
                && econ_int(&gold[0], "source") == GOLD_NORMAL_SOURCE
                && econ_int(&food[0], "source") == HARVEST_FOOD_FIXTURE
                && econ_int(&food[1], "source") == HARVEST_FOOD_NORMAL
                && at(&gold[0], GOLD_SOURCE_AT.0, GOLD_SOURCE_AT.1)
                && at(&food[0], HARVEST_FOOD_AT.0, HARVEST_FOOD_AT.1)
                && at(&food[1], -HARVEST_FOOD_AT.0, -HARVEST_FOOD_AT.1)
                && money
                    .iter()
                    .all(|l| econ_int(l, "available") == ECONOMY_START_CENTS)
                && !text.contains("kind=train"),
            format!(
                "{} | {} | {}",
                gold.join(" | "),
                food.join(" | "),
                free.join(" | ")
            ),
        );
        let harvester = econ_int(&free[0], "entity") as u64;
        let enemy_harvester = econ_int(&free[1], "entity") as u64;
        let unit_tag = format!("unit={harvester} ");

        for tag in ["RTS_FOOD_SOURCE_RELEVANT", "RTS_GOLD_SOURCE_RELEVANT"] {
            client_a.wait(&[tag], Duration::from_secs(10))?;
            client_b.wait(&[tag], Duration::from_secs(10))?;
        }
        let owners = |t: &str, tag: &str| -> Vec<String> {
            t.lines()
                .filter(|l| l.contains(tag))
                .filter_map(|l| field(l, "owner").map(str::to_owned))
                .collect()
        };
        let (text_a, text_b) = (client_a.text(), client_b.text());
        report.verdict(
            "HARV-P02-CACHES-OWN-TEAM-ONLY",
            owners(&text_a, "RTS_FOOD_SOURCE_RELEVANT") == ["1"]
                && owners(&text_b, "RTS_FOOD_SOURCE_RELEVANT") == ["2"]
                && owners(&text_a, "RTS_GOLD_SOURCE_RELEVANT") == ["1"]
                && owners(&text_b, "RTS_GOLD_SOURCE_RELEVANT") == ["2"],
            format!(
                "client A food {:?}, client B food {:?}",
                owners(&text_a, "RTS_FOOD_SOURCE_RELEVANT"),
                owners(&text_b, "RTS_FOOD_SOURCE_RELEVANT")
            ),
        );
        zoom_out(&client_a, 5)?;
        thread::sleep(Duration::from_millis(800));
        report.artifact(
            "client_a_start_png",
            &client_a.screenshot(&evidence, "01-start")?,
        );

        // Its first trip with no input at all, gold by default.
        let loaded = server.wait(
            &["RTS_HARVEST_TX participant=1 kind=load ", &unit_tag],
            Duration::from_secs(40),
        )?;
        let carried_client = client_a.wait(
            &[
                "RTS_HARVESTER_CARRY_CLIENT",
                &format!("entity={harvester} "),
                "carried=1 ",
            ],
            Duration::from_secs(5),
        );
        thread::sleep(Duration::from_millis(500));
        report.artifact(
            "client_a_carrying_png",
            &client_a.screenshot(&evidence, "02-carrying")?,
        );
        let delivered = server.wait(
            &["RTS_HARVEST_TX participant=1 kind=deliver ", &unit_tag],
            Duration::from_secs(30),
        )?;
        report.verdict(
            "HARV-P03-IT-WORKS-WITH-NO-INPUT",
            field(&loaded, "resource") == Some("gold")
                && econ_int(&loaded, "amount") == GOLD_LOAD
                && field(&delivered, "resource") == Some("gold")
                && econ_int(&delivered, "held") == GOLD_LOAD,
            format!("{} | {}", loaded.trim(), delivered.trim()),
        );

        // The owner fix: the owner receives a harvester's load and category, and a trained
        // rifleman's ammunition, none of which reached it before.
        let (bought, _) = economy_key_outcome(
            &mut client_a,
            &mut server,
            None,
            &["RTS_ECONOMY_TX participant=1 kind=train"],
        )?;
        let spawned = server.wait(
            &["RTS_PRODUCTION_SPAWN participant=1", "recipe=0"],
            Duration::from_secs(20),
        )?;
        let rifleman = econ_int(&spawned, "entity") as u64;
        let start = Instant::now();
        let mut ammo = 0;
        while start.elapsed() < Duration::from_secs(8) && ammo == 0 {
            let text = client_a.text();
            ammo = text
                .lines()
                .rev()
                .filter(|l| l.contains("MP_CLIENT_SAMPLE"))
                .find(|l| field(l, "entity").and_then(|v| v.parse::<u64>().ok()) == Some(rifleman))
                .map_or(0, |l| econ_int(l, "ammo"));
            thread::sleep(Duration::from_millis(250));
        }
        let category = carried_client
            .as_ref()
            .is_ok_and(|l| field(l, "military") == Some("0"));
        report.verdict(
            "HARV-P04-OWNER-FIELDS-REACH-THE-OWNER",
            carried_client.is_ok() && category && ammo > 0,
            format!(
                "harvester {carried_client:?}; trained rifleman {rifleman} ammo {ammo} after {}",
                bought.trim()
            ),
        );
        let text_b = client_b.text();
        let enemy_copies = text_b
            .lines()
            .filter(|l| {
                l.contains("RTS_HARVESTER_CARRY_CLIENT")
                    && field(l, "entity").and_then(|v| v.parse::<u64>().ok())
                        != Some(enemy_harvester)
            })
            .count();
        report.verdict(
            "HARV-P05-THE-ENEMY-GETS-NONE-OF-IT",
            enemy_copies == 0,
            format!("{enemy_copies} carry updates for units client B does not own"),
        );

        // A box over everything takes the soldiers, not the harvester.
        let boxed_mark = client_a.offset();
        client_a.drag_hold(
            (24.0, 24.0),
            (f64::from(WIDTH) - 24.0, f64::from(HEIGHT) - 24.0),
        )?;
        client_a.release()?;
        let boxed = client_a.wait_line(
            &["RTS_SELECTION_SET", "source=box", "op=replace"],
            Duration::from_secs(5),
            boxed_mark,
        )?;
        let ids = selection_ids(&boxed);
        report.verdict(
            "HARV-P06-A-BOX-TAKES-SOLDIERS",
            !ids.is_empty() && !ids.contains(&harvester) && ids.contains(&rifleman),
            boxed.trim().to_owned(),
        );

        // A right-click on the food cache during its next load: no gold moves, and food is its target.
        let source_screen = wait_screen(
            &mut client_a,
            |t| probe_screen(t, 0),
            "the gold cache on client A",
        )?;
        let food_screen = wait_screen(
            &mut client_a,
            |t| probe_screen(t, 1),
            "the food cache on client A",
        )?;
        let server_mark = server.offset();
        let loading = server.wait_line(
            &[
                "RTS_HARVEST_TX participant=1 kind=loading",
                &unit_tag,
                "resource=gold",
            ],
            Duration::from_secs(30),
            server_mark,
        )?;
        thread::sleep(Duration::from_millis(700));
        let mut selected = false;
        for (dx, dy) in [(0.0, 0.0), (0.0, -8.0), (0.0, -16.0), (4.0, -4.0)] {
            let mark = client_a.offset();
            client_a.click(source_screen.0 + dx, source_screen.1 + dy, "1")?;
            if client_a
                .wait_line(
                    &["RTS_SELECTION player=", &format!("unit={harvester}")],
                    Duration::from_millis(350),
                    mark,
                )
                .is_ok()
            {
                selected = true;
                break;
            }
        }
        if !selected {
            return Err("could not select the harvester at the gold cache".to_owned());
        }
        let move_mark = server.offset();
        client_a.click(food_screen.0, food_screen.1, "3")?;
        server.wait_line(
            &[
                "RTS_ORDER_ADMITTED order=Move",
                &format!("unit={harvester}"),
                "participant=1",
            ],
            Duration::from_secs(5),
            move_mark,
        )?;
        let detached = server.wait_line(
            &["RTS_HARVEST_TX participant=1 kind=detached", &unit_tag],
            Duration::from_secs(5),
            move_mark,
        )?;
        let resumed = server.wait_line(
            &["RTS_HARVEST_TX participant=1 kind=resumed", &unit_tag],
            Duration::from_secs(40),
            move_mark,
        )?;
        report.verdict(
            "HARV-P07-THE-FOOD-CACHE-TAKES-OVER",
            field(&resumed, "resource") == Some("food")
                && econ_int(&resumed, "source") == HARVEST_FOOD_FIXTURE
                && econ_int(&resumed, "carried") == 0
                && econ_int(&detached, "source") == econ_int(&loading, "source"),
            format!(
                "{} | {} | {}",
                loading.trim(),
                detached.trim(),
                resumed.trim()
            ),
        );

        // Two food trips, then the small cache is empty and it moves on to the gold by itself.
        let first_food = server.wait_line(
            &[
                "RTS_HARVEST_TX participant=1 kind=deliver ",
                &unit_tag,
                "resource=food",
            ],
            Duration::from_secs(40),
            move_mark,
        )?;
        let reserve_1 = server.wait_line(
            &["RTS_FOOD_TX participant=1 kind=harvest"],
            Duration::from_secs(5),
            move_mark,
        )?;
        client_a.wait(
            &[
                "RTS_HARVESTER_CARRY_CLIENT",
                &format!("entity={harvester} "),
                "carried=2 ",
            ],
            Duration::from_secs(5),
        )?;
        report.artifact(
            "client_a_food_png",
            &client_a.screenshot(&evidence, "03-food-delivered")?,
        );
        let retarget = server.wait_line(
            &["RTS_HARVEST_TX participant=1 kind=retarget", &unit_tag],
            Duration::from_secs(60),
            move_mark,
        )?;
        let next_gold = server.wait_line(
            &[
                "RTS_HARVEST_TX participant=1 kind=load ",
                &unit_tag,
                "resource=gold",
            ],
            Duration::from_secs(40),
            move_mark,
        )?;
        let text = server.text();
        let food_loads = text[move_mark.min(text.len())..]
            .lines()
            .filter(|l| {
                l.contains("RTS_HARVEST_TX participant=1 kind=deliver")
                    && l.contains("resource=food")
            })
            .count();
        report.verdict(
            "HARV-P08-FOOD-REACHES-THE-RESERVE",
            econ_int(&first_food, "amount") == HARVEST_FOOD_LOAD
                && econ_int(&reserve_1, "paid") == HARVEST_FOOD_LOAD
                && field(&reserve_1, "reconciles") == Some("1")
                && food_loads == 2,
            format!(
                "{food_loads} food deliveries | {} | {}",
                first_food.trim(),
                reserve_1.trim()
            ),
        );
        report.verdict(
            "HARV-P09-IT-MOVES-ON-WHEN-THE-CACHE-EMPTIES",
            field(&retarget, "resource") == Some("gold")
                && econ_int(&next_gold, "amount") == GOLD_LOAD,
            format!("{} | {}", retarget.trim(), next_gold.trim()),
        );
        thread::sleep(Duration::from_millis(500));
        report.artifact(
            "client_a_moved_on_png",
            &client_a.screenshot(&evidence, "04-moved-on")?,
        );

        // Gold it brought in sells as before.
        let (client_mark, server_mark) = (client_a.offset(), server.offset());
        client_a.key("l")?;
        let sale = server.wait_line(
            &["RTS_ECONOMY_TX participant=1 kind=sale"],
            Duration::from_secs(5),
            server_mark,
        )?;
        client_a.wait_line(
            &["RTS_ECONOMY_OUTCOME participant=1", "op=3", "accepted=1"],
            Duration::from_secs(5),
            client_mark,
        )?;

        // Only each seat's own data reached it, and everything replays.
        let text = server.text();
        let (text_a, text_b) = (client_a.text(), client_b.text());
        let a_leaks = text_a
            .lines()
            .filter(|l| l.contains("RTS_ECONOMY_VIEW participant=2"))
            .count();
        let b_leaks = text_b
            .lines()
            .filter(|l| l.contains("RTS_ECONOMY_VIEW participant=1"))
            .count();
        report.verdict(
            "HARV-P10-OWNER-ONLY",
            a_leaks == 0 && b_leaks == 0,
            format!("A's copies of B's view {a_leaks}, B's copies of A's view {b_leaks}"),
        );
        let replays = [
            harvest_replay(&text, 1, &generation, "gold", GOLD_NORMAL_SOURCE),
            harvest_replay(&text, 1, &generation, "food", HARVEST_FOOD_FIXTURE),
            harvest_replay(&text, 2, &generation, "gold", GOLD_NORMAL_SOURCE),
            harvest_replay(&text, 2, &generation, "food", HARVEST_FOOD_NORMAL),
        ];
        report.verdict(
            "HARV-P11-BOTH-RESOURCES-REPLAY",
            replays.iter().all(Result::is_ok),
            format!("{replays:?}"),
        );
        let ration_replay = food_replay(&text, 1, &generation, FOOD_NORMAL_RATIONS);
        let lines_1 = economy_lines(&text, 1, &generation);
        let money_replay = economy_replay(&lines_1, ECONOMY_START_CENTS);
        report.verdict(
            "HARV-P12-RATIONS-AND-MONEY-REPLAY",
            ration_replay.is_ok() && money_replay.is_ok() && econ_int(&sale, "amount") == 6_062,
            format!(
                "rations {ration_replay:?}; money {money_replay:?}; {}",
                sale.trim()
            ),
        );

        // Repeatability: the normalized sequence for the owner's harvester, from the player's order
        // on, because the number of gold trips before it depends on how long the inputs took.
        let mut trace = String::new();
        let from_step = econ_int(&detached, "step");
        for line in text.lines().filter(|l| {
            l.contains("RTS_HARVEST_TX participant=1")
                && l.contains(&unit_tag)
                && l.contains(&format!("generation={generation} "))
                && econ_int(l, "step") >= from_step
                && econ_int(l, "step") <= econ_int(&next_gold, "step")
        }) {
            trace.push_str(&format!(
                "{} {} amount={}\n",
                field(line, "kind").unwrap_or("?"),
                field(line, "resource").unwrap_or("?"),
                econ_int(line, "amount"),
            ));
        }
        fs::write(evidence.join("harvest-trace.txt"), &trace).map_err(|e| e.to_string())?;
        report.artifact("harvest_trace", &evidence.join("harvest-trace.txt"));

        // A soldier takes the objective with both harvesters at work, and the next generation has
        // its own harvesters and full caches.
        select_entity(&mut client_a, "MP_CLIENT_SAMPLE", rifleman)?;
        thread::sleep(Duration::from_millis(600));
        let objective = wait_screen(
            &mut client_a,
            |t| objective_screen(t, "MP_CLIENT_OBJECTIVE"),
            "objective on client A",
        )?;
        let at_move = server.offset();
        client_a.click(objective.0, objective.1, "3")?;
        server.wait_line(
            &[
                "RTS_ORDER_ADMITTED order=Move",
                &format!("unit={rifleman}"),
                "participant=1",
            ],
            Duration::from_secs(5),
            at_move,
        )?;
        server.wait(
            &["MATCH_RESULTS outcome=objective_secured", &gen_tag],
            Duration::from_secs(180),
        )?;
        let cleared = server.wait(
            &["RTS_ECONOMY_CLEARED participant=1", "reason=match_ended"],
            Duration::from_secs(5),
        )?;
        let at_reset = server.offset();
        client_a.hover(640.0, 360.0)?;
        client_a.key("Escape")?;
        client_b.stop();
        server.wait_line(
            &["MP_DEDICATED_SESSION_RESET", "travel=1"],
            Duration::from_secs(60),
            at_reset,
        )?;
        let next = server.wait_line(
            &["MP_MATCH_AWAITING_PARTICIPANTS"],
            Duration::from_secs(90),
            at_reset,
        )?;
        let generation2 = field(&next, "generation").unwrap_or("0").to_owned();
        let text = server.text();
        let tail = &text[at_reset.min(text.len())..];
        let free2 = tail
            .lines()
            .filter(|l| {
                l.contains("RTS_HARVESTER_START")
                    && l.contains(&format!("generation={generation2}"))
            })
            .count();
        report.verdict(
            "HARV-P13-NEXT-GENERATION-FRESH",
            generation2 != generation
                && field(&cleared, "reconciles") == Some("1")
                && field(&cleared, "gold_reconciles") == Some("1")
                && free2 == 2
                && !tail.lines().any(|l| l.contains("RTS_HARVEST_TX")),
            format!(
                "generation {generation} then {generation2}: {free2} harvesters | {}",
                cleared.trim()
            ),
        );
        Ok(())
    })();
    launch_options_verdict(
        report,
        "HARV-P00-LAUNCH-OPTIONS",
        &[&server, &client_a, &client_b],
    );
    client_a.stop();
    client_b.stop();
    server.stop();
    result
}

// `--buildings SERVER CLIENT [--evidence DIR] [--port N]` plays building damage, capture, demolition
// and repair with real input from two rendered remote clients on a dedicated 1v1, from the normal
// start. Team B holds its fire so the attacker reaches its base; every later shot at a building is
// an explicit order. Team A's soldier walks a corridor south of the objective and clear of both
// militia, so the match does not end on the way and nothing else damages it.
const BLD_OWN_PRODUCER: (f64, f64) = (-1740.2, -937.0);
const BLD_OWN_GENERATOR: (f64, f64) = (-1574.3, -1245.2);
const BLD_CORRIDOR: (f64, f64) = (200.0, -290.0);
const BLD_POST: (f64, f64) = (1250.0, 1000.0);
const BLD_RETREAT: (f64, f64) = (950.0, 1000.0);
const BLD_OBJECTIVE: (f64, f64) = (0.0, 150.0);
const BLD_PROBE_OWN_PRODUCER: usize = 0;
const BLD_PROBE_OWN_GENERATOR: usize = 1;
const BLD_PROBE_ENEMY_PRODUCER: usize = 2;
const BLD_PROBE_ENEMY_GENERATOR: usize = 3;
const BLD_PROBE_CORRIDOR: usize = 4;
const BLD_PROBE_POST: usize = 5;
const BLD_PROBE_RETREAT: usize = 6;
const BLD_PROBE_OBJECTIVE: usize = 7;

fn building_probes() -> String {
    let points = [
        BLD_OWN_PRODUCER,
        BLD_OWN_GENERATOR,
        (-BLD_OWN_PRODUCER.0, -BLD_OWN_PRODUCER.1),
        (-BLD_OWN_GENERATOR.0, -BLD_OWN_GENERATOR.1),
        BLD_CORRIDOR,
        BLD_POST,
        BLD_RETREAT,
        BLD_OBJECTIVE,
    ];
    let joined: Vec<String> = points.iter().map(|p| format!("{}:{}", p.0, p.1)).collect();
    format!("-PolitikaMPGroundProbe={}", joined.join("/"))
}

/// Where a ground probe projects to now, on the screen or not.
fn probe_projection(text: &str, index: usize) -> Option<(f64, f64)> {
    let index = index.to_string();
    text.lines()
        .rev()
        .find(|l| l.contains("MP_CLIENT_GROUND_PROBE") && field(l, "index") == Some(&index))
        .filter(|l| field(l, "on_screen") == Some("1"))
        .and_then(|l| Some((number(l, "screen_x")?, number(l, "screen_y")?)))
}

/// Pans with the real keys until a world point is well inside the view, and returns its latest
/// projection. Samples arrive about once a second, so each tap waits for a fresh one.
fn pan_to_probe(game: &mut Game, index: usize) -> Result<(f64, f64), String> {
    let (width, height) = (f64::from(WIDTH), f64::from(HEIGHT));
    let start = Instant::now();
    while start.elapsed() < Duration::from_secs(90) {
        let mark = game.offset();
        game.wait_line(
            &["MP_CLIENT_GROUND_PROBE", &format!("index={index} ")],
            Duration::from_secs(5),
            mark,
        )?;
        let (x, y) = probe_projection(&game.text(), index)
            .ok_or_else(|| format!("{}: probe {index} is behind the camera", game.label))?;
        let horizontal = if x < 240.0 {
            Some(("a", 240.0 - x))
        } else if x > width - 240.0 {
            Some(("d", x - (width - 240.0)))
        } else {
            None
        };
        let vertical = if y < 160.0 {
            Some(("w", 160.0 - y))
        } else if y > height - 160.0 {
            Some(("s", y - (height - 160.0)))
        } else {
            None
        };
        if horizontal.is_none() && vertical.is_none() {
            return Ok((x, y));
        }
        for (key, off_by) in [horizontal, vertical].into_iter().flatten() {
            let hold = (off_by * 1.2).clamp(150.0, 900.0) as u64;
            game.xdo_here(&["keydown", key])?;
            thread::sleep(Duration::from_millis(hold));
            game.xdo_here(&["keyup", key])?;
        }
    }
    Err(format!("{}: could not pan to probe {index}", game.label))
}

/// Puts the real cursor on a building and returns where. The cursor line reads it from there.
fn point_at_probe(game: &mut Game, index: usize) -> Result<(f64, f64), String> {
    let at = pan_to_probe(game, index)?;
    game.hover(at.0, at.1)?;
    thread::sleep(Duration::from_millis(350));
    Ok(at)
}

/// The next cursor line naming a building, after `mark`, containing every fragment.
fn wait_cursor(game: &mut Game, fragments: &[&str], mark: usize) -> Result<String, String> {
    let mut wanted = vec!["RTS_BUILDING_CURSOR"];
    wanted.extend_from_slice(fragments);
    game.wait_line(&wanted, Duration::from_secs(5), mark)
}

/// Whether a fragment set appears on the server within `window`, starting from `mark`.
fn appears(server: &mut Game, fragments: &[&str], window: Duration, mark: usize) -> bool {
    server.wait_line(fragments, window, mark).is_ok()
}

/// Presses H until the selection holds its fire.
fn set_hold_fire(game: &mut Game, server: &mut Game, participant: u32) -> Result<String, String> {
    let tag = format!("RTS_STANCE_SET participant={participant} ");
    for _ in 0..4 {
        let mark = server.offset();
        game.key("h")?;
        let set = server.wait_line(&[&tag], Duration::from_secs(5), mark)?;
        if field(&set, "stance") == Some("hold_fire") {
            return Ok(set);
        }
    }
    Err(format!("{}: never reached hold_fire", game.label))
}

fn buildings_session(options: &DedicatedOptions, report: &mut Report) -> Result<(), String> {
    let client_options = &options.client;
    let evidence = &client_options.evidence;
    let mut args = server_args(client_options.port, "1v1");
    args.retain(|a| a != NO_STARTING_HARVESTERS);
    args.push(STARTING_HARVESTERS.to_owned());
    let mut server = Game::launch_server(&options.server, "server", evidence, &args)?;
    report.fact("server_command", server.command.join(" "));
    server.wait(&["MP_DEDICATED_SERVER_READY"], Duration::from_secs(180))?;
    let awaiting = server.wait(&["MP_MATCH_AWAITING_PARTICIPANTS"], Duration::from_secs(60))?;
    let generation = field(&awaiting, "generation").unwrap_or("0").to_owned();
    let launch = |label: &str, participant: u32, server: &mut Game| {
        let display = Display::start(evidence, label)?;
        let mut args = client_args(client_options, true, false, false);
        args.push(building_probes());
        let game = Game::launch(
            &client_options.binary,
            label,
            evidence,
            &args,
            Some(&display.name()),
        )?;
        server.wait(
            &[
                &format!("MP_PARTICIPANT_BOUND participant={participant}"),
                "remote=1",
            ],
            Duration::from_secs(90),
        )?;
        Ok::<(Game, Display), String>((game, display))
    };
    let (mut client_a, _display_a) = launch("client-a", 1, &mut server)?;
    let (mut client_b, _display_b) = launch("client-b", 2, &mut server)?;
    report.fact("client_a_command", client_a.command.join(" "));
    server.wait(
        &["MP_MATCH_ACTIVE", &format!("generation={generation}")],
        Duration::from_secs(30),
    )?;
    for client in [&mut client_a, &mut client_b] {
        client.wait(
            &[
                "MP_CLIENT_MATCH_OBSERVED",
                "map=map.test.multiplayer_skirmish",
            ],
            Duration::from_secs(60),
        )?;
        client.attach_window()?;
    }
    thread::sleep(Duration::from_secs(6));

    let result = (|| -> Result<(), String> {
        let gen_tag = format!("generation={generation}");
        let owners = entity_owners(&server.text());
        let soldiers = |owner: u32| -> Vec<u64> {
            owners
                .iter()
                .filter(|(_, o)| *o == owner)
                .map(|(e, _)| *e)
                .collect()
        };
        let (team_a, team_b) = (soldiers(1), soldiers(2));
        if team_a.len() < 3 || team_b.len() < 3 {
            return Err(format!("unexpected 1v1 roster {owners:?}"));
        }
        // The two authored soldiers come first; the free harvester has a later id.
        let (attacker, reserve) = (team_a[0], team_a[1]);
        let defender = team_b[0];
        report.fact("team_a", format!("{team_a:?}"));
        report.fact("team_b", format!("{team_b:?}"));
        zoom_out(&client_a, 5)?;
        zoom_out(&client_b, 5)?;
        thread::sleep(Duration::from_millis(1500));

        // Under fog each client starts with its own two buildings only.
        let kinds = |text: &str| -> Vec<String> {
            let mut seen: Vec<String> = text
                .lines()
                .filter(|l| l.contains("RTS_STRUCTURE_RELEVANT"))
                .filter_map(|l| Some(format!("{}:{}", field(l, "kind")?, field(l, "owner")?)))
                .collect();
            seen.sort();
            seen
        };
        let (start_a, start_b) = (kinds(&client_a.text()), kinds(&client_b.text()));
        report.verdict(
            "BLD-P01-EACH-CLIENT-HAS-ITS-OWN-BUILDINGS",
            start_a == ["generator:1", "producer:1"] && start_b == ["generator:2", "producer:2"],
            format!("client A {start_a:?}, client B {start_b:?}"),
        );

        // Team B holds its fire, so the attacker is not in a soldier's fight.
        client_b.drag_hold(
            (24.0, 24.0),
            (f64::from(WIDTH) - 24.0, f64::from(HEIGHT) - 24.0),
        )?;
        client_b.release()?;
        thread::sleep(Duration::from_millis(500));
        let held_b = set_hold_fire(&mut client_b, &mut server, 2)?;
        report.fact("team_b_stance", held_b.trim().to_owned());

        // The cursor names your own building, and Z takes it down: a remote client's real input.
        let mark_cursor = client_a.offset();
        let own_generator = point_at_probe(&mut client_a, BLD_PROBE_OWN_GENERATOR)?;
        let named = wait_cursor(
            &mut client_a,
            &["Your generator", "100 / 100", "online"],
            mark_cursor,
        );
        report.verdict(
            "BLD-P02-THE-CURSOR-NAMES-A-BUILDING",
            named.is_ok(),
            format!("{named:?}"),
        );
        let mark = server.offset();
        client_a.key("z")?;
        let demolished = server.wait_line(
            &["RTS_BUILDING_DEMOLISH participant=1", "kind=generator"],
            Duration::from_secs(5),
            mark,
        );
        thread::sleep(Duration::from_millis(400));
        let mark_reject = server.offset();
        client_a.key("b")?;
        let unpowered = server.wait_line(
            &[
                "RTS_ECONOMY_REJECT participant=1",
                "rejection=producer_unpowered",
            ],
            Duration::from_secs(5),
            mark_reject,
        );
        report.artifact(
            "client_a_generator_demolished_png",
            &client_a.screenshot(evidence, "01-generator-demolished")?,
        );
        report.verdict(
            "BLD-P03-Z-DEMOLISHES-YOUR-GENERATOR",
            demolished.is_ok() && unpowered.is_ok(),
            format!("{demolished:?} | {unpowered:?}"),
        );

        // A right-click on your own husk sends the selected unit. It walks over and works, and the
        // plant stays down until the work is finished.
        select_entity(&mut client_a, "MP_CLIENT_SAMPLE", reserve)?;
        let reserve_tag = format!("unit={reserve} ");
        let mark = server.offset();
        let mark_cursor = client_a.offset();
        client_a.click(own_generator.0, own_generator.1, "3")?;
        let begun = server.wait_line(
            &["RTS_BUILDING_REPAIR phase=", &reserve_tag],
            Duration::from_secs(5),
            mark,
        );
        let working = server.wait_line(
            &["RTS_BUILDING_REPAIR phase=work", &reserve_tag],
            Duration::from_secs(60),
            mark,
        );
        let reading = wait_cursor(
            &mut client_a,
            &["Your generator", "offline", "repairing"],
            mark_cursor,
        );
        let mark_reject = server.offset();
        client_a.key("b")?;
        let still_down = server.wait_line(
            &[
                "RTS_ECONOMY_REJECT participant=1",
                "rejection=producer_unpowered",
            ],
            Duration::from_secs(3),
            mark_reject,
        );
        report.artifact(
            "client_a_repair_under_way_png",
            &client_a.screenshot(evidence, "01b-repair-under-way")?,
        );

        // X ends the work, and the health restored so far stays.
        let mark = server.offset();
        client_a.key("x")?;
        let stopped = server.wait_line(
            &["RTS_BUILDING_REPAIR phase=end", &reserve_tag],
            Duration::from_secs(3),
            mark,
        );
        let first_steps = stopped
            .as_ref()
            .ok()
            .and_then(|l| number(l, "steps"))
            .unwrap_or(0.0);
        let kept = stopped
            .as_ref()
            .ok()
            .and_then(|l| number(l, "health"))
            .unwrap_or(-1.0);
        let mark_cursor = client_a.offset();
        client_a.hover(640.0, 40.0)?;
        thread::sleep(Duration::from_millis(300));
        client_a.hover(own_generator.0, own_generator.1)?;
        let health_text = format!("{kept:.0} / 100");
        let stopped_reading = wait_cursor(
            &mut client_a,
            &["Your generator", &health_text, "right-click to repair"],
            mark_cursor,
        );
        let finished_early = appears(
            &mut server,
            &["RTS_BUILDING_REPAIR participant=1"],
            Duration::from_secs(1),
            mark,
        );
        report.verdict(
            "BLD-P18-AN-ORDER-STOPS-THE-REPAIR-AND-KEEPS-THE-HEALTH",
            stopped.is_ok()
                && first_steps >= 1.0
                && first_steps < 100.0
                && kept == first_steps
                && stopped_reading.is_ok()
                && !finished_early,
            format!("{stopped:?} | {stopped_reading:?} | finished anyway: {finished_early}"),
        );

        // A second right-click finishes the job from there, and training works again.
        let mark = server.offset();
        client_a.click(own_generator.0, own_generator.1, "3")?;
        let finished = server.wait_line(
            &["RTS_BUILDING_REPAIR phase=end", &reserve_tag],
            Duration::from_secs(30),
            mark,
        );
        let second_steps = finished
            .as_ref()
            .ok()
            .and_then(|l| number(l, "steps"))
            .unwrap_or(0.0);
        let repaired = server.wait_line(
            &["RTS_BUILDING_REPAIR participant=1", "health=100"],
            Duration::from_secs(2),
            mark,
        );
        thread::sleep(Duration::from_millis(400));
        let mark_train = server.offset();
        client_a.key("b")?;
        let trained = server.wait_line(
            &["RTS_ECONOMY_TX participant=1", "kind=train"],
            Duration::from_secs(5),
            mark_train,
        );
        report.verdict(
            "BLD-P04-A-RIGHT-CLICK-REPAIRS-YOUR-GENERATOR",
            begun.is_ok()
                && working.is_ok()
                && reading.is_ok()
                && still_down.is_ok()
                && repaired.is_ok()
                && first_steps + second_steps == 100.0
                && trained.is_ok(),
            format!(
                "{begun:?} | {working:?} | {reading:?} | {still_down:?} | {repaired:?} | steps {first_steps} + {second_steps} | {trained:?}"
            ),
        );

        // Your own healthy building is not a refusal: the click is a move to the ground under the
        // cursor. The cursor is on its roof or side, so that ground can lie past its centre.
        let own_producer = pan_to_probe(&mut client_a, BLD_PROBE_OWN_PRODUCER)?;
        let mark = server.offset();
        client_a.click(own_producer.0, own_producer.1, "3")?;
        let moved_under = server.wait_line(
            &["RTS_ORDER_ADMITTED order=Move", &format!("unit={reserve} ")],
            Duration::from_secs(5),
            mark,
        );
        let arrived_under = wait_arrival(
            &mut server,
            reserve,
            BLD_OWN_PRODUCER,
            400.0,
            Duration::from_secs(30),
        );
        report.verdict(
            "BLD-P17-A-RIGHT-CLICK-ON-YOUR-BUILDING-IS-A-MOVE",
            moved_under.is_ok() && arrived_under.is_ok(),
            format!("{moved_under:?} | {arrived_under:?}"),
        );

        // The walk: past the objective and the militia, into sight of the enemy base.
        select_entity(&mut client_a, "MP_CLIENT_SAMPLE", attacker)?;
        thread::sleep(Duration::from_millis(300));
        let held_a = set_hold_fire(&mut client_a, &mut server, 1)?;
        report.fact("team_a_stance", held_a.trim().to_owned());
        for (index, point, name) in [
            (BLD_PROBE_CORRIDOR, BLD_CORRIDOR, "corridor"),
            (BLD_PROBE_POST, BLD_POST, "post"),
        ] {
            let at = pan_to_probe(&mut client_a, index)?;
            let mark = server.offset();
            client_a.click(at.0, at.1, "3")?;
            server
                .wait_line(
                    &[
                        "RTS_ORDER_ADMITTED order=Move",
                        &format!("unit={attacker} "),
                    ],
                    Duration::from_secs(5),
                    mark,
                )
                .map_err(|e| format!("the move to the {name}: {e}"))?;
            wait_arrival(
                &mut server,
                attacker,
                point,
                150.0,
                Duration::from_secs(120),
            )
            .map_err(|e| format!("the walk to the {name}: {e}"))?;
        }
        client_a.wait(
            &["RTS_STRUCTURE_RELEVANT", "kind=generator", "owner=2"],
            Duration::from_secs(10),
        )?;
        client_a.wait(
            &["RTS_STRUCTURE_RELEVANT", "kind=producer", "owner=2"],
            Duration::from_secs(10),
        )?;
        thread::sleep(Duration::from_millis(1500));
        report.verdict(
            "BLD-P05-A-SEEN-ENEMY-BUILDING-ARRIVES",
            !start_a.iter().any(|k| k.ends_with(":2")),
            format!("client A now {:?}", kinds(&client_a.text())),
        );
        report.artifact(
            "client_a_enemy_base_png",
            &client_a.screenshot(evidence, "02-enemy-base")?,
        );

        // Z is yours alone, and C is refused on a producer while you still have a living one.
        select_entity(&mut client_a, "MP_CLIENT_SAMPLE", attacker)?;
        point_at_probe(&mut client_a, BLD_PROBE_ENEMY_PRODUCER)?;
        let mark = server.offset();
        client_a.key("z")?;
        let enemy_demolish = appears(
            &mut server,
            &["RTS_BUILDING_DEMOLISH"],
            Duration::from_secs(2),
            mark,
        );
        let mark = server.offset();
        client_a.key("c")?;
        let early_capture = appears(
            &mut server,
            &["RTS_BUILDING_CAPTURE"],
            Duration::from_secs(2),
            mark,
        );

        // C on the generator walks in and holds; a move order during the hold cancels it.
        point_at_probe(&mut client_a, BLD_PROBE_ENEMY_GENERATOR)?;
        let mark = server.offset();
        client_a.key("c")?;
        let begun = server.wait_line(
            &["RTS_BUILDING_CAPTURE phase=", &format!("unit={attacker} ")],
            Duration::from_secs(5),
            mark,
        );
        if begun.is_ok() {
            wait_arrival(
                &mut server,
                attacker,
                (-BLD_OWN_GENERATOR.0, -BLD_OWN_GENERATOR.1),
                200.0,
                Duration::from_secs(20),
            )?;
            thread::sleep(Duration::from_millis(600));
        }
        let retreat = pan_to_probe(&mut client_a, BLD_PROBE_RETREAT)?;
        let mark = server.offset();
        client_a.click(retreat.0, retreat.1, "3")?;
        server.wait_line(
            &[
                "RTS_ORDER_ADMITTED order=Move",
                &format!("unit={attacker} "),
            ],
            Duration::from_secs(5),
            mark,
        )?;
        let cancelled = !appears(
            &mut server,
            &["RTS_BUILDING_CAPTURE unit=", "captured=1"],
            Duration::from_secs(6),
            mark,
        );
        report.verdict(
            "BLD-P06-CAPTURE-RULES-HOLD-ON-THE-SERVER",
            begun.is_ok() && !enemy_demolish && !early_capture && cancelled,
            format!(
                "Z on the enemy producer logged {enemy_demolish}; C on it with a living producer logged {early_capture}; {begun:?}; a move during the hold cancelled it: {cancelled}"
            ),
        );

        // A right-click on the enemy generator is an attack: one hit takes it offline, the next
        // shots finish it, and its owner's client sees all of it.
        let generator = pan_to_probe(&mut client_a, BLD_PROBE_ENEMY_GENERATOR)?;
        let mark = server.offset();
        client_a.click(generator.0, generator.1, "3")?;
        let attack = server.wait_line(
            &[
                "RTS_ORDER_ADMITTED order=Attack",
                &format!("unit={attacker} "),
            ],
            Duration::from_secs(5),
            mark,
        );
        report.verdict(
            "BLD-P07-A-RIGHT-CLICK-ATTACKS-A-BUILDING",
            attack.is_ok(),
            format!("{attack:?}"),
        );
        attack?;
        let first_hit = server.wait_line(
            &[
                "RTS_BUILDING_DAMAGE participant=2",
                "kind=generator",
                "offline=1",
            ],
            Duration::from_secs(30),
            mark,
        )?;
        let mark_b = client_b.offset();
        point_at_probe(&mut client_b, BLD_PROBE_ENEMY_GENERATOR)?;
        let offline_b = wait_cursor(&mut client_b, &["Your generator", "offline"], mark_b);
        let killed = server.wait_line(
            &[
                "RTS_BUILDING_DAMAGE participant=2",
                "kind=generator",
                "health=0 ",
            ],
            Duration::from_secs(120),
            mark,
        )?;
        thread::sleep(Duration::from_millis(500));
        let mark_cursor = client_b.offset();
        client_b.hover(640.0, 40.0)?;
        thread::sleep(Duration::from_millis(300));
        let own_husk = pan_to_probe(&mut client_b, BLD_PROBE_ENEMY_GENERATOR)?;
        client_b.hover(own_husk.0, own_husk.1)?;
        let husk_line = wait_cursor(
            &mut client_b,
            &["Your generator", "0 / 100", "right-click to repair"],
            mark_cursor,
        );
        let mark_reject = server.offset();
        client_b.key("b")?;
        let b_unpowered = server.wait_line(
            &[
                "RTS_ECONOMY_REJECT participant=2",
                "rejection=producer_unpowered",
            ],
            Duration::from_secs(5),
            mark_reject,
        );
        report.artifact(
            "client_b_generator_destroyed_png",
            &client_b.screenshot(evidence, "03-husk-as-its-owner-sees-it")?,
        );
        report.verdict(
            "BLD-P08-THE-GENERATOR-GOES-DOWN-FOR-ITS-OWNER",
            offline_b.is_ok() && husk_line.is_ok() && b_unpowered.is_ok(),
            format!(
                "{} | {} | {offline_b:?} | {husk_line:?} | {b_unpowered:?}",
                first_hit.trim(),
                killed.trim()
            ),
        );

        // Its owner sends a soldier, and the attacker still standing there shoots it off the job. One
        // hit ends the work and the health restored so far stays.
        select_entity(&mut client_b, "MP_CLIENT_SAMPLE", defender)?;
        let defender_tag = format!("unit={defender} ");
        let husk = pan_to_probe(&mut client_b, BLD_PROBE_ENEMY_GENERATOR)?;
        let mark = server.offset();
        client_b.click(husk.0, husk.1, "3")?;
        server
            .wait_line(
                &["RTS_BUILDING_REPAIR phase=work", &defender_tag],
                Duration::from_secs(60),
                mark,
            )
            .map_err(|e| format!("the walk to the husk: {e}"))?;
        let attacked = order_attack(&mut client_a, &mut server, attacker, defender);
        let hit = server.wait_line(
            &["RTS_BUILDING_REPAIR phase=hit", &defender_tag],
            Duration::from_secs(5),
            mark,
        );
        client_a.key("x")?;
        let ended = server.wait_line(
            &["RTS_BUILDING_REPAIR phase=end", &defender_tag],
            Duration::from_secs(3),
            mark,
        );
        let first_steps_b = ended
            .as_ref()
            .ok()
            .and_then(|l| number(l, "steps"))
            .unwrap_or(0.0);
        let kept_b = ended.as_ref().ok().and_then(|l| number(l, "health"));
        let finished_under_fire = appears(
            &mut server,
            &["RTS_BUILDING_REPAIR participant=2"],
            Duration::from_secs(1),
            mark,
        );
        report.verdict(
            "BLD-P19-A-HIT-ON-THE-UNIT-STOPS-THE-REPAIR",
            attacked.is_ok()
                && hit.is_ok()
                && ended.is_ok()
                && first_steps_b >= 1.0
                && first_steps_b < 100.0
                && !finished_under_fire,
            format!("{attacked:?} | {hit:?} | {ended:?} | finished anyway: {finished_under_fire}"),
        );
        thread::sleep(Duration::from_millis(500));

        // Left alone it finishes, and its owner queues two units on the restored power.
        let husk = pan_to_probe(&mut client_b, BLD_PROBE_ENEMY_GENERATOR)?;
        let mark = server.offset();
        client_b.click(husk.0, husk.1, "3")?;
        let finished_b = server.wait_line(
            &["RTS_BUILDING_REPAIR phase=end", &defender_tag],
            Duration::from_secs(30),
            mark,
        );
        let second_steps_b = finished_b
            .as_ref()
            .ok()
            .and_then(|l| number(l, "steps"))
            .unwrap_or(0.0);
        let repaired_b = server.wait_line(
            &["RTS_BUILDING_REPAIR participant=2", "health=100"],
            Duration::from_secs(2),
            mark,
        );
        thread::sleep(Duration::from_millis(400));
        let mark = server.offset();
        client_b.key("b")?;
        thread::sleep(Duration::from_millis(300));
        client_b.key("b")?;
        let b_queued = server.wait_line(
            &["RTS_ECONOMY_TX participant=2", "kind=train", "queued=2"],
            Duration::from_secs(5),
            mark,
        );
        report.verdict(
            "BLD-P09-ITS-OWNER-REPAIRS-IT",
            repaired_b.is_ok()
                && kept_b.is_some_and(|kept| second_steps_b == 100.0 - kept)
                && b_queued.is_ok(),
            format!(
                "{repaired_b:?} | kept {kept_b:?} of {first_steps_b} steps, then {second_steps_b} | {b_queued:?}"
            ),
        );

        // A holds next to the generator until it changes hands.
        select_entity(&mut client_a, "MP_CLIENT_SAMPLE", attacker)?;
        let mark_cursor = client_a.offset();
        point_at_probe(&mut client_a, BLD_PROBE_ENEMY_GENERATOR)?;
        let mark = server.offset();
        client_a.key("c")?;
        let captured = server.wait_line(
            &[
                "RTS_BUILDING_CAPTURE unit=",
                "from=2",
                "to=1",
                "kind=generator",
                "captured=1",
            ],
            Duration::from_secs(40),
            mark,
        );
        thread::sleep(Duration::from_millis(800));
        let ours = wait_cursor(&mut client_a, &["Your generator", "online"], mark_cursor);
        let powerless = server.wait_line(
            &["RTS_POWER_STATE participant=2", "producer_powered=0"],
            Duration::from_secs(5),
            mark,
        );
        report.artifact(
            "client_a_generator_captured_png",
            &client_a.screenshot(evidence, "04-generator-captured")?,
        );
        report.verdict(
            "BLD-P10-C-CAPTURES-THE-GENERATOR",
            captured.is_ok() && ours.is_ok() && powerless.is_ok(),
            format!("{captured:?} | {ours:?} | {powerless:?}"),
        );

        // Z on your own producer refunds its queue once and takes it off the map.
        let mark = server.offset();
        pan_to_probe(&mut client_a, BLD_PROBE_OWN_PRODUCER)?;
        client_a.key("b")?;
        thread::sleep(Duration::from_millis(300));
        client_a.key("b")?;
        let a_queued = server.wait_line(
            &["RTS_ECONOMY_TX participant=1", "kind=train", "queued="],
            Duration::from_secs(5),
            mark,
        )?;
        thread::sleep(Duration::from_millis(400));
        point_at_probe(&mut client_a, BLD_PROBE_OWN_PRODUCER)?;
        let mark = server.offset();
        let mark_a = client_a.offset();
        client_a.key("z")?;
        let lost = server.wait_line(
            &["RTS_BUILDING_DEMOLISH participant=1", "kind=producer"],
            Duration::from_secs(5),
            mark,
        );
        let refunded = lost.as_ref().map_or(0, |l| econ_int(l, "refunded"));
        let withdrawn = client_a.wait_line(
            &["RTS_STRUCTURE_WITHDRAWN", "kind=producer", "owner=1"],
            Duration::from_secs(5),
            mark_a,
        );
        thread::sleep(Duration::from_millis(500));
        let mark_again = server.offset();
        client_a.key("z")?;
        let second = appears(
            &mut server,
            &["RTS_BUILDING_DEMOLISH"],
            Duration::from_secs(2),
            mark_again,
        );
        let mark_reject = server.offset();
        client_a.key("b")?;
        let no_producer = server.wait_line(
            &["RTS_ECONOMY_REJECT participant=1", "rejection=no_producer"],
            Duration::from_secs(5),
            mark_reject,
        );
        report.artifact(
            "client_a_producer_gone_png",
            &client_a.screenshot(evidence, "05-producer-gone")?,
        );
        report.verdict(
            "BLD-P11-Z-TAKES-YOUR-PRODUCER-OFF-THE-MAP",
            refunded > 0 && withdrawn.is_ok() && !second && no_producer.is_ok(),
            format!(
                "{} | {lost:?} | {withdrawn:?} | second press logged {second} | {no_producer:?}",
                a_queued.trim()
            ),
        );

        // With no producer of its own, A can capture B's: B's queue comes back once, and A trains there.
        select_entity_near_probe(&mut client_a, attacker, BLD_PROBE_ENEMY_PRODUCER)?;
        point_at_probe(&mut client_a, BLD_PROBE_ENEMY_PRODUCER)?;
        let mark = server.offset();
        client_a.key("c")?;
        let producer_taken = server.wait_line(
            &[
                "RTS_BUILDING_CAPTURE unit=",
                "from=2",
                "to=1",
                "kind=producer",
                "captured=1",
            ],
            Duration::from_secs(40),
            mark,
        );
        thread::sleep(Duration::from_millis(500));
        let mark_train = server.offset();
        client_a.key("b")?;
        let spawned = server.wait_line(
            &["RTS_PRODUCTION_SPAWN participant=1"],
            Duration::from_secs(30),
            mark_train,
        );
        let near_captured = spawned.as_ref().is_ok_and(|l| {
            distance(
                (number(l, "x").unwrap_or(0.0), number(l, "y").unwrap_or(0.0)),
                (-BLD_OWN_PRODUCER.0, -BLD_OWN_PRODUCER.1),
            ) < 800.0
        });
        report.artifact(
            "client_a_producer_captured_png",
            &client_a.screenshot(evidence, "06-producer-captured")?,
        );
        report.verdict(
            "BLD-P12-THE-CAPTURED-PRODUCER-WORKS",
            producer_taken
                .as_ref()
                .is_ok_and(|l| econ_int(l, "refunded") > 0)
                && near_captured,
            format!("{producer_taken:?} | {spawned:?}"),
        );

        // A's new rifleman holds its fire, or it would shoot B's soldier before the next check.
        let rifleman = spawned.as_ref().map_or(0, |l| econ_int(l, "entity") as u64);
        select_entity(&mut client_a, "MP_CLIENT_SAMPLE", rifleman)?;
        let held_new = set_hold_fire(&mut client_a, &mut server, 1)?;
        report.fact("new_rifleman_stance", held_new.trim().to_owned());

        // The harvester kept its load through the gap, and delivers to the captured producer.
        let captured_mark = server.offset();
        let delivered_there = server.wait_line(
            &["RTS_HARVEST_TX participant=1 kind=deliver"],
            Duration::from_secs(120),
            captured_mark,
        );
        report.verdict(
            "BLD-P16-THE-HARVESTER-DELIVERS-TO-THE-CAPTURED-PRODUCER",
            delivered_there.is_ok(),
            format!(
                "{delivered_there:?}; hold and resume lines before it: {}",
                server
                    .text()
                    .lines()
                    .filter(|l| l.contains("RTS_HARVEST_TX participant=1 kind=holding")
                        || l.contains("RTS_HARVEST_TX participant=1 kind=resumed"))
                    .count()
            ),
        );

        // B's soldier destroys it by fire with a queue on it: one refund, gone for both clients.
        let mark_train = server.offset();
        client_a.key("b")?;
        server.wait_line(
            &["RTS_ECONOMY_TX participant=1", "kind=train"],
            Duration::from_secs(5),
            mark_train,
        )?;
        select_entity(&mut client_b, "MP_CLIENT_SAMPLE", defender)?;
        let target = pan_to_probe(&mut client_b, BLD_PROBE_ENEMY_PRODUCER)?;
        let (mark, mark_a, mark_b) = (server.offset(), client_a.offset(), client_b.offset());
        client_b.click(target.0, target.1, "3")?;
        let b_attack = server.wait_line(
            &[
                "RTS_ORDER_ADMITTED order=Attack",
                &format!("unit={defender} "),
            ],
            Duration::from_secs(5),
            mark,
        );
        let destroyed = server.wait_line(
            &[
                "RTS_BUILDING_DAMAGE participant=1",
                "kind=producer",
                "producer_lost=1",
            ],
            Duration::from_secs(180),
            mark,
        );
        let gone_a = client_a.wait_line(
            &["RTS_STRUCTURE_WITHDRAWN", "kind=producer", "owner=1"],
            Duration::from_secs(5),
            mark_a,
        );
        let gone_b = client_b.wait_line(
            &["RTS_STRUCTURE_WITHDRAWN", "kind=producer", "owner=1"],
            Duration::from_secs(5),
            mark_b,
        );
        let text = server.text();
        let refunds = text[mark.min(text.len())..]
            .lines()
            .filter(|l| {
                l.contains("RTS_BUILDING_DAMAGE participant=1") && econ_int(l, "refunded") > 0
            })
            .count();
        report.verdict(
            "BLD-P13-FIRE-DESTROYS-A-PRODUCER-WITH-ONE-REFUND",
            b_attack.is_ok()
                && destroyed.is_ok()
                && refunds == 1
                && gone_a.is_ok()
                && gone_b.is_ok(),
            format!(
                "{b_attack:?} | {destroyed:?} | {refunds} refunding hits | {gone_a:?} | {gone_b:?}"
            ),
        );

        // Ammunition never goes below zero while firing at a building.
        let lowest = client_b
            .text()
            .lines()
            .filter(|l| {
                l.contains("MP_CLIENT_SAMPLE")
                    && field(l, "entity").and_then(|v| v.parse::<u64>().ok()) == Some(defender)
            })
            .map(|l| econ_int(l, "ammo"))
            .min()
            .unwrap_or(i64::MIN);
        report.verdict(
            "BLD-P14-AMMUNITION-STAYS-IN-RANGE",
            lowest >= 0,
            format!("lowest ammo the owner saw on {defender}: {lowest}"),
        );

        // The match ends with every ledger whole, and the next one starts with whole buildings.
        select_entity_near_probe(&mut client_a, reserve, BLD_PROBE_OWN_GENERATOR)?;
        let objective = pan_to_probe(&mut client_a, BLD_PROBE_OBJECTIVE)?;
        client_a.click(objective.0, objective.1, "3")?;
        server.wait(
            &["MATCH_RESULTS outcome=objective_secured", &gen_tag],
            Duration::from_secs(240),
        )?;
        let cleared_1 = server.wait(
            &["RTS_ECONOMY_CLEARED participant=1", "reason=match_ended"],
            Duration::from_secs(5),
        )?;
        let cleared_2 = server.wait(
            &["RTS_ECONOMY_CLEARED participant=2", "reason=match_ended"],
            Duration::from_secs(5),
        )?;
        let at_reset = server.offset();
        client_a.hover(640.0, 360.0)?;
        client_a.key("Escape")?;
        client_b.stop();
        server.wait_line(
            &["MP_DEDICATED_SESSION_RESET", "travel=1"],
            Duration::from_secs(60),
            at_reset,
        )?;
        let next = server.wait_line(
            &["MP_MATCH_AWAITING_PARTICIPANTS"],
            Duration::from_secs(90),
            at_reset,
        )?;
        let generation2 = field(&next, "generation").unwrap_or("0").to_owned();
        let text = server.text();
        let tail = &text[at_reset.min(text.len())..];
        let whole = tail
            .lines()
            .filter(|l| {
                l.contains("RTS_POWER_START")
                    && l.contains(&format!("generation={generation2}"))
                    && field(l, "producer_powered") == Some("1")
            })
            .count();
        report.verdict(
            "BLD-P15-LEDGERS-WHOLE-AND-THE-NEXT-MATCH-FRESH",
            generation2 != generation
                && field(&cleared_1, "reconciles") == Some("1")
                && field(&cleared_2, "reconciles") == Some("1")
                && whole == 2
                && !tail.lines().any(|l| l.contains("RTS_BUILDING_DAMAGE")),
            format!(
                "generation {generation} then {generation2}: {whole} powered seats | {} | {}",
                cleared_1.trim(),
                cleared_2.trim()
            ),
        );

        // Repeatability: which buildings went down, came back and changed hands, in order. The
        // amounts refunded depend on how far training got, so they are left out.
        let mut trace = String::new();
        for line in text.lines().filter(|l| l.contains("RTS_BUILDING_")) {
            let parts: Vec<String> = ["participant", "from", "to", "kind", "captured", "new_id"]
                .iter()
                .filter_map(|k| Some(format!("{k}={}", field(line, k)?)))
                .collect();
            let tag = [
                "RTS_BUILDING_DEMOLISH",
                "RTS_BUILDING_REPAIR participant=",
                "RTS_BUILDING_CAPTURE unit=",
            ]
            .iter()
            .find(|t| line.contains(**t));
            if let Some(tag) = tag {
                trace.push_str(&format!("{tag} {}\n", parts.join(" ")));
            }
        }
        fs::write(evidence.join("buildings-trace.txt"), &trace).map_err(|e| e.to_string())?;
        report.artifact("buildings_trace", &evidence.join("buildings-trace.txt"));
        Ok(())
    })();
    launch_options_verdict(
        report,
        "BLD-P00-LAUNCH-OPTIONS",
        &[&server, &client_a, &client_b],
    );
    client_a.stop();
    client_b.stop();
    server.stop();
    result
}

/// Pans to where a unit is expected, then selects it with real input.
fn select_entity_near_probe(game: &mut Game, unit: u64, probe: usize) -> Result<(), String> {
    pan_to_probe(game, probe)?;
    thread::sleep(Duration::from_millis(1200));
    select_entity(game, "MP_CLIENT_SAMPLE", unit)
}

// `--food SERVER CLIENT [--evidence DIR] [--port N]` (ECON-E3) plays one food shortage with real
// input. Seat 1 starts with two rations, so step 200 empties the reserve and step 400 cannot be
// paid. Seat 2 keeps the normal reserve. Two purchases, the shortage, a refused purchase, N during
// the shortage, recovery at the next full issue, the held unit, training afterwards, a match end
// and the next generation.
const FOOD_START_CENTS: i64 = 200_000;
const FOOD_FIXTURE_RATIONS: i64 = 2;
const FOOD_NORMAL_RATIONS: i64 = 240;
const FOOD_INTERVAL_STEPS: i64 = 200;
const FOOD_PACK_RATIONS: i64 = 50;
const FOOD_PACK_CENTS: i64 = 5_000;

/// A participant's upkeep lines in one generation, each exactly one interval after the last, with
/// the reserve following from its start and the purchases.
fn food_replay(
    text: &str,
    participant: u32,
    generation: &str,
    start: i64,
) -> Result<usize, String> {
    let tag = format!("RTS_FOOD_TX participant={participant} ");
    let generation = format!("generation={generation} ");
    let mut rations = start;
    let mut issues = 0_i64;
    for line in text
        .lines()
        .filter(|l| l.contains(&tag) && l.contains(&generation))
    {
        match field(line, "kind") {
            Some("purchase") => rations += FOOD_PACK_RATIONS,
            Some("harvest") => rations += econ_int(line, "paid"),
            Some("upkeep") => {
                issues += 1;
                if econ_int(line, "step") != issues * FOOD_INTERVAL_STEPS {
                    return Err(format!("upkeep {issues} off its cadence: {}", line.trim()));
                }
                let (owed, paid) = (econ_int(line, "owed"), econ_int(line, "paid"));
                if paid != owed.min(rations) {
                    return Err(format!(
                        "expected to pay {} of {owed}: {}",
                        owed.min(rations),
                        line.trim()
                    ));
                }
                rations -= paid;
                if field(line, "shortage") != Some(if paid < owed { "1" } else { "0" }) {
                    return Err(format!("wrong shortage flag: {}", line.trim()));
                }
            }
            other => return Err(format!("unknown kind {other:?}: {}", line.trim())),
        }
        if econ_int(line, "rations") != rations || field(line, "reconciles") != Some("1") {
            return Err(format!("expected {rations} rations: {}", line.trim()));
        }
    }
    Ok(issues as usize)
}

fn food_session(options: &DedicatedOptions, report: &mut Report) -> Result<(), String> {
    let evidence = options.client.evidence.clone();
    let (mut server, mut client_a, mut client_b, _display, generation) = economy_start_session(
        options,
        &[
            format!("-PolitikaMPEconomyStartCents={FOOD_START_CENTS}"),
            format!("-PolitikaMPEconomyStartFood=1:{FOOD_FIXTURE_RATIONS}"),
        ],
    )?;
    report.fact("server_command", server.command.join(" "));
    report.fact("client_a_command", client_a.command.join(" "));
    report.fact("client_b_command", client_b.command.join(" "));

    let result = (|| -> Result<(), String> {
        let text = server.text();
        let starts: Vec<&str> = text
            .lines()
            .filter(|l| {
                l.contains("RTS_FOOD_START") && l.contains(&format!("generation={generation}"))
            })
            .collect();
        report.verdict(
            "FOOD-P01-RESERVES-AT-THE-START",
            starts.len() == 2
                && econ_int(starts[0], "rations") == FOOD_FIXTURE_RATIONS
                && field(starts[0], "participant") == Some("1")
                && econ_int(starts[1], "rations") == FOOD_NORMAL_RATIONS
                && field(starts[1], "participant") == Some("2"),
            starts.join(" | "),
        );
        zoom_out(&client_a, 5)?;
        thread::sleep(Duration::from_millis(800));
        report.artifact(
            "client_a_start_png",
            &client_a.screenshot(&evidence, "01-start")?,
        );

        // Two purchases while fed; the second waits behind the first.
        let (first, _) = economy_key_outcome(
            &mut client_a,
            &mut server,
            None,
            &["RTS_ECONOMY_TX participant=1 kind=train"],
        )?;
        let admitted = econ_int(&first, "step");
        let (second, _) = economy_key_outcome(
            &mut client_a,
            &mut server,
            None,
            &["RTS_ECONOMY_TX participant=1 kind=train"],
        )?;

        // Step 200 pays in full and empties the reserve; step 400 cannot be paid.
        let paid = server.wait(
            &["RTS_FOOD_TX participant=1 kind=upkeep", "step=200"],
            Duration::from_secs(30),
        )?;
        report.verdict(
            "FOOD-P02-DUE-UPKEEP-CHARGED-ONCE",
            econ_int(&paid, "owed") == FOOD_FIXTURE_RATIONS
                && econ_int(&paid, "paid") == FOOD_FIXTURE_RATIONS
                && econ_int(&paid, "rations") == 0
                && field(&paid, "shortage") == Some("0"),
            paid.trim(),
        );
        let short = server.wait(
            &["RTS_FOOD_TX participant=1 kind=upkeep", "shortage=1"],
            Duration::from_secs(30),
        )?;
        let short_step = econ_int(&short, "step");
        let held = econ_int(&short, "head_remaining");
        // Normally the first unit has appeared by then, so the force is three and it has joined the
        // upkeep set. On a slow start it is still in training, held like the second.
        let early = admitted + (ECONOMY_TRAIN_STEPS as i64) < short_step;
        let force = if early { 3 } else { 2 };
        report.verdict(
            "FOOD-P03-SHORTAGE-STOPS-AT-ZERO",
            short_step == 2 * FOOD_INTERVAL_STEPS
                && econ_int(&short, "owed") == force
                && econ_int(&short, "paid") == 0
                && econ_int(&short, "rations") == 0
                && econ_int(&short, "queued") == if early { 1 } else { 2 },
            format!("{} | {} | {}", first.trim(), second.trim(), short.trim()),
        );
        client_a.wait(
            &["RTS_ECONOMY_VIEW participant=1", "shortage=1"],
            Duration::from_secs(5),
        )?;
        thread::sleep(Duration::from_millis(600));
        report.artifact(
            "client_a_shortage_png",
            &client_a.screenshot(&evidence, "02-shortage")?,
        );

        let (refused, refused_client) = economy_key_outcome(
            &mut client_a,
            &mut server,
            None,
            &["RTS_ECONOMY_REJECT participant=1", "op=0"],
        )?;
        report.verdict(
            "FOOD-P04-SHORTAGE-REFUSES-TRAINING",
            field(&refused, "rejection") == Some("food_shortage")
                && field(&refused_client, "accepted") == Some("0")
                && econ_int(&refused_client, "amount") == 0,
            format!("{} | client {}", refused.trim(), refused_client.trim()),
        );
        thread::sleep(Duration::from_millis(400));
        report.artifact(
            "client_a_refused_png",
            &client_a.screenshot(&evidence, "03-refused")?,
        );

        // N works during the shortage, and the purchase alone does not end it.
        let (client_mark, server_mark) = (client_a.offset(), server.offset());
        client_a.key("n")?;
        let bought_money = server.wait_line(
            &["RTS_ECONOMY_TX participant=1 kind=food"],
            Duration::from_secs(5),
            server_mark,
        )?;
        let bought = server.wait_line(
            &["RTS_FOOD_TX participant=1 kind=purchase"],
            Duration::from_secs(5),
            server_mark,
        )?;
        let bought_client = client_a.wait_line(
            &["RTS_ECONOMY_OUTCOME participant=1", "accepted=1"],
            Duration::from_secs(5),
            client_mark,
        )?;
        let bought_step = econ_int(&bought, "step");
        report.verdict(
            "FOOD-P05-PURCHASE-OPEN-IN-A-SHORTAGE",
            econ_int(&bought_money, "amount") == FOOD_PACK_CENTS
                && field(&bought_money, "reconciles") == Some("1")
                && econ_int(&bought, "rations") == FOOD_PACK_RATIONS
                && field(&bought, "shortage") == Some("1")
                && bought_step < short_step + FOOD_INTERVAL_STEPS
                && econ_int(&bought_client, "amount") == FOOD_PACK_CENTS,
            format!(
                "{} | {} | client {}",
                bought_money.trim(),
                bought.trim(),
                bought_client.trim()
            ),
        );
        thread::sleep(Duration::from_millis(400));
        report.artifact(
            "client_a_bought_png",
            &client_a.screenshot(&evidence, "04-bought")?,
        );

        // The next due issue is paid in full, the shortage ends and the same entry resumes.
        let recovered = server.wait(
            &[
                "RTS_FOOD_TX participant=1 kind=upkeep",
                "shortage=0",
                "step=600",
            ],
            Duration::from_secs(30),
        )?;
        report.verdict(
            "FOOD-P06-NEXT-FULL-ISSUE-ENDS-IT",
            econ_int(&recovered, "owed") == force
                && econ_int(&recovered, "paid") == force
                && econ_int(&recovered, "rations") == FOOD_PACK_RATIONS - force
                && econ_int(&recovered, "head_remaining") == held,
            format!("short {} | recovered {}", short.trim(), recovered.trim()),
        );
        let at_recovery = server.offset();
        let spawn = server.wait_line(
            &["RTS_PRODUCTION_SPAWN participant=1", "transaction=2"],
            Duration::from_secs(30),
            at_recovery,
        )?;
        let spawned = server.text();
        let first_spawn = spawned
            .lines()
            .find(|l| l.contains("RTS_PRODUCTION_SPAWN participant=1"))
            .unwrap_or_default()
            .to_owned();
        let delay = econ_int(&spawn, "step") - admitted;
        let lost = econ_int(&recovered, "step") - short_step;
        report.verdict(
            "FOOD-P07-HELD-UNIT-LATE-BY-THE-SHORTAGE",
            econ_int(&first_spawn, "step")
                == admitted + ECONOMY_TRAIN_STEPS as i64 + if early { 0 } else { lost }
                && delay == 2 * ECONOMY_TRAIN_STEPS as i64 + lost,
            format!(
                "first {}; second {delay} steps after the first purchase, {lost} of them held: {}",
                first_spawn.trim(),
                spawn.trim()
            ),
        );
        thread::sleep(Duration::from_millis(600));
        report.artifact(
            "client_a_recovered_png",
            &client_a.screenshot(&evidence, "05-recovered")?,
        );
        let (again, _) = economy_key_outcome(
            &mut client_a,
            &mut server,
            None,
            &["RTS_ECONOMY_TX participant=1 kind=train"],
        )?;
        report.verdict(
            "FOOD-P08-TRAINING-WORKS-AFTER-RECOVERY",
            econ_int(&again, "amount") == ECONOMY_PRICE_CENTS
                && econ_int(&again, "queued") == 1
                && field(&again, "reconciles") == Some("1"),
            again.trim(),
        );

        // Nobody starved, the other seat never noticed and neither client saw the other's food.
        let text = server.text();
        let (text_a, text_b) = (client_a.text(), client_b.text());
        let enemy_short = text
            .lines()
            .filter(|l| l.contains("RTS_FOOD_TX participant=2") && l.contains("shortage=1"))
            .count();
        let deaths = text
            .lines()
            .filter(|l| l.contains("RTS_GAMEPLAY_EVENT") && l.contains("kind=UnitDestroyed"))
            .count();
        let a_leaks = text_a
            .lines()
            .filter(|l| l.contains("RTS_ECONOMY_VIEW participant=2"))
            .count();
        let b_leaks = text_b
            .lines()
            .filter(|l| l.contains("RTS_ECONOMY_VIEW participant=1"))
            .count();
        let b_short = text_b
            .lines()
            .filter(|l| l.contains("RTS_ECONOMY_VIEW") && l.contains("shortage=1"))
            .count();
        report.verdict(
            "FOOD-P09-NO-HARM-AND-OWNER-ONLY",
            enemy_short == 0 && deaths == 0 && a_leaks == 0 && b_leaks == 0 && b_short == 0,
            format!(
                "enemy shortages {enemy_short}, units destroyed {deaths}; A's copies of B's view {a_leaks}, B's copies of A's view {b_leaks}, B short views {b_short}"
            ),
        );
        let replay_1 = food_replay(&text, 1, &generation, FOOD_FIXTURE_RATIONS);
        let replay_2 = food_replay(&text, 2, &generation, FOOD_NORMAL_RATIONS);
        report.verdict(
            "FOOD-P10-RESERVES-REPLAY",
            replay_1.as_ref().is_ok_and(|n| *n >= 3) && replay_2 == replay_1,
            format!("issues to participant 1 {replay_1:?}, participant 2 {replay_2:?}"),
        );
        let lines_1 = economy_lines(&text, 1, &generation);
        let replay = economy_replay(&lines_1, FOOD_START_CENTS);
        report.verdict(
            "FOOD-P11-MONEY-REPLAYS",
            replay.is_ok()
                && lines_1
                    .iter()
                    .filter(|l| field(l, "kind") == Some("food"))
                    .count()
                    == 1,
            format!("{} money lines; {replay:?}", lines_1.len()),
        );

        // Repeatability: the normalized sequence, without timing or income interleaving.
        let mut trace: String = lines_1
            .iter()
            .filter(|l| field(l, "kind") != Some("income"))
            .map(|l| {
                format!(
                    "{} {} queued={}\n",
                    field(l, "kind").unwrap_or("?"),
                    econ_int(l, "amount"),
                    econ_int(l, "queued")
                )
            })
            .collect();
        for line in text.lines().filter(|l| {
            l.contains("RTS_FOOD_TX participant=1 kind=upkeep")
                && econ_int(l, "step") <= 3 * FOOD_INTERVAL_STEPS
        }) {
            trace.push_str(&format!(
                "upkeep step={} owed={} paid={} rations={} shortage={}\n",
                econ_int(line, "step"),
                econ_int(line, "owed"),
                econ_int(line, "paid"),
                econ_int(line, "rations"),
                field(line, "shortage").unwrap_or("?")
            ));
        }
        trace.push_str(&format!(
            "held_steps={lost} late_by={}\n",
            delay - 2 * ECONOMY_TRAIN_STEPS as i64
        ));
        fs::write(evidence.join("food-trace.txt"), &trace).map_err(|e| e.to_string())?;
        report.artifact("food_trace", &evidence.join("food-trace.txt"));

        // A match end, then a next generation that owes nothing to the shortage.
        let owned: Vec<u64> = entity_owners(&server.text())
            .iter()
            .filter(|(_, owner)| *owner == 1)
            .map(|(id, _)| *id)
            .collect();
        let unit = *owned.first().ok_or("no authored unit for participant 1")?;
        select_entity(&mut client_a, "MP_CLIENT_SAMPLE", unit)?;
        thread::sleep(Duration::from_millis(600));
        let objective = wait_screen(
            &mut client_a,
            |t| objective_screen(t, "MP_CLIENT_OBJECTIVE"),
            "objective on client A",
        )?;
        let at_move = server.offset();
        client_a.click(objective.0, objective.1, "3")?;
        server.wait_line(
            &[
                "RTS_ORDER_ADMITTED order=Move",
                &format!("unit={unit}"),
                "participant=1",
            ],
            Duration::from_secs(5),
            at_move,
        )?;
        server.wait(
            &[
                "MATCH_RESULTS outcome=objective_secured",
                &format!("generation={generation}"),
            ],
            Duration::from_secs(180),
        )?;
        let cleared = server.wait(
            &["RTS_ECONOMY_CLEARED participant=1", "reason=match_ended"],
            Duration::from_secs(5),
        )?;
        let at_reset = server.offset();
        client_a.hover(640.0, 360.0)?;
        client_a.key("Escape")?;
        client_b.stop();
        server.wait_line(
            &["MP_DEDICATED_SESSION_RESET", "travel=1"],
            Duration::from_secs(60),
            at_reset,
        )?;
        let next = server.wait_line(
            &["MP_MATCH_AWAITING_PARTICIPANTS"],
            Duration::from_secs(90),
            at_reset,
        )?;
        let generation2 = field(&next, "generation").unwrap_or("0").to_owned();
        let text = server.text();
        let tail = &text[at_reset.min(text.len())..];
        let starts2: Vec<&str> = tail
            .lines()
            .filter(|l| {
                l.contains("RTS_FOOD_START") && l.contains(&format!("generation={generation2}"))
            })
            .collect();
        report.verdict(
            "FOOD-P12-NEXT-GENERATION-FRESH",
            generation2 != generation
                && field(&cleared, "reconciles") == Some("1")
                && starts2.len() == 2
                && econ_int(starts2[0], "rations") == FOOD_FIXTURE_RATIONS
                && econ_int(starts2[1], "rations") == FOOD_NORMAL_RATIONS
                && !tail.lines().any(|l| l.contains("RTS_FOOD_TX")),
            format!(
                "generation {generation} then {generation2}: {} | {}",
                cleared.trim(),
                starts2.join(" | ")
            ),
        );
        Ok(())
    })();
    launch_options_verdict(
        report,
        "FOOD-P00-LAUNCH-OPTIONS",
        &[&server, &client_a, &client_b],
    );
    client_a.stop();
    client_b.stop();
    server.stop();
    result
}

fn economy_limits_session(options: &DedicatedOptions, report: &mut Report) -> Result<(), String> {
    let evidence = options.client.evidence.clone();
    let extra = [
        "-PolitikaMPUnitScale=15".to_owned(),
        "-PolitikaMPUnitScaleHold".to_owned(),
        format!("-PolitikaMPEconomyStartCents={ECONOMY_LIMITS_CENTS}"),
    ];
    let (mut server, mut client_a, mut client_b, _display, generation) =
        economy_start_session(options, &extra)?;
    report.fact("server_command", server.command.join(" "));
    let result = (|| -> Result<(), String> {
        launch_options_verdict(
            report,
            "ECON-L00-LAUNCH-OPTIONS",
            &[&server, &client_a, &client_b],
        );
        zoom_out(&client_a, 5)?;
        let mut last = String::new();
        for index in 0..5 {
            let (line, _) = economy_key_outcome(
                &mut client_a,
                &mut server,
                None,
                &[
                    "RTS_ECONOMY_TX participant=1 kind=train",
                    &format!("queued={} ", index + 1),
                ],
            )?;
            last = line;
        }
        let (full, full_client) = economy_key_outcome(
            &mut client_a,
            &mut server,
            None,
            &["RTS_ECONOMY_REJECT participant=1", "rejection=queue_full"],
        )?;
        report.verdict(
            "ECON-L01-QUEUE-LIMIT",
            field(&full_client, "rejection") == Some("9") && econ_int(&last, "queued") == 5,
            format!(
                "{} | {} | client {}",
                last.trim(),
                full.trim(),
                full_client.trim()
            ),
        );
        report.artifact(
            "client_a_queue_full_png",
            &client_a.screenshot(&evidence, "01-queue-full")?,
        );
        let spawn = server.wait(
            &["RTS_PRODUCTION_SPAWN participant=1"],
            Duration::from_secs(20),
        )?;
        let (cap, cap_client) = economy_key_outcome(
            &mut client_a,
            &mut server,
            None,
            &["RTS_ECONOMY_REJECT participant=1", "rejection=unit_cap"],
        )?;
        report.verdict(
            "ECON-L02-UNIT-LIMIT-COUNTS-QUEUE",
            econ_int(&spawn, "living") == 16 && field(&cap_client, "rejection") == Some("10"),
            format!(
                "{} | {} | client {}",
                spawn.trim(),
                cap.trim(),
                cap_client.trim()
            ),
        );
        for remaining in (0..4).rev() {
            let (refund, _) = economy_key_outcome(
                &mut client_a,
                &mut server,
                Some("shift"),
                &[
                    "RTS_ECONOMY_TX participant=1 kind=refund",
                    &format!("queued={remaining} "),
                ],
            )?;
            last = refund;
        }
        let (nothing, nothing_client) = economy_key_outcome(
            &mut client_a,
            &mut server,
            Some("shift"),
            &[
                "RTS_ECONOMY_REJECT participant=1",
                "rejection=nothing_to_cancel",
            ],
        )?;
        let lines = economy_lines(&server.text(), 1, &generation);
        let replay = economy_replay(&lines, ECONOMY_LIMITS_CENTS);
        report.verdict(
            "ECON-L03-CANCEL-EMPTIES-THE-QUEUE",
            field(&nothing_client, "rejection") == Some("11") && replay.is_ok(),
            format!("{} | {} | replay {replay:?}", last.trim(), nothing.trim()),
        );
        let spent = econ_int(&last, "spent") - econ_int(&last, "refunded");
        report.verdict(
            "ECON-L04-ONE-PRICE-FOR-ONE-UNIT",
            spent == ECONOMY_PRICE_CENTS,
            format!(
                "net spending {spent} after five purchases, four refunds and one unit: {}",
                last.trim()
            ),
        );
        thread::sleep(Duration::from_millis(500));
        report.artifact(
            "client_a_limits_png",
            &client_a.screenshot(&evidence, "02-after")?,
        );
        Ok(())
    })();
    client_a.stop();
    client_b.stop();
    server.stop();
    result
}

fn run_dedicated_session(
    flag: &str,
    schema: &'static str,
    name: &str,
    session: DedicatedSession,
) -> ! {
    let mut options = match parse_dedicated_options() {
        Ok(options) => options,
        Err(error) => {
            eprintln!("{}", error.replace("--dedicated", flag));
            std::process::exit(2);
        }
    };
    match fs::create_dir_all(&options.client.evidence)
        .and_then(|_| options.client.evidence.canonicalize())
    {
        Ok(absolute) => options.client.evidence = absolute,
        Err(error) => {
            eprintln!("evidence directory: {error}");
            std::process::exit(2);
        }
    }
    let mut report = Report {
        schema: Some(schema),
        ..Report::default()
    };
    for (name, binary) in [
        ("server", &options.server),
        ("client", &options.client.binary),
    ] {
        report.fact(&format!("{name}_binary"), binary.display().to_string());
        if let Ok(digest) = sha256(binary) {
            report.fact(&format!("{name}_executable_sha256"), digest);
        }
        if let Some(archive) = binary.ancestors().nth(4)
            && let Ok(identity) = fs::read_to_string(archive.join("BuildIdentity.json"))
        {
            report.fact(
                &format!("{name}_build_identity"),
                identity.trim().to_owned(),
            );
        }
    }
    if let Err(error) = session(&options, &mut report) {
        report.mark(
            &format!("SESSION-{name}"),
            if error.starts_with("BLOCKED") {
                "BLOCKED"
            } else {
                "FAIL"
            },
            error,
        );
    }
    let path = options
        .client
        .evidence
        .join(format!("{name}-evidence.json"));
    if let Err(error) = report.write(&path) {
        eprintln!("write evidence: {error}");
        std::process::exit(2);
    }
    println!("evidence: {}", path.display());
    std::process::exit(if report.failed() { 1 } else { 0 });
}

fn main() {
    if env::args().nth(1).as_deref() == Some("--fog-memory") {
        run_dedicated_session(
            "--fog-memory",
            "politika.mp2.fog-memory-evidence.v1",
            "fog-memory",
            fog_memory_session,
        );
    }
    if env::args().nth(1).as_deref() == Some("--engagement") {
        run_dedicated_session(
            "--engagement",
            "politika.mp2.unaligned-engagement-evidence.v1",
            "engagement",
            engagement_session,
        );
    }
    if env::args().nth(1).as_deref() == Some("--group-orders") {
        run_dedicated_session(
            "--group-orders",
            "politika.grp.group-orders-evidence.v1",
            "group-orders",
            group_orders_session,
        );
    }
    if env::args().nth(1).as_deref() == Some("--control-groups") {
        run_dedicated_session(
            "--control-groups",
            "politika.grp.control-groups-evidence.v1",
            "control-groups",
            control_groups_session,
        );
    }
    if env::args().nth(1).as_deref() == Some("--group-move-shape") {
        run_dedicated_session(
            "--group-move-shape",
            "politika.form.group-move-shape-evidence.v1",
            "group-move-shape",
            group_move_shape_session,
        );
    }
    if env::args().nth(1).as_deref() == Some("--group-move-facing") {
        run_dedicated_session(
            "--group-move-facing",
            "politika.form.group-move-facing-evidence.v1",
            "group-move-facing",
            group_move_facing_session,
        );
    }
    if env::args().nth(1).as_deref() == Some("--pointer-stall") {
        run_dedicated_session(
            "--pointer-stall",
            "politika.rts.pointer-stall-evidence.v1",
            "pointer-stall",
            pointer_stall_session,
        );
    }
    if env::args().nth(1).as_deref() == Some("--power") {
        run_dedicated_session(
            "--power",
            "politika.econ.power-evidence.v1",
            "power",
            power_session,
        );
    }
    if env::args().nth(1).as_deref() == Some("--buildings") {
        run_dedicated_session(
            "--buildings",
            "politika.econ.buildings-evidence.v1",
            "buildings",
            buildings_session,
        );
    }
    if env::args().nth(1).as_deref() == Some("--harvest") {
        run_dedicated_session(
            "--harvest",
            "politika.econ.harvest-evidence.v1",
            "harvest",
            harvest_session,
        );
    }
    if env::args().nth(1).as_deref() == Some("--gold") {
        run_dedicated_session(
            "--gold",
            "politika.econ.gold-evidence.v1",
            "gold",
            gold_session,
        );
    }
    if env::args().nth(1).as_deref() == Some("--food") {
        run_dedicated_session(
            "--food",
            "politika.econ.food-evidence.v1",
            "food",
            food_session,
        );
    }
    if env::args().nth(1).as_deref() == Some("--compare-normalized") {
        let (Some(a), Some(b)) = (env::args().nth(2), env::args().nth(3)) else {
            eprintln!(
                "usage: rts_multiplayer --compare-normalized RUN1/normalized.txt RUN2/normalized.txt"
            );
            std::process::exit(2);
        };
        let (Ok(left), Ok(right)) = (fs::read_to_string(&a), fs::read_to_string(&b)) else {
            eprintln!("FAIL: cannot read {a} or {b}");
            std::process::exit(2);
        };
        let problems = compare_normalized(&left, &right);
        for problem in &problems {
            println!("DIFF {problem}");
        }
        println!(
            "{} normalized comparison: {} lines, {} differences",
            if problems.is_empty() { "PASS" } else { "FAIL" },
            left.lines().count(),
            problems.len()
        );
        std::process::exit(i32::from(!problems.is_empty()));
    }
    if env::args().nth(1).as_deref() == Some("--formation-lifecycle") {
        run_dedicated_session(
            "--formation-lifecycle",
            "politika.form.formation-lifecycle-evidence.v1",
            "formation-lifecycle",
            formation_lifecycle_session,
        );
    }
    if env::args().nth(1).as_deref() == Some("--formation-stance") {
        run_dedicated_session(
            "--formation-stance",
            "politika.form.formation-stance-evidence.v1",
            "formation-stance",
            formation_stance_session,
        );
    }
    if env::args().nth(1).as_deref() == Some("--formation-discipline") {
        run_dedicated_session(
            "--formation-discipline",
            "politika.disc.formation-discipline-evidence.v1",
            "formation-discipline",
            formation_discipline_session,
        );
    }
    if env::args().nth(1).as_deref() == Some("--discipline-stress") {
        run_dedicated_session(
            "--discipline-stress",
            "politika.disc.discipline-stress-evidence.v1",
            "discipline-stress",
            discipline_stress_session,
        );
    }
    if env::args().nth(1).as_deref() == Some("--discipline-effects") {
        run_dedicated_session(
            "--discipline-effects",
            "politika.disc.discipline-effects-evidence.v1",
            "discipline-effects",
            discipline_effects_session,
        );
    }
    if env::args().nth(1).as_deref() == Some("--discipline-rout") {
        run_dedicated_session(
            "--discipline-rout",
            "politika.disc.discipline-rout-evidence.v1",
            "discipline-rout",
            discipline_rout_session,
        );
    }
    if env::args().nth(1).as_deref() == Some("--economy") {
        run_dedicated_session(
            "--economy",
            "politika.econ.economy-evidence.v1",
            "economy",
            economy_session,
        );
    }
    if env::args().nth(1).as_deref() == Some("--economy-limits") {
        run_dedicated_session(
            "--economy-limits",
            "politika.econ.economy-limits-evidence.v1",
            "economy-limits",
            economy_limits_session,
        );
    }
    if env::args().nth(1).as_deref() == Some("--unit-scale") {
        run_unit_scale();
    }
    if env::args().nth(1).as_deref() == Some("--dedicated") {
        run_dedicated();
    }
    if env::args().nth(1).as_deref() == Some("--dedicated-layouts") {
        run_layouts(false);
    }
    if env::args().nth(1).as_deref() == Some("--listen-layouts") {
        run_layouts(true);
    }
    if env::args().nth(1).as_deref() == Some("--shipping-boot") {
        let (Some(binary), Some(evidence)) = (env::args().nth(2), env::args().nth(3)) else {
            eprintln!("usage: rts_multiplayer --shipping-boot SHIPPING_BINARY EVIDENCE_DIR");
            std::process::exit(2);
        };
        let evidence = PathBuf::from(evidence);
        match shipping_boot(Path::new(&binary), &evidence) {
            Ok(report) => {
                let path = evidence.join("shipping-evidence.json");
                let _ = report.write(&path);
                println!("evidence: {}", path.display());
                std::process::exit(if report.failed() { 1 } else { 0 });
            }
            Err(error) => {
                eprintln!("FAIL shipping boot: {error}");
                std::process::exit(1);
            }
        }
    }
    let mut options = match parse_options() {
        Ok(options) => options,
        Err(error) => {
            eprintln!("{error}");
            std::process::exit(2);
        }
    };
    // Game processes change directory at startup; every log path must be absolute.
    match fs::create_dir_all(&options.evidence).and_then(|_| options.evidence.canonicalize()) {
        Ok(absolute) => options.evidence = absolute,
        Err(error) => {
            eprintln!("evidence directory: {error}");
            std::process::exit(2);
        }
    }
    let mut report = Report::default();
    report.fact("binary", options.binary.display().to_string());
    if let Ok(digest) = sha256(&options.binary) {
        report.fact("executable_sha256", digest);
    }
    if let Some(archive) = options.binary.ancestors().nth(4) {
        if let Ok(identity) = fs::read_to_string(archive.join("BuildIdentity.json")) {
            report.fact("build_identity", identity.trim().to_owned());
        }
        for container in [
            "Politika/Content/Paks/Politika-Linux.utoc",
            "Politika/Content/Paks/Politika-Linux.ucas",
        ] {
            if let Ok(digest) = sha256(&archive.join(container)) {
                report.fact(container, digest);
            }
        }
    }
    report.fact("layout", options.layout.clone());
    report.fact("port", options.port.to_string());
    for (session, run) in [
        (
            "gameplay",
            session_gameplay as fn(&Options, &mut Report) -> Result<(), String>,
        ),
        ("host-loss", session_host_loss),
    ] {
        if let Err(error) = run(&options, &mut report) {
            report.mark(
                &format!("SESSION-{session}"),
                if error.starts_with("BLOCKED") {
                    "BLOCKED"
                } else {
                    "FAIL"
                },
                error,
            );
        }
    }
    let path = options.evidence.join("final-evidence.json");
    if let Err(error) = report.write(&path) {
        eprintln!("write evidence: {error}");
        std::process::exit(2);
    }
    println!("evidence: {}", path.display());
    std::process::exit(if report.failed() { 1 } else { 0 });
}

#[cfg(test)]
mod tests {
    #[test]
    fn gold_replay_follows_the_ledger_and_refuses_a_mismatch() {
        let tx = |kind: &str,
                  amount: i64,
                  source: i64,
                  carried: i64,
                  held: i64,
                  delivered: i64,
                  sold: i64| {
            format!(
                "RTS_HARVEST_TX participant=1 kind={kind} resource=gold unit=1 amount={amount} gross=6250 fee=188 source={source} carried={carried} held={held} delivered={delivered} sold={sold} lost=0 harvesters=1 reconciles=1 generation=1 step=1"
            )
        };
        let good = [
            tx("harvester", 0, 60, 0, 0, 0, 0),
            tx("load", 25, 35, 25, 0, 0, 0),
            tx("detached", 0, 35, 25, 0, 0, 0),
            tx("deliver", 25, 35, 0, 25, 25, 0),
            tx("sale", 25, 35, 0, 0, 25, 25),
        ]
        .join("\n");
        assert_eq!(super::harvest_replay(&good, 1, "1", "gold", 60), Ok(5));
        assert_eq!(super::harvest_replay(&good, 2, "1", "gold", 60), Ok(0));
        // A delivery that creates gold, and a line that says it does not reconcile.
        let created = tx("load", 25, 35, 25, 0, 0, 0) + "\n" + &tx("deliver", 40, 35, 0, 40, 40, 0);
        assert!(super::harvest_replay(&created, 1, "1", "gold", 60).is_err());
        let unreconciled = tx("load", 25, 35, 25, 0, 0, 0).replace("reconciles=1", "reconciles=0");
        assert!(super::harvest_replay(&unreconciled, 1, "1", "gold", 60).is_err());
        // Selling gold that was never delivered, and a routine line that moves gold.
        assert!(
            super::harvest_replay(&tx("sale", 25, 60, 0, -25, 0, 25), 1, "1", "gold", 60).is_err()
        );
        assert!(
            super::harvest_replay(&tx("waiting", 5, 60, 0, 0, 0, 0), 1, "1", "gold", 60).is_err()
        );
    }

    use super::{
        ENGAGE_FACING_POST, ENGAGE_WITHHELD_POST, FACING_PROBES, SIGHT_CM, compare_normalized,
        contact_memory_audit, distance, drag_expect, frame_extent, frame_windows,
        host_contact_audit, host_shot_cue_audit, log_time, phase_traffic, probe_screen,
        timed_net_samples,
    };

    #[test]
    fn drag_expectation_follows_the_formation_contract() {
        let min = (-2000.0, -2000.0);
        // Left to right on screen is +X to -X in the world: facing +Y, five files for 600 cm.
        let line = drag_expect(FACING_PROBES[1], FACING_PROBES[2], min, 150.0, 20, true).unwrap();
        assert!(distance(line.facing, (0.0, 1.0)) < 1e-12);
        assert!(distance(line.anchor, (-1450.0 + 2000.0, -1000.0 + 2000.0)) < 1e-9);
        assert_eq!(line.frontage, 5);
        // Reversed: same midpoint and width, opposite facing.
        let back = drag_expect(FACING_PROBES[2], FACING_PROBES[1], min, 150.0, 20, true).unwrap();
        assert_eq!((back.anchor, back.frontage), (line.anchor, line.frontage));
        assert!(distance(back.facing, (0.0, -1.0)) < 1e-12);
        // Halves round up; under one spacing keeps the default; clamped to the members.
        let at = |cm: f64, members: usize| {
            drag_expect((0.0, 0.0), (cm, 0.0), (0.0, 0.0), 150.0, members, true)
                .unwrap()
                .frontage
        };
        assert_eq!(
            (at(225.0, 20), at(224.9, 20), at(149.9, 20), at(150.0, 20)),
            (3, 2, 0, 2)
        );
        assert_eq!((at(3000.0, 8), at(1e300, 20)), (8, 20));
        // Wedge/Perimeter keep the facing only; no direction without length.
        assert_eq!(
            drag_expect((0.0, 0.0), (900.0, 0.0), (0.0, 0.0), 150.0, 20, false)
                .unwrap()
                .frontage,
            0
        );
        assert!(drag_expect((5.0, 5.0), (5.5, 5.0), (0.0, 0.0), 150.0, 20, true).is_none());
        assert!(drag_expect((0.0, 0.0), (f64::NAN, 0.0), (0.0, 0.0), 150.0, 20, true).is_none());
        // The probe drags request the files the session expects.
        let files = |a: usize, b: usize, spacing: f64| {
            drag_expect(FACING_PROBES[a], FACING_PROBES[b], min, spacing, 20, true)
                .unwrap()
                .frontage
        };
        assert_eq!(
            (
                files(9, 10, 150.0),
                files(13, 14, 150.0),
                files(15, 16, 150.0)
            ),
            (4, 5, 4)
        );
    }

    #[test]
    fn frame_extent_measures_across_and_along_the_facing() {
        let block: Vec<(f64, f64)> = (0..20)
            .map(|i| (f64::from(i % 5) * 150.0, f64::from(i / 5) * 150.0))
            .collect();
        let (across, along) = frame_extent(&block, (0.0, 1.0));
        assert!((across - 600.0).abs() < 1e-9 && (along - 450.0).abs() < 1e-9);
    }

    #[test]
    fn normalized_runs_compare_with_frozen_tolerances() {
        let a = "LINE shape=line anchor=550.000,1000.000 facing=0.000000,1.000000 frontage=5 slots=13:0:325.0:1225.0\nESC cancel=Escape sent=0";
        assert!(compare_normalized(a, a).is_empty());
        let near = a
            .replace("550.000", "550.400")
            .replace("0.000000,1.000000", "0.000900,1.000000");
        assert!(compare_normalized(a, &near).is_empty());
        assert_eq!(
            compare_normalized(a, &a.replace("550.000", "550.600")).len(),
            1
        );
        assert_eq!(
            compare_normalized(a, &a.replace("facing=0.000000", "facing=0.002000")).len(),
            1
        );
        assert_eq!(compare_normalized(a, &a.replace("13:0:", "13:1:")).len(), 1);
        assert_eq!(
            compare_normalized(a, &a.replace("sent=0", "sent=1")).len(),
            1
        );
        assert_eq!(compare_normalized(a, "LINE shape=line").len(), 2);
    }

    const HOST_SEEN: &str = "LogPolitikaStartup: Display: MP_HOST_UNIT_FOG entity=11 owner=2 team=2 hidden=0 first=0 alive=1 x=10.0 y=20.0";
    const HOST_HIDDEN: &str = "LogPolitikaStartup: Display: MP_HOST_UNIT_FOG entity=11 owner=2 team=2 hidden=1 first=0 alive=1 x=15.0 y=25.0";
    const HOST_CONTACT: &str = "LogPolitikaStartup: Display: MP_LOCAL_CONTACT_LAST_KNOWN role=host generation=1 viewer=1 viewer_team=1 entity=11 owner=2 team=2 x=15.0 y=25.0 contacts=1";

    #[test]
    fn host_contact_must_follow_its_own_fog_withdrawal() {
        let (recorded, transitions, problems) =
            host_contact_audit(&[HOST_SEEN, HOST_HIDDEN, HOST_CONTACT].join("\n"));
        assert_eq!((recorded, transitions), (1, 2));
        assert!(problems.is_empty(), "{problems:?}");
        // Never seen by the host, or remembered somewhere other than where it vanished.
        assert_eq!(
            host_contact_audit(&[HOST_HIDDEN, HOST_CONTACT].join("\n"))
                .2
                .len(),
            1
        );
        let moved = HOST_CONTACT.replace("x=15.0", "x=40.0");
        assert_eq!(
            host_contact_audit(&[HOST_SEEN, HOST_HIDDEN, &moved].join("\n"))
                .2
                .len(),
            1
        );
        let own = HOST_CONTACT.replace("team=2 x", "team=1 x");
        assert_eq!(
            host_contact_audit(&[HOST_SEEN, HOST_HIDDEN, &own].join("\n"))
                .2
                .len(),
            1
        );
    }

    #[test]
    fn host_shot_cue_must_match_the_allied_decision() {
        let host = |cue: &str, px: f64| {
            format!(
                "MP_HOST_SHOT_CUE sequence=7 participant=1 shooter=4 target=9 cue={cue} target_x=100.0 target_y=0.0 point_x={px:.1} point_y=0.0 generation=1"
            )
        };
        let ally = |cue: &str| {
            format!(
                "MP_SHOT_CUE_SENT sequence=7 participant=3 shooter=4 target=9 cue={cue} target_x=100.0 target_y=0.0 point_x=100.0 point_y=0.0 generation=1"
            )
        };
        let (total, counts, compared, problems) =
            host_shot_cue_audit(&[host("exact", 100.0), ally("exact")].join("\n"), &[3]);
        assert_eq!((total, counts, compared), (1, [1, 0, 0], 1));
        assert!(problems.is_empty(), "{problems:?}");
        assert_eq!(
            host_shot_cue_audit(&[host("exact", 100.0), ally("facing")].join("\n"), &[3])
                .3
                .len(),
            1
        );
        // A facing cue on the target, or no allied decision to compare with.
        assert_eq!(
            host_shot_cue_audit(&[host("facing", 120.0), ally("facing")].join("\n"), &[3])
                .3
                .len(),
            1
        );
        assert_eq!(host_shot_cue_audit(&host("exact", 100.0), &[3]).3.len(), 1);
    }

    const RELEVANT: &str =
        "LogPolitikaStartup: Display: MP_CLIENT_UNIT_RELEVANT entity=11 owner=0 team=3";
    const WITHDRAWN: &str = "LogPolitikaStartup: Display: MP_CLIENT_UNIT_WITHDRAWN entity=11 owner=0 team=3 alive=1 reason=0 x=0.0 y=-1050.0";
    const LAST_KNOWN: &str = "LogPolitikaStartup: Display: MP_LOCAL_CONTACT_LAST_KNOWN role=client generation=1 viewer=1 viewer_team=1 entity=11 owner=0 team=3 x=0.0 y=-1050.0 contacts=1";

    #[test]
    fn accepts_a_contact_recorded_from_a_live_withdrawal() {
        let text = [RELEVANT, WITHDRAWN, LAST_KNOWN].join("\n");
        let (recorded, deaths, problems) = contact_memory_audit(&text);
        assert_eq!((recorded, deaths), (1, 0));
        assert!(problems.is_empty(), "{problems:?}");
    }

    #[test]
    fn rejects_a_contact_the_client_was_never_sent() {
        let (_, _, problems) = contact_memory_audit(LAST_KNOWN);
        assert_eq!(problems.len(), 1);
    }

    #[test]
    fn rejects_a_contact_at_a_position_it_was_not_seen() {
        let moved = LAST_KNOWN.replace("y=-1050.0", "y=-900.0");
        let (_, _, problems) = contact_memory_audit(&[RELEVANT, WITHDRAWN, &moved].join("\n"));
        assert_eq!(problems.len(), 1);
    }

    #[test]
    fn rejects_a_contact_after_a_seen_death_or_on_the_viewers_side() {
        let death =
            "LogPolitikaStartup: Display: MP_CLIENT_UNIT_HEALTH entity=11 owner=0 health=0 max=100";
        let (_, deaths, problems) =
            contact_memory_audit(&[RELEVANT, death, WITHDRAWN, LAST_KNOWN].join("\n"));
        assert_eq!((deaths, problems.len()), (1, 1));
        let own = LAST_KNOWN.replace("team=3 x", "team=1 x");
        let (_, _, problems) = contact_memory_audit(&[RELEVANT, WITHDRAWN, &own].join("\n"));
        assert_eq!(problems.len(), 1);
    }

    #[test]
    fn ground_probe_reads_the_latest_on_screen_point_only() {
        let line = |index: u32, on: u32, x: u32| {
            format!(
                "LogPolitikaStartup: Display: MP_CLIENT_GROUND_PROBE index={index} x=300.0 y=1300.0 z=-15.0 ground_hit=1 on_screen={on} screen_x={x} screen_y=196 viewer=1"
            )
        };
        let text = [line(0, 1, 400), line(1, 1, 558), line(0, 1, 458)].join("\n");
        assert_eq!(probe_screen(&text, 0), Some((458.0, 196.0)));
        assert_eq!(probe_screen(&text, 1), Some((558.0, 196.0)));
        assert_eq!(probe_screen(&[line(0, 0, 458)].join("\n"), 0), None);
        assert_eq!(probe_screen(&text, 2), None);
    }

    fn stats(time: &str, participant: u32, elapsed: f64, out: u64, inb: u64) -> String {
        format!(
            "[2026.10.29-22.00.{time}][  1]LogPolitikaStartup: Display: MP_NET_STATS role=server reason=periodic generation=1 step=0 participant={participant} team=1 elapsed_s={elapsed:.3} in_bytes={inb} out_bytes={out} in_packets=0 out_packets=0 in_lost=0 out_lost=0 ooo_recovered=0 ooo_lost=0 ooo_duplicate=0 in_bps=0 out_bps=0 avg_lag_ms=0.0 jitter_ms=0.0 visible_units=4 net_speed=100000 channels=9"
        )
    }

    #[test]
    fn unit_scale_traffic_splits_at_the_engagement() {
        assert_eq!(
            log_time(&stats("01:000", 1, 0.0, 0, 0)),
            Some("2026.10.29-22.00.01:000")
        );
        assert_eq!(log_time("MP_NET_STATS role=server"), None);
        let text = [
            stats("01:000", 1, 0.0, 0, 0),
            stats("02:000", 1, 1.0, 1000, 100),
            stats("02:000", 2, 1.0, 9999, 9999),
            stats("03:000", 1, 2.0, 2000, 200),
            stats("04:000", 1, 3.0, 8000, 500),
            stats("05:000", 1, 4.0, 9000, 600),
        ]
        .join("\n");
        let samples = timed_net_samples(&text, "server", "1");
        assert_eq!(samples.len(), 6);
        let engage = "2026.10.29-22.00.03:500";
        let idle = phase_traffic(&samples, 1, None, Some(engage)).unwrap();
        assert_eq!(
            (idle.down, idle.up, idle.peak_down, idle.net_speed),
            (1000.0, 100.0, 1000.0, 100000.0)
        );
        // The engagement opens at the last idle sample, so the 6000 B burst belongs to it.
        let fight = phase_traffic(&samples, 1, Some(engage), None).unwrap();
        assert_eq!(
            (fight.down, fight.peak_down, fight.seconds),
            (3500.0, 6000.0, 2.0)
        );
        assert!(phase_traffic(&samples, 3, None, None).is_none());
        assert!(phase_traffic(&samples, 1, Some("2026.10.29-22.00.00:000"), None).is_none());
    }

    #[test]
    fn client_frame_windows_split_by_phase_and_skip_empty_windows() {
        let line = |time: &str, frames: u32, mean: f64, max: f64| {
            format!(
                "[2026.10.29-22.00.{time}][  1]LogPolitikaStartup: Display: MP_CLIENT_FRAME_WINDOW generation=1 participant=1 elapsed_s=1.000 frames={frames} mean_ms={mean:.2} max_ms={max:.2} present_units=4"
            )
        };
        let text = [
            line("01:000", 30, 10.0, 20.0),
            line("02:000", 0, 0.0, 0.0),
            line("03:000", 20, 30.0, 90.0),
        ]
        .join("\n");
        let engage = Some("2026.10.29-22.00.02:500");
        assert_eq!(frame_windows(&text, None, engage), Some((10.0, 20.0, 1)));
        assert_eq!(frame_windows(&text, engage, None), Some((30.0, 90.0, 1)));
        assert_eq!(
            frame_windows(&text, Some("2026.10.29-22.00.09:000"), None),
            None
        );
    }

    #[test]
    fn engagement_posts_match_the_fixture_vision_geometry() {
        // Fixture placements: northern/southern militia, team B's start units.
        let (north, south) = ((0.0, 1250.0), (0.0, -1050.0));
        let team_b = [(1350.0, 600.0), (1220.0, 780.0)];
        let standoff = distance(ENGAGE_FACING_POST, north);
        assert!((standoff - (650.0 - 100.0)).abs() < 5.0);
        assert!(team_b.iter().any(|b| {
            let d = distance(*b, ENGAGE_FACING_POST);
            d > 800.0 + 100.0 && d < SIGHT_CM - 100.0
        }));
        assert!(team_b.iter().all(|b| distance(*b, north) > SIGHT_CM));
        assert!(distance(ENGAGE_WITHHELD_POST, south) < SIGHT_CM - 300.0);
        assert!(
            team_b
                .iter()
                .all(|b| distance(*b, ENGAGE_WITHHELD_POST) > SIGHT_CM + 600.0)
        );
    }
}
