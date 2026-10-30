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
    let mut pass = games
        .iter()
        .any(|g| g.command.iter().any(|a| a.starts_with("-PolitikaMP")));
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
    vec![
        format!("/Game/Maps/RTSVerticalSlice?PolitikaLayout={layout}"),
        format!("-port={port}"),
        "-PolitikaTestFixture=MultiplayerSkirmish".into(),
        "-RTSAcceptance=multiplayer".into(),
    ]
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
    vec![
        format!("/Game/Maps/RTSVerticalSlice?listen?PolitikaLayout={layout}"),
        format!("-port={port}"),
        "-PolitikaTestFixture=MultiplayerSkirmish".into(),
        "-RTSAcceptance=multiplayer".into(),
        "-windowed".into(),
        format!("-ResX={WIDTH}"),
        format!("-ResY={HEIGHT}"),
    ]
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
/// A zoomed-out unit is small, so nearby points on its body are also tried.
fn order_attack(
    game: &mut Game,
    server: &mut Game,
    unit: u64,
    target: u64,
) -> Result<String, String> {
    select_entity(game, "MP_CLIENT_SAMPLE", unit)?;
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
            &[
                "RTS_ORDER_ADMITTED order=Attack ",
                &format!("unit={unit} "),
                &format!("target={target} "),
            ],
            Duration::from_secs(3),
            mark,
        ) {
            return Ok(line);
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
// box-selects its 20 units with real X input and right-clicks three destinations: a Line
// move clear of the control point (a capture would end the match), V then a Column move, and V then a Line move so close to the
// western playable edge that part of the nominal line is outside it. For each move the
// client's plan and per-unit slots are checked against what the server received, projected
// and routed, and against where every unit actually stopped.
// ---------------------------------------------------------------------------------------

/// Ground probe points (world cm), all well outside the control point's 300 cm capture radius
/// at (0, 150): a Line destination east of seat 1, a Column destination west of it, and a Line
/// destination 100 cm inside the western playable edge (x = -2000).
const SHAPE_LINE_POINT: (f64, f64) = (-450.0, -1000.0);
const SHAPE_COLUMN_POINT: (f64, f64) = (-1000.0, -1050.0);
const SHAPE_EDGE_POINT: (f64, f64) = (-1900.0, -650.0);
/// The planner's slot spacing (`politika::formation::Layout::spacingCm`).
const SHAPE_SPACING_CM: f64 = 150.0;

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
    /// The planner must skip at least one out-of-bounds or blocked candidate.
    expect_rejected: bool,
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
    let client_text = client.text()[before_client..].to_owned();
    let mut slots: Vec<(u64, i64, (f64, f64), (f64, f64))> = client_text
        .lines()
        .filter(|l| l.contains("RTS_GROUP_MOVE_SLOT") && field(l, "batch") == Some(batch.as_str()))
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
    let slot_units: Vec<u64> = slots.iter().map(|s| s.0).collect();
    let size = pair(&plan, "playable_size").unwrap_or((0.0, 0.0));
    let map_points: Vec<(f64, f64)> = slots.iter().map(|s| s.2).collect();
    let in_bounds = map_points
        .iter()
        .all(|p| p.0 >= 50.0 && p.1 >= 50.0 && p.0 <= size.0 - 50.0 && p.1 <= size.1 - 50.0);
    let placed = slots.iter().filter(|s| s.1 >= 0).count();
    let separation = min_separation(&map_points);
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
            && (separation - SHAPE_SPACING_CM).abs() < 0.5
            && in_bounds
            && (rejected > 0) == step.expect_rejected,
        format!(
            "{} slots for {} own units, {placed} placed, min separation {separation:.1} cm, in bounds {in_bounds}, rejected {rejected}",
            slots.len(),
            own.len()
        ),
    );

    // Geometry along the plan's facing: Line spans 10 slots across and 2 ranks deep;
    // Column spans 2 files across and 10 ranks deep (when no slot was replaced).
    if let Some(facing) = pair(&plan, "facing") {
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
        if !step.expect_rejected {
            let (want_width, want_depth) = if step.shape == "column" {
                (SHAPE_SPACING_CM, 9.0 * SHAPE_SPACING_CM)
            } else {
                (9.0 * SHAPE_SPACING_CM, SHAPE_SPACING_CM)
            };
            report.verdict(
                &format!("FORM-04-{id}-SHAPE"),
                (width - want_width).abs() < 0.5 && (depth - want_depth).abs() < 0.5,
                format!(
                    "across {width:.1} (want {want_width}), along {depth:.1} (want {want_depth})"
                ),
            );
        }
    }

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
    let ready: Vec<(u64, (f64, f64), (f64, f64))> = server_text
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
            && final_separation >= SHAPE_SPACING_CM - 20.0,
        format!(
            "{count}/{} arrived, {failed} navigation failures, {at_slot} stopped within 30 cm of their resolved slot, final min separation {final_separation:.1} cm",
            own.len()
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
    args_a.push(format!(
        "-PolitikaMPGroundProbe={}:{}/{}:{}/{}:{}",
        SHAPE_LINE_POINT.0,
        SHAPE_LINE_POINT.1,
        SHAPE_COLUMN_POINT.0,
        SHAPE_COLUMN_POINT.1,
        SHAPE_EDGE_POINT.0,
        SHAPE_EDGE_POINT.1
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
                expect_rejected: false,
            },
        )?;

        // 2. V: Column, sent nothing by itself.
        let before_v = client_a.offset();
        let before_v_server = server.offset();
        client_a.key("v")?;
        let shape =
            client_a.wait_line(&["RTS_GROUP_MOVE_SHAPE"], Duration::from_secs(5), before_v)?;
        thread::sleep(Duration::from_millis(500));
        let sent = client_a.text()[before_v..]
            .lines()
            .filter(|l| l.contains("RTS_COMMAND_INTENT_SENT"))
            .count();
        let received = server.text()[before_v_server..]
            .lines()
            .filter(|l| l.contains("MP_COMMAND_RECEIVED") && l.contains(" participant=1 "))
            .count();
        report.verdict(
            "FORM-02-SHAPE-KEY-IS-LOCAL",
            field(&shape, "shape") == Some("column") && sent == 0 && received == 0,
            format!(
                "{}; intents sent {sent}, server received {received}",
                shape.trim()
            ),
        );
        report.artifact(
            "client_a_column_hud_png",
            &client_a.screenshot(evidence, "02-column-selected")?,
        );
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
                expect_rejected: false,
            },
        )?;

        // 3. V again: Line next to the western edge; out-of-bounds slots are replaced.
        let before_v = client_a.offset();
        client_a.key("v")?;
        let shape =
            client_a.wait_line(&["RTS_GROUP_MOVE_SHAPE"], Duration::from_secs(5), before_v)?;
        report.verdict(
            "FORM-SHAPE-CYCLES-BACK-TO-LINE",
            field(&shape, "shape") == Some("line"),
            shape.trim().to_owned(),
        );
        let edge = wait_screen(&mut client_a, |t| probe_screen(t, 2), "edge probe")?;
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
                expect_rejected: true,
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
        report.verdict(
            "FORM-09-LOSS-DEGRADES-AND-SCHEDULES-ONE-REGROUP",
            lost != 0
                && lost != independent
                && lost == *followers.iter().chain([lost].iter()).max().unwrap_or(&0)
                && field(&left, "remaining") == Some("19")
                && field(&scheduled, "due_step").and_then(|v| v.parse::<u64>().ok())
                    == Some(loss_step + 40)
                && regroup_step == loss_step + 40,
            format!(
                "{} | {} | {} | {}",
                casualty.trim(),
                left.trim(),
                scheduled.trim(),
                regroup.trim()
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
            let returning = disengaged
                .iter()
                .filter(|l| l.contains(" return=1 "))
                .count();
            if (destroyed.is_ok() && returned.len() >= returning && !engaged.is_empty())
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
    report.artifact("server_log", &server.log);
    report.artifact("client_a_log", &client_a.log);
    report.artifact("client_b_log", &client_b.log);
    client_a.stop();
    client_b.stop();
    server.stop();
    drop(display_a);
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
    use super::{
        ENGAGE_FACING_POST, ENGAGE_WITHHELD_POST, SIGHT_CM, contact_memory_audit, distance,
        frame_windows, host_contact_audit, host_shot_cue_audit, log_time, phase_traffic,
        probe_screen, timed_net_samples,
    };

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
