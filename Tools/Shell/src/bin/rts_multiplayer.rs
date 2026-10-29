//! Packaged two-process multiplayer acceptance (MP-1).
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
        let _ = self.child.kill();
        let _ = self.child.wait();
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
            if let Some(tail) = text.get(offset..) {
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
        let mut json = String::from(
            "{\n  \"schema\": \"politika.mp1.packaged-evidence.v1\",\n  \"facts\": {\n",
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
        "friendly_target" => &[8],
        "duplicate_replay" | "reordered_older" => &[16],
        "after_termination" => &[12],
        name if name.starts_with("hidden_issuer_") => &[1, 9],
        _ => return None,
    })
}

fn check_probes(report: &mut Report, client: &Game, host: &Game) {
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
    // The server attributes every probe to the connection-bound participant, never another.
    let server_attributed = host_text
        .lines()
        .filter(|l| {
            l.contains("MP_COMMAND_RECEIVED") || l.contains("MP_COMMAND_REJECTED_AT_INTAKE")
        })
        .filter(|l| field(l, "participant") != Some("1"))
        .all(|l| field(l, "participant") == Some("2"));
    if !server_attributed {
        problems.push("server attributed a remote command to a participant other than 2".into());
    }
    report.verdict("MP1-NEG-PROBES", checked >= 14 && problems.is_empty(),
        format!("{checked} correlated hostile/edge probes over the real ServerSubmitCommandIntent RPC; problems={problems:?}"));
}

fn health_consistency(report: &mut Report, host: &Game, client: &Game) {
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
    report.verdict("MP1-POS-DAMAGE-CONSISTENCY", observed > 0 && problems.is_empty(),
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
        prelogin.contains("protocol=1") && client.pid() != host.pid(),
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
        health_consistency(report, &host, &client);
        client.wait(
            &["MP_PROBE_SENT name=after_termination"],
            Duration::from_secs(10),
        )?;
        thread::sleep(Duration::from_secs(1));
        check_probes(report, &client, &host);

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

fn main() {
    let options = match parse_options() {
        Ok(options) => options,
        Err(error) => {
            eprintln!("{error}");
            std::process::exit(2);
        }
    };
    if let Err(error) = fs::create_dir_all(&options.evidence) {
        eprintln!("evidence directory: {error}");
        std::process::exit(2);
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
