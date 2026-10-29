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
        for (key, order) in [("f", 7u32), ("s", 3), ("g", 4)] {
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
        for (key, order) in [("s", "Stop"), ("g", "Guard")] {
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
        host.key("a")?;
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
        for (key, order) in [("f", 7u32), ("s", 3), ("g", 4)] {
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
        for (key, order) in [("s", "Stop"), ("g", "Guard")] {
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
        client_a.key("a")?;
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
        thread::sleep(Duration::from_secs(1));
        report.artifact(
            "client_a_menu_png",
            &client_a.screenshot(evidence, "05-menu")?,
        );
        report.artifact(
            "client_b_menu_png",
            &client_b.screenshot(evidence, "05-menu")?,
        );
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

fn parse_layout_options() -> Result<LayoutOptions, String> {
    let usage = "usage: rts_multiplayer --dedicated-layouts SERVER_BINARY CLIENT_BINARY \
                 [--layouts 1v1,2v1,3v1,2v2] [--net-profile none|lag-loss|reorder|duplicate] \
                 [--legacy-client OLD_PROTOCOL_CLIENT] [--evidence DIR] [--port N]";
    let mut args = env::args().skip(2);
    let server = PathBuf::from(args.next().ok_or(usage)?)
        .canonicalize()
        .map_err(|e| format!("server binary: {e}"))?;
    let client = PathBuf::from(args.next().ok_or(usage)?)
        .canonicalize()
        .map_err(|e| format!("client binary: {e}"))?;
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
            "build/diagnostics/mp2-multiclient/packaged-{epoch}"
        ))
    });
    Ok(LayoutOptions {
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

fn shot_cue_consistency(server: &Game, clients: &[Game]) -> (usize, [usize; 3], Vec<String>) {
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

/// F36 and network-condition verdicts for one dedicated layout run: the emulation each
/// process applied, exactly-once ordered command intake, per-connection bandwidth from the
/// engine's own counters, observed impairment, and authoritative step cost.
fn network_checks(
    report: &mut Report,
    id: &dyn Fn(&str) -> String,
    layout: &str,
    profile: &NetProfile,
    server: &Game,
    clients: &[Game],
    generation: &str,
) {
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
        let participant = (index + 1).to_string();
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
    for participant in 1..=clients.len() as u32 {
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
            .get(participant as usize - 1)
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
            && field(l, "seats") == Some(clients.len().to_string().as_str())
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

fn dedicated_layout(
    options: &LayoutOptions,
    layout: &str,
    port: u16,
    report: &mut Report,
) -> Result<(), String> {
    let teams = layout_teams(layout).ok_or("unknown layout")?;
    let seats = teams.len();
    let profile = options.profile;
    let suffix = if profile.name == "none" {
        String::new()
    } else {
        format!("-{}", profile.name.to_uppercase())
    };
    let id = |name: &str| format!("MP2-{}{suffix}-{name}", layout.to_uppercase());
    let evidence = options
        .evidence
        .join(format!("layout-{layout}{}", suffix.to_lowercase()));
    fs::create_dir_all(&evidence).map_err(|e| e.to_string())?;
    let client_options = Options {
        binary: options.client.clone(),
        layout: layout.into(),
        evidence: evidence.clone(),
        port,
    };

    let emulation: Vec<String> = profile.args.iter().map(|a| (*a).to_owned()).collect();
    let mut launch_args = server_args(port, layout);
    launch_args.extend(emulation.iter().cloned());
    let mut server = Game::launch_server(&options.server, "server", &evidence, &launch_args)?;
    report.fact(&format!("{layout}_server_pid"), server.pid().to_string());
    report.fact(
        &format!("{layout}_server_command"),
        server.command.join(" "),
    );
    let server_pid = server.pid();
    let ready = server.wait(&["MP_DEDICATED_SERVER_READY"], Duration::from_secs(120))?;
    let awaiting = server.wait(&["MP_MATCH_AWAITING_PARTICIPANTS"], Duration::from_secs(30))?;
    let generation = field(&awaiting, "generation").unwrap_or("0").to_owned();
    let session = server.wait(&["MP_SESSION_LAYOUT"], Duration::from_secs(5))?;
    report.verdict(
        &id("LAYOUT"),
        field(&session, "layout") == Some(layout)
            && field(&session, "seats") == Some(seats.to_string().as_str())
            && field(&session, "fog") == Some("1")
            && field(&session, "hosting") == Some("dedicated")
            && field(&session, "remote_seats") == Some(seats.to_string().as_str())
            && field(&ready, "local_players") == Some("0")
            && field(&ready, "can_render") == Some("0"),
        format!("{}; {}", ready.trim(), session.trim()),
    );

    // Clients join one at a time so the server binds seats in a known join order.
    let mut displays = Vec::new();
    let mut clients: Vec<Game> = Vec::new();
    let mut bindings = Vec::new();
    for seat in 0..seats {
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
    let mut assignments = Vec::new();
    for (index, client) in clients.iter_mut().enumerate() {
        assignments.push(client.wait(
            &[&format!("MP_CLIENT_ASSIGNMENT participant={} ", index + 1)],
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
    let bind_ok = bindings.iter().zip(teams).all(|(bound, team)| {
        field(bound, "team") == Some(team.to_string().as_str())
            && field(bound, "remote") == Some("1")
    }) && assignments
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
    report.verdict(
        &id("ACTIVE-ALL-REMOTE"),
        field(&active, "hosting") == Some("dedicated")
            && field(&active, "local") == Some("0")
            && field(&active, "remote") == Some(seats.to_string().as_str())
            && pids.len() == seats + 1,
        format!(
            "{} independent processes {pids:?}: {}",
            seats + 1,
            active.trim()
        ),
    );

    for client in &mut clients {
        client.attach_window()?;
    }
    let mut recorders = Vec::new();
    for (display, client) in displays.iter().zip(&clients) {
        recorders.push(Recorder::start(&display.name(), &evidence, &client.label)?);
    }
    let result = (|| -> Result<(), String> {
        thread::sleep(Duration::from_secs(3));
        for client in &clients {
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
        for (index, client) in clients.iter().enumerate() {
            let participant = index as u32 + 1;
            let team = teams[index];
            let expected: Vec<u64> = samples
                .iter()
                .filter(|(_, _, mask)| mask & (1 << team) != 0)
                .map(|(entity, _, _)| *entity)
                .collect();
            let received: Vec<u64> = relevant_entities(&client.text())
                .iter()
                .map(|(entity, _)| *entity)
                .collect();
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
            select_entity(&mut clients[index], "MP_CLIENT_SAMPLE", unit)?;
            for (key, order) in [("s", "Stop"), ("g", "Guard")] {
                let mark = server.offset();
                clients[index].key(key)?;
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
        let server_text = server.text();
        let all_remote = count_lines(&server_text, &["MP_COMMAND_RECEIVED", "remote=0"]) == 0
            && (1..=seats).all(|p| {
                count_lines(
                    &server_text,
                    &[&format!("MP_COMMAND_RECEIVED participant={p} remote=1")],
                ) > 0
            });
        report.verdict(
            &id("ORDERS-EVERY-CLIENT"),
            all_remote && order_detail.len() == seats * 2,
            format!(
                "{} real-input orders, every participant remote: {order_detail:?}",
                order_detail.len()
            ),
        );

        // Contest the objective: team B moves part-way, team A takes it (participant 1
        // moves onto it, other team-A members attack-move to it).
        let since = server.offset();
        for index in 0..seats {
            let participant = index as u32 + 1;
            let unit = first_unit(participant).ok_or("missing unit")?;
            let client = &mut clients[index];
            zoom_out(client, 5)?;
            thread::sleep(Duration::from_millis(1500));
            let objective = wait_screen(
                client,
                |t| objective_screen(t, "MP_CLIENT_OBJECTIVE"),
                "objective",
            )?;
            if teams[index] == 2 {
                let own = wait_screen(
                    client,
                    |t| entity_screen(t, "MP_CLIENT_SAMPLE", unit),
                    "own unit",
                )?;
                select_entity(client, "MP_CLIENT_SAMPLE", unit)?;
                client.click(
                    objective.0 + (own.0 - objective.0) * 0.45,
                    objective.1 + (own.1 - objective.1) * 0.45,
                    "3",
                )?;
            } else if participant == 1 {
                select_entity(client, "MP_CLIENT_SAMPLE", unit)?;
                client.click(objective.0, objective.1, "3")?;
                // 1v1 has no team-A ally to support the capture; a lone unit can lose the
                // duel at the point and leave the match uncapturable. Its second unit
                // attack-moves in support, as the allied seats do in larger layouts.
                if let Some(support) = (seats == 2).then(|| second_unit(1)).flatten() {
                    thread::sleep(Duration::from_millis(400));
                    select_entity(client, "MP_CLIENT_SAMPLE", support)?;
                    client.hover(objective.0 - 18.0, objective.1 + 10.0)?;
                    thread::sleep(Duration::from_millis(150));
                    client.key("a")?;
                }
            } else {
                select_entity(client, "MP_CLIENT_SAMPLE", unit)?;
                client.hover(objective.0 + 18.0, objective.1 + 10.0)?;
                thread::sleep(Duration::from_millis(150));
                client.key("a")?;
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
        for client in &clients {
            report.artifact(
                &format!("{layout}_{}_orders_png", client.label),
                &client.screenshot(&evidence, "02-orders")?,
            );
        }

        // Contact: each team-B client eventually receives a team-A unit it did not see at start.
        let mut reveals = Vec::new();
        let mut reveal_ok = true;
        for index in (0..seats).filter(|i| teams[*i] == 2) {
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
        for client in &clients {
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
                && field(&line, "viewer_team") == Some(teams[index].to_string().as_str());
            observed.push(line.trim().to_owned());
        }
        thread::sleep(Duration::from_millis(1500));
        for client in &clients {
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

        let remote: Vec<u32> = (1..=seats as u32).collect();
        // Probing clients: participant 2 (team B) and, with three or more seats, 3 (team A).
        let probing: Vec<usize> = [1usize, 2].into_iter().filter(|i| *i < seats).collect();
        for &index in &probing {
            let client = &mut clients[index];
            client.wait(
                &["MP_PROBE_SENT name=after_termination"],
                Duration::from_secs(10),
            )?;
        }
        thread::sleep(Duration::from_secs(1));
        for &index in &probing {
            let client = &clients[index];
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

        let (received, counts, problems) = shot_cue_consistency(&server, &clients);
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
        let mut memory_detail = Vec::new();
        let mut memory_ok = true;
        for (client, label) in clients.iter().zip(1..) {
            let (recorded, deaths, problems) = contact_memory_audit(&client.text());
            memory_ok &= problems.is_empty();
            memory_detail.push(format!(
                "client {label}: recorded {recorded}, deaths seen {deaths}, problems={problems:?}"
            ));
        }
        report.verdict(
            &id("FOG-MEMORY-NO-LEAK"),
            memory_ok,
            memory_detail.join("; "),
        );
        network_checks(report, &id, layout, profile, &server, &clients, &generation);

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
    for (recorder, client) in recorders.into_iter().zip(&clients) {
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
    report.artifact(&format!("{layout}_server_log"), &server.log.clone());
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

fn run_dedicated_layouts() -> ! {
    let mut options = match parse_layout_options() {
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
        schema: Some("politika.mp2.multiclient-evidence.v1"),
        ..Report::default()
    };
    report.fact("network_protocol", network_protocol());
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
        if let Err(error) = dedicated_layout(&options, layout, port, &mut report) {
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
    if let Err(error) = legacy_protocol_refusal(&options, legacy_port, &mut report) {
        report.mark("SESSION-legacy-protocol", "FAIL", error);
    }
    let path = options.evidence.join("multiclient-evidence.json");
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

fn run_fog_memory() -> ! {
    let mut options = match parse_dedicated_options() {
        Ok(options) => options,
        Err(error) => {
            eprintln!("{}", error.replace("--dedicated", "--fog-memory"));
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
        schema: Some("politika.mp2.fog-memory-evidence.v1"),
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
    if let Err(error) = fog_memory_session(&options, &mut report) {
        report.mark(
            "SESSION-fog-memory",
            if error.starts_with("BLOCKED") {
                "BLOCKED"
            } else {
                "FAIL"
            },
            error,
        );
    }
    let path = options.client.evidence.join("fog-memory-evidence.json");
    if let Err(error) = report.write(&path) {
        eprintln!("write evidence: {error}");
        std::process::exit(2);
    }
    println!("evidence: {}", path.display());
    std::process::exit(if report.failed() { 1 } else { 0 });
}

fn main() {
    if env::args().nth(1).as_deref() == Some("--fog-memory") {
        run_fog_memory();
    }
    if env::args().nth(1).as_deref() == Some("--dedicated") {
        run_dedicated();
    }
    if env::args().nth(1).as_deref() == Some("--dedicated-layouts") {
        run_dedicated_layouts();
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
    use super::contact_memory_audit;

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
}
