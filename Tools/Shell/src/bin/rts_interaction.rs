//! Rendered Linux Development package interaction replay.
//! Run under Xvfb: cargo run --locked --manifest-path Tools/Shell/Cargo.toml --bin rts_interaction -- PACKAGE/POLITIKA/Binaries/Linux/Politika
//! Unreal remains authoritative; this harness only drives input and asserts its structured events.
use std::{
    env, fs,
    fs::OpenOptions,
    io::copy,
    path::{Path, PathBuf},
    process::{Child, Command, Stdio},
    thread,
    time::{Duration, Instant, SystemTime, UNIX_EPOCH},
};

#[path = "../navigation_obstacle.rs"]
mod navigation_obstacle;

fn output(command: &mut Command) -> Result<String, String> {
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

fn input(window: &str, args: &[&str]) -> Result<(), String> {
    let mut command = Command::new("xdotool");
    if args.first() == Some(&"mousemove") {
        command.args(["mousemove", "--sync", "--window", window]);
        command.args(&args[2..]);
    } else {
        command.args(args);
    }
    output(&mut command).map(|_| ())
}

fn capture(window: &str, evidence: &Path, name: &str) -> Result<(), String> {
    let path = evidence.join(format!("{name}.png"));
    let mut command = Command::new("import");
    command.args(["-window", window]).arg(&path);
    output(&mut command).map_err(|e| format!("screenshot {name}: {e}"))?;
    let size = fs::metadata(&path).map_err(|e| e.to_string())?.len();
    if size < 1024 {
        return Err(format!(
            "screenshot {name} is unexpectedly small ({size} bytes)"
        ));
    }
    Ok(())
}

fn start_video_recording(display: &str, geometry: &str, evidence: &Path) -> Result<Child, String> {
    let field = |name: &str| -> Result<i32, String> {
        geometry
            .lines()
            .find_map(|line| line.strip_prefix(&format!("{name}=")))
            .ok_or_else(|| format!("window geometry is missing {name}"))?
            .parse::<i32>()
            .map_err(|e| format!("invalid {name} in window geometry: {e}"))
    };
    let x = field("X")?;
    let y = field("Y")?;
    let video = evidence.join("take.mp4");
    let log = fs::File::create(evidence.join("video-recorder.log")).map_err(|e| e.to_string())?;
    let mut command = Command::new("ffmpeg");
    command.args([
        "-hide_banner",
        "-loglevel",
        "warning",
        "-y",
        "-f",
        "x11grab",
        "-framerate",
        "30",
        "-video_size",
        "1280x720",
        "-i",
    ]);
    command
        .arg(format!("{display}+{x},{y}"))
        .args([
            "-an",
            "-c:v",
            "libx264",
            "-preset",
            "veryfast",
            "-crf",
            "22",
            "-pix_fmt",
            "yuv420p",
            "-movflags",
            "+faststart",
        ])
        .arg(video)
        .stdin(Stdio::null())
        .stdout(Stdio::null())
        .stderr(Stdio::from(log));
    command.spawn().map_err(|e| {
        format!("start ffmpeg X11 video recording (install ffmpeg with x11grab/libx264): {e}")
    })
}

fn stop_video_recording(mut recorder: Child, evidence: &Path) -> Result<(), String> {
    // SIGINT lets FFmpeg write the MP4 index and close the file cleanly.
    let signal = Command::new("kill")
        .args(["-INT", &recorder.id().to_string()])
        .status()
        .map_err(|e| format!("signal ffmpeg recorder: {e}"))?;
    if !signal.success() {
        let _ = recorder.kill();
        let _ = recorder.wait();
        return Err("could not stop ffmpeg recorder cleanly".into());
    }
    let status = recorder
        .wait()
        .map_err(|e| format!("wait for ffmpeg recorder: {e}"))?;
    // FFmpeg uses exit code 255 for a deliberate SIGINT after it has flushed the MP4 trailer.
    if !status.success() && status.code() != Some(255) {
        return Err(format!(
            "ffmpeg recorder exited {status}; see video-recorder.log"
        ));
    }
    let path = evidence.join("take.mp4");
    let size = fs::metadata(&path)
        .map_err(|e| format!("video was not finalized: {e}"))?
        .len();
    if size < 1024 {
        return Err(format!(
            "recorded video is unexpectedly small ({size} bytes)"
        ));
    }
    let probe = Command::new("ffprobe")
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
        .arg(&path)
        .output()
        .map_err(|e| format!("inspect finalized MP4 with ffprobe: {e}"))?;
    let stream = String::from_utf8_lossy(&probe.stdout).trim().to_owned();
    if !probe.status.success() || stream != "h264,1280,720" {
        return Err(format!(
            "recorded MP4 failed stream validation ({stream}); see video-recorder.log"
        ));
    }
    Ok(())
}

fn copy_video_to_desktop(source: &Path, epoch: u64) -> Result<PathBuf, String> {
    let desktop = Command::new("xdg-user-dir")
        .arg("DESKTOP")
        .output()
        .ok()
        .filter(|result| result.status.success())
        .map(|result| String::from_utf8_lossy(&result.stdout).trim().to_owned())
        .filter(|path| !path.is_empty())
        .map(PathBuf::from)
        .or_else(|| env::var_os("HOME").map(|home| PathBuf::from(home).join("Desktop")))
        .ok_or("could not determine the user's Desktop directory")?;
    if !desktop.is_dir() {
        return Err(format!(
            "Desktop directory does not exist: {}",
            desktop.display()
        ));
    }

    let base = format!("Politika-RTS-interaction-{epoch}");
    for suffix in 1..=100 {
        let name = if suffix == 1 {
            format!("{base}.mp4")
        } else {
            format!("{base}-{suffix}.mp4")
        };
        let destination = desktop.join(name);
        let mut output = match OpenOptions::new()
            .write(true)
            .create_new(true)
            .open(&destination)
        {
            Ok(file) => file,
            Err(error) if error.kind() == std::io::ErrorKind::AlreadyExists => continue,
            Err(error) => {
                return Err(format!(
                    "create Desktop video copy {}: {error}",
                    destination.display()
                ));
            }
        };
        let copied = copy(
            &mut fs::File::open(source).map_err(|e| e.to_string())?,
            &mut output,
        )
        .map_err(|e| format!("copy video to Desktop {}: {e}", destination.display()))?;
        let source_size = fs::metadata(source).map_err(|e| e.to_string())?.len();
        if copied != source_size {
            return Err(format!(
                "Desktop video copy size mismatch: source={source_size}, copy={copied}"
            ));
        }
        return Ok(destination);
    }
    Err(format!(
        "could not allocate a non-overwriting Desktop filename for {base}"
    ))
}

fn require_render_difference(before: &Path, after: &Path) -> Result<(), String> {
    let mut compare = Command::new("compare");
    compare
        .args(["-metric", "AE"])
        .arg(before)
        .arg(after)
        .arg("null:");
    let result = compare
        .output()
        .map_err(|e| format!("ImageMagick compare: {e}"))?;
    // ImageMagick exits 1 for a successfully computed non-identical image.
    if !result.status.success() && result.status.code() != Some(1) {
        return Err(format!(
            "image comparison failed: {}",
            String::from_utf8_lossy(&result.stderr)
        ));
    }
    let pixels = String::from_utf8_lossy(&result.stderr)
        .split_whitespace()
        .next()
        .ok_or("image comparison returned no pixel difference")?
        .parse::<f64>()
        .map_err(|e| format!("invalid ImageMagick difference: {e}"))?;
    if !pixels.is_finite() || pixels < 100.0 {
        return Err(format!(
            "camera controls did not produce a meaningful rendered change ({pixels} pixels)"
        ));
    }
    Ok(())
}

fn wait_for(
    child: &mut Child,
    log: &Path,
    marker: &str,
    timeout: Duration,
) -> Result<String, String> {
    wait_for_since(child, log, marker, timeout, 0)
}

fn wait_for_since(
    child: &mut Child,
    log: &Path,
    marker: &str,
    timeout: Duration,
    offset: usize,
) -> Result<String, String> {
    let start = Instant::now();
    loop {
        if let Some(status) = child.try_wait().map_err(|e| e.to_string())? {
            return Err(format!(
                "packaged process exited {status} before `{marker}`; log={}",
                log.display()
            ));
        }
        if let Ok(text) = fs::read_to_string(log) {
            if text.get(offset..).is_some_and(|tail| tail.contains(marker)) {
                return Ok(text);
            }
        }
        if start.elapsed() >= timeout {
            return Err(format!(
                "timeout waiting for `{marker}`; this is not a pass; log={}",
                log.display()
            ));
        }
        thread::sleep(Duration::from_millis(100));
    }
}

fn main() -> Result<(), String> {
    let participant_authority = env::args().nth(2).as_deref() == Some("--participant-authority");
    if env::args().nth(2).as_deref() == Some("--navigation-interruption") {
        return navigation_obstacle::run_interruption();
    }
    if env::args().nth(2).as_deref() == Some("--navigation-repeatability") {
        return navigation_obstacle::run_repeatability();
    }
    if env::args().nth(2).as_deref() == Some("--navigation-unreachable") {
        return navigation_obstacle::run_unreachable();
    }
    if env::args().nth(2).as_deref() == Some("--navigation-destination-failure") {
        return navigation_obstacle::run_destination_failure();
    }
    if env::args().nth(2).as_deref() == Some("--navigation-projection-displacement") {
        return navigation_obstacle::run_projection_displacement();
    }
    if env::args().nth(2).as_deref() == Some("--static-obstacle") {
        return navigation_obstacle::run();
    }
    if env::var_os("DISPLAY").is_none() {
        return Err(
            "BLOCKED: run this rendered acceptance tool under xvfb-run (DISPLAY is unset)".into(),
        );
    }
    let binary = PathBuf::from(
        env::args_os()
            .nth(1)
            .ok_or("usage: rts_interaction PACKAGED_BINARY")?,
    )
    .canonicalize()
    .map_err(|e| e.to_string())?;
    let package = binary
        .parent()
        .and_then(Path::parent)
        .and_then(Path::parent)
        .and_then(Path::parent)
        .ok_or("expected packaged Binaries/Linux/ executable")?;
    let identity =
        fs::read_to_string(package.join("BuildIdentity.json")).map_err(|e| e.to_string())?;
    let revision = identity
        .lines()
        .find(|l| l.contains("\"git_revision\""))
        .ok_or("BuildIdentity.json has no git_revision")?
        .trim();
    let epoch = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .map_err(|e| e.to_string())?
        .as_secs();
    let evidence = package.join(format!("content-acceptance-interaction-{epoch}"));
    fs::create_dir_all(&evidence).map_err(|e| e.to_string())?;
    let log = evidence.join("rts-interaction.log");
    let user_dir = evidence.join("user");
    fs::create_dir_all(&user_dir).map_err(|e| e.to_string())?;

    let mut launch = Command::new(&binary);
    launch.args([
        "/Game/Maps/RTSVerticalSlice",
        "-windowed",
        "-ResX=1280",
        "-ResY=720",
        "-unattended",
        "-nosplash",
    ]);
    if participant_authority {
        launch.arg("-RTSAcceptance=participantauthority");
    }
    let mut child = launch
        .arg(format!("-UserDir={}/", user_dir.display()))
        .arg(format!("-ABSLOG={}", log.display()))
        .stdout(Stdio::null())
        .stderr(Stdio::null())
        .spawn()
        .map_err(|e| format!("launch package: {e}"))?;
    let mut video_recorder: Option<Child> = None;
    let result = (|| -> Result<(), String> {
        wait_for(
            &mut child,
            &log,
            "MATCH_CONTENT_READY map=map.prototype.rts_vertical_slice units=3 objective=objective.central_control",
            Duration::from_secs(60),
        )?;
        if participant_authority {
            wait_for(
                &mut child,
                &log,
                "RTS_ACCEPTANCE_PARTICIPANT_AUTHORITY result=PASS",
                Duration::from_secs(15),
            )?;
            let authority_log = fs::read_to_string(&log).map_err(|e| e.to_string())?;
            if !authority_log.contains("foreign_selection_rejected=1")
                || !authority_log.contains("dead_selection_retired=1")
                || !authority_log.contains("destroyed_queued=1")
                || !authority_log.contains("unaligned_record=1")
                || !authority_log.contains("system_owner=0 system_faction=faction.unaligned")
            {
                return Err(
                    "participant authority package marker lacks required setup assertions".into(),
                );
            }
        }
        let mut window = None;
        for _ in 0..100 {
            let mut search = Command::new("xdotool");
            search.args(["search", "--onlyvisible", "--name", "Politika"]);
            if let Ok(found) = output(&mut search) {
                if let Some(id) = found.lines().next() {
                    window = Some(id.to_owned());
                    break;
                }
            }
            thread::sleep(Duration::from_millis(100));
        }
        let window = window.ok_or("packaged game produced no visible window")?;
        let mut geometry = Command::new("xdotool");
        geometry.args(["getwindowgeometry", "--shell", &window]);
        let geometry = output(&mut geometry)?;
        if !geometry.contains("WIDTH=1280") || !geometry.contains("HEIGHT=720") {
            return Err(format!("unexpected rendered window geometry: {geometry}"));
        }
        fs::write(evidence.join("window-geometry.txt"), &geometry).map_err(|e| e.to_string())?;
        let display =
            env::var("DISPLAY").map_err(|_| "DISPLAY disappeared during recording setup")?;
        video_recorder = Some(start_video_recording(&display, &geometry, &evidence)?);
        let mut focus = Command::new("xdotool");
        focus.args(["windowfocus", &window]);
        let _ = output(&mut focus); // Some headless X servers have no WM focus protocol.
        thread::sleep(Duration::from_secs(2));

        // The fixture map is fixed and its window geometry is checked above; all game-state
        // assertions below come from Development-only events emitted at authoritative C++ edges.
        input(
            &window,
            &["mousemove", "--sync", "735", "552", "click", "1"],
        )?;
        wait_for(
            &mut child,
            &log,
            "RTS_SELECTION player=1 unit=2",
            Duration::from_secs(5),
        )?;
        capture(&window, &evidence, "01-selected")?;

        // The legitimate local selection must not be replaceable by a foreign unit.
        let foreign_log_start = fs::read_to_string(&log).map_err(|e| e.to_string())?.len();
        input(
            &window,
            &["mousemove", "--sync", "577", "323", "click", "1"],
        )?;
        wait_for_since(
            &mut child,
            &log,
            "RTS_SELECTION_REJECTED player=1 unit=3 reason=NotOwned",
            Duration::from_secs(5),
            foreign_log_start,
        )?;
        let selection_log = fs::read_to_string(&log).map_err(|e| e.to_string())?;
        if selection_log.contains("RTS_SELECTION player=1 unit=3") {
            return Err("opposing unit was logged as selected by the local player".into());
        }
        let local_log_start = selection_log.len();
        input(
            &window,
            &["mousemove", "--sync", "735", "552", "click", "1"],
        )?;
        wait_for_since(
            &mut child,
            &log,
            "RTS_SELECTION player=1 unit=2",
            Duration::from_secs(5),
            local_log_start,
        )?;

        if participant_authority {
            // The authority fixture seeds Guard on the opponent to prove order preservation.
            // Its combat setup differs from canonical PAC-10; qualify that replay separately.
            capture(&window, &evidence, "02-authority-local-reselected")?;
            input(&window, &["key", "Escape"])?;
            wait_for(
                &mut child,
                &log,
                "LoadMap: /Game/Maps/MainMenu",
                Duration::from_secs(10),
            )?;
            capture(&window, &evidence, "09-main-menu")?;
            return Ok(());
        }

        // Move to the occupied control point first, proving contested state is reached.
        input(
            &window,
            &["mousemove", "--sync", "640", "360", "click", "3"],
        )?;
        wait_for(
            &mut child,
            &log,
            "RTS_OBJECTIVE_STATE contested=1",
            Duration::from_secs(15),
        )?;
        capture(&window, &evidence, "03-objective-contested")?;

        // Attempt to attack the other friendly unit. It must be rejected and have no attack effect.
        // Authority setup deliberately damages its dead-issuer fixture. Only events after this
        // input belong to the friendly-target check; earlier setup is not a side effect.
        let friendly_log_start = fs::read_to_string(&log).map_err(|e| e.to_string())?.len();
        input(
            &window,
            &["mousemove", "--sync", "799", "500", "click", "3"],
        )?;
        wait_for_since(
            &mut child,
            &log,
            "reason=Orders_cannot_target_friendly_units.",
            Duration::from_secs(5),
            friendly_log_start,
        )?;
        wait_for_since(
            &mut child,
            &log,
            "entity=2 order=1 accepted=0 rejection=8",
            Duration::from_secs(5),
            friendly_log_start,
        )?;
        let current = fs::read_to_string(&log).map_err(|e| e.to_string())?;
        let current = &current[friendly_log_start..];
        if !current.contains("reason=Orders_cannot_target_friendly_units.") {
            return Err("rejected order was not the expected friendly-target rule".into());
        }
        if current.contains("RTS_ORDER_ADMITTED order=Attack")
            || current.contains("RTS_HEALTH_CHANGED")
        {
            return Err("friendly-target rejection caused an attack or health change".into());
        }
        if !current.contains("kind=CommandOutcome") || !current.contains("accepted=0 rejection=8") {
            return Err(
                "friendly-target rejection did not publish its typed command result".into(),
            );
        }

        input(
            &window,
            &["mousemove", "--sync", "650", "450", "click", "3"],
        )?;
        wait_for(
            &mut child,
            &log,
            "RTS_ORDER_ADMITTED order=Move",
            Duration::from_secs(5),
        )?;
        wait_for(
            &mut child,
            &log,
            "RTS_GAMEPLAY_EVENT kind=CommandOutcome",
            Duration::from_secs(5),
        )?;
        wait_for(
            &mut child,
            &log,
            "RTS_POSITION_UPDATED order=Move",
            Duration::from_secs(5),
        )?;
        input(&window, &["key", "s"])?;
        wait_for(
            &mut child,
            &log,
            "RTS_ORDER_ADMITTED order=Stop",
            Duration::from_secs(5),
        )?;
        input(&window, &["key", "g"])?;
        wait_for(
            &mut child,
            &log,
            "RTS_ORDER_ADMITTED order=Guard unit=2",
            Duration::from_secs(5),
        )?;
        capture(&window, &evidence, "04-guard")?;
        // The rifleman may already be within weapon range after its Guard order. In that
        // legitimate case AttackMove acquires and fires rather than producing a movement
        // position event, so prove the authoritative order then accept either runtime path.
        let attack_move_log_start = fs::read_to_string(&log).map_err(|e| e.to_string())?.len();
        input(&window, &["mousemove", "--sync", "650", "430", "key", "a"])?;
        wait_for(
            &mut child,
            &log,
            "RTS_ORDER_ADMITTED order=AttackMove",
            Duration::from_secs(5),
        )?;
        let attack_move_log = fs::read_to_string(&log).map_err(|e| e.to_string())?;
        if !attack_move_log[attack_move_log_start..]
            .contains("RTS_POSITION_UPDATED order=AttackMove")
        {
            wait_for_since(
                &mut child,
                &log,
                "INFANTRY_SHOT actor=",
                Duration::from_secs(5),
                attack_move_log_start,
            )?;
        }
        input(
            &window,
            &["mousemove", "--sync", "577", "323", "click", "3"],
        )?;
        wait_for(
            &mut child,
            &log,
            "RTS_ORDER_ADMITTED order=Attack",
            Duration::from_secs(5),
        )?;
        wait_for(
            &mut child,
            &log,
            "RTS_HEALTH_CHANGED unit=3",
            Duration::from_secs(10),
        )?;
        capture(&window, &evidence, "05-health-damage")?;
        wait_for(
            &mut child,
            &log,
            "RTS_UNIT_DESTROYED unit=3",
            Duration::from_secs(20),
        )?;
        let lifecycle_log = fs::read_to_string(&log).map_err(|e| e.to_string())?;
        for kind in [
            "UnitDamaged",
            "UnitDestroyed",
            "TargetLost",
            "OrderChanged",
            "ObjectiveChanged",
        ] {
            if !lifecycle_log.contains(&format!("RTS_GAMEPLAY_EVENT kind={kind}")) {
                return Err(format!("packaged run did not publish typed {kind} event"));
            }
        }
        capture(&window, &evidence, "06-enemy-destroyed")?;

        // With the enemy destroyed, return to the objective and observe uncontested progress.
        input(
            &window,
            &["mousemove", "--sync", "640", "360", "click", "3"],
        )?;
        wait_for(
            &mut child,
            &log,
            "RTS_OBJECTIVE_STATE contested=0",
            Duration::from_secs(15),
        )?;
        let tail = fs::read_to_string(&log).map_err(|e| e.to_string())?;
        let objective = tail
            .lines()
            .rev()
            .find(|l| l.contains("RTS_OBJECTIVE_STATE contested=0"))
            .ok_or("missing uncontested objective event")?;
        if !objective.contains("progress_milli=") {
            return Err("objective progress field missing".into());
        }
        capture(&window, &evidence, "07-objective-uncontested")?;

        // Pan last so fixed world-coordinate clicks above remain stable throughout the replay.
        input(&window, &["keydown", "w", "keydown", "d"])?;
        thread::sleep(Duration::from_millis(800));
        input(&window, &["keyup", "w", "keyup", "d"])?;
        wait_for(
            &mut child,
            &log,
            "RTS_CAMERA_INPUT axis=forward value=1.0",
            Duration::from_secs(5),
        )?;
        wait_for(
            &mut child,
            &log,
            "RTS_CAMERA_INPUT axis=right value=1.0",
            Duration::from_secs(5),
        )?;
        let camera_log = fs::read_to_string(&log).map_err(|e| e.to_string())?;
        let camera_state = camera_log
            .lines()
            .rev()
            .find(|line| line.contains("RTS_CAMERA_INPUT axis=right value=1.0"))
            .ok_or("missing positive right-pan state")?;
        let moved = camera_state.split_whitespace().any(|field| {
            field
                .strip_prefix("x=")
                .or_else(|| field.strip_prefix("y="))
                .and_then(|value| value.parse::<f32>().ok())
                .is_some_and(|value| value.abs() > 1.0)
        });
        if !moved {
            return Err(format!(
                "camera input reached the controller but its clamped position did not change: {camera_state}"
            ));
        }
        capture(&window, &evidence, "08-camera-pan")?;
        require_render_difference(
            &evidence.join("07-objective-uncontested.png"),
            &evidence.join("08-camera-pan.png"),
        )?;

        input(&window, &["key", "Escape"])?;
        wait_for(
            &mut child,
            &log,
            "LoadMap: /Game/Maps/MainMenu",
            Duration::from_secs(10),
        )?;
        capture(&window, &evidence, "09-main-menu")?;
        Ok(())
    })();

    let video_result = video_recorder
        .map(|recorder| stop_video_recording(recorder, &evidence))
        .unwrap_or_else(|| Err("video recording did not start".into()));
    let _ = child.kill();
    let _ = child.wait();
    let desktop_copy_result = if video_result.is_ok() {
        Some(copy_video_to_desktop(&evidence.join("take.mp4"), epoch))
    } else {
        None
    };
    result?;
    video_result?;
    let desktop_copy = desktop_copy_result
        .ok_or("video was not available for Desktop copy")?
        .map_err(|e| e)?;
    fs::write(
        evidence.join("video-manifest.tsv"),
        format!(
            "file\twidth\theight\tframes_per_second\taudio\tdesktop_copy\twindow_geometry\ntake.mp4\t1280\t720\t30\tno\t{}\t{}",
            desktop_copy.display(),
            fs::read_to_string(evidence.join("window-geometry.txt"))
                .unwrap_or_default()
                .replace('\n', " ")
                .trim()
        ),
    )
    .map_err(|e| e.to_string())?;
    let report = if participant_authority {
        format!(
            "requirement\tverdict\tdetail\nBUILD\tRECORDED\t{}\nPA-AUTHORITY\tPASS\tDevelopment-only participantauthority selector reported exact C++ boundary assertions; rendered bound controller foreign selection was rejected, local selection remained available, and Escape returned to MainMenu at verified 1280x720; see take.mp4; desktop_copy={}\n",
            revision,
            desktop_copy.display()
        )
    } else {
        format!(
            "requirement\tverdict\tdetail\nBUILD\tRECORDED\t{}\nPAC-10\tPASS\tpackaged rendered input exercised local selection, move/position update, typed friendly-target rejection without attack side effects, stop, guard, attack-move, typed damage/death/target-loss/order/objective events, and return to MainMenu at verified 1280x720; see take.mp4; desktop_copy={}\n",
            revision,
            desktop_copy.display()
        )
    };
    fs::write(evidence.join("results.tsv"), report).map_err(|e| e.to_string())?;
    println!(
        "PASS {}: packaged RTS interaction replay; evidence {}; Desktop video {}",
        if participant_authority {
            "PA-AUTHORITY"
        } else {
            "PAC-10"
        },
        evidence.display(),
        desktop_copy.display()
    );
    Ok(())
}
