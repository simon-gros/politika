//! Rendered packaged invalid-content recovery and same-session retry.
//! Run under Xvfb with the Linux Development package executable as its argument.
use std::{
    env, fs,
    path::{Path, PathBuf},
    process::{Child, Command, Stdio},
    thread,
    time::{Duration, Instant, SystemTime, UNIX_EPOCH},
};

fn run(command: &mut Command) -> Result<String, String> {
    let result = command.output().map_err(|e| e.to_string())?;
    if !result.status.success() {
        return Err(format!(
            "command exited {}: {}",
            result.status,
            String::from_utf8_lossy(&result.stderr)
        ));
    }
    Ok(String::from_utf8_lossy(&result.stdout).trim().to_owned())
}

fn wait_for(
    child: &mut Child,
    log: &Path,
    marker: &str,
    timeout: Duration,
) -> Result<String, String> {
    let start = Instant::now();
    loop {
        if let Some(status) = child.try_wait().map_err(|e| e.to_string())? {
            return Err(format!(
                "process exited {status} before `{marker}`; log={}",
                log.display()
            ));
        }
        if let Ok(text) = fs::read_to_string(log) {
            if text.contains(marker) {
                return Ok(text);
            }
        }
        if start.elapsed() >= timeout {
            return Err(format!("timeout waiting for `{marker}`; not a pass"));
        }
        thread::sleep(Duration::from_millis(100));
    }
}

fn screenshot(window: &str, evidence: &Path, name: &str) -> Result<(), String> {
    let path = evidence.join(format!("{name}.png"));
    run(Command::new("import").args(["-window", window]).arg(&path))?;
    if fs::metadata(&path).map_err(|e| e.to_string())?.len() < 1024 {
        return Err(format!("screenshot {name} is unexpectedly small"));
    }
    Ok(())
}

fn main() -> Result<(), String> {
    if env::var_os("DISPLAY").is_none() {
        return Err("BLOCKED: run under xvfb-run; DISPLAY is unset".into());
    }
    let binary = PathBuf::from(
        env::args_os()
            .nth(1)
            .ok_or("usage: shell_recovery PACKAGED_BINARY")?,
    )
    .canonicalize()
    .map_err(|e| e.to_string())?;
    let fixture = env::args()
        .nth(2)
        .unwrap_or_else(|| "InvalidMapSchema".into());
    let (fixture_map, diagnostic, property, values, asset_name) = match fixture.as_str() {
        "InvalidMapSchema" => (
            "map.test.invalid_schema",
            "CONTENT.SCHEMA.UNSUPPORTED",
            "SchemaVersion",
            "expected=1|actual=2",
            "DA_InvalidMapSchema",
        ),
        "InvalidParticipantFaction" => (
            "map.test.invalid_participant_faction",
            "CONTENT.FACTION.MISMATCH",
            "UnitPlacements[0].UnitArchetypeId",
            "expected=faction.player|actual=faction.opposition",
            "DA_InvalidParticipantFaction",
        ),
        other => return Err(format!("unknown recovery fixture `{other}`")),
    };
    let package = binary
        .parent()
        .and_then(Path::parent)
        .and_then(Path::parent)
        .and_then(Path::parent)
        .ok_or("expected packaged Binaries/Linux/ executable")?;
    let identity =
        fs::read_to_string(package.join("BuildIdentity.json")).map_err(|e| e.to_string())?;
    let manifest = fs::read_to_string(package.join("Manifest_UFSFiles_Linux.txt"))
        .map_err(|e| e.to_string())?;
    if !manifest.contains(&format!(
        "Politika/Content/Politika/TestFixtures/{asset_name}.uasset"
    )) {
        return Err(format!(
            "BLOCKED: cooked fixture {asset_name} is absent from package manifest"
        ));
    }
    let revision = identity
        .lines()
        .find(|line| line.contains("\"git_revision\""))
        .ok_or("BuildIdentity.json has no git_revision")?
        .trim();
    let epoch = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .map_err(|e| e.to_string())?
        .as_secs();
    let evidence = package.join(format!("content-acceptance-recovery-{epoch}"));
    fs::create_dir_all(&evidence).map_err(|e| e.to_string())?;
    let log = evidence.join("shell-recovery.log");
    let user = evidence.join("user");
    fs::create_dir_all(&user).map_err(|e| e.to_string())?;
    let selector_argument = format!("-PolitikaTestFixture={fixture}");
    let mut child = Command::new(&binary)
        .arg("/Game/Maps/RTSVerticalSlice")
        .arg(selector_argument)
        .args([
            "-windowed",
            "-ResX=1280",
            "-ResY=720",
            "-unattended",
            "-nosplash",
        ])
        .arg(format!("-UserDir={}/", user.display()))
        .arg(format!("-ABSLOG={}", log.display()))
        .stdout(Stdio::null())
        .stderr(Stdio::null())
        .spawn()
        .map_err(|e| e.to_string())?;
    let process_id = child.id();
    let result = (|| -> Result<(), String> {
        let diagnostic_marker = format!("{diagnostic}|error|{fixture_map}|{property}|");
        let rejected_marker = format!(
            "MATCH_ADMISSION_REJECTED fixture={fixture_map} units=0 objective=0 simulation_started=0"
        );
        let state_marker = format!(
            "MATCH_ADMISSION_STATE map={fixture_map} participants=0 local_player=0 unaligned_system=0 units=0 objectives=0 selection=0 pending_commands=0 accepted_sequences=0 simulation_steps=0 content_ready=0 terminal_result=0"
        );
        let recovery_marker = format!("MATCH_ADMISSION_RECOVERY map={fixture_map} route=main_menu");
        let mut rejected = wait_for(
            &mut child,
            &log,
            &diagnostic_marker,
            Duration::from_secs(60),
        )?;
        for marker in [
            values,
            "MATCH_LIFECYCLE phase=8",
            rejected_marker.as_str(),
            state_marker.as_str(),
            recovery_marker.as_str(),
            "LoadMap: /Game/Maps/MainMenu",
        ] {
            rejected = wait_for(&mut child, &log, marker, Duration::from_secs(10))?;
            if !rejected.contains(marker) {
                return Err(format!("missing packaged rejection evidence `{marker}`"));
            }
        }
        wait_for(
            &mut child,
            &log,
            "Load map complete /Game/Maps/MainMenu",
            Duration::from_secs(10),
        )?;
        thread::sleep(Duration::from_millis(500));
        if rejected.contains("\"code\":\"InvalidLaunchOptions\"") {
            return Err("fixture selector was incorrectly rejected as a startup option".into());
        }
        let mut window = None;
        for _ in 0..100 {
            let mut search = Command::new("xdotool");
            search.args(["search", "--onlyvisible", "--name", "Politika"]);
            if let Ok(found) = run(&mut search) {
                if let Some(id) = found.lines().next() {
                    window = Some(id.to_owned());
                    break;
                }
            }
            thread::sleep(Duration::from_millis(100));
        }
        let window =
            window.ok_or("no visible packaged shell window after recoverable rejection")?;
        let mut geo = Command::new("xdotool");
        geo.args(["getwindowgeometry", "--shell", &window]);
        let geometry = run(&mut geo)?;
        if !geometry.contains("WIDTH=1280") || !geometry.contains("HEIGHT=720") {
            return Err(format!("unexpected shell window geometry: {geometry}"));
        }
        fs::write(evidence.join("window-geometry.txt"), geometry).map_err(|e| e.to_string())?;
        // Let the first post-transition slate frame render; LoadMap is logged before its first present.
        thread::sleep(Duration::from_secs(2));
        screenshot(&window, &evidence, "01-rejected-menu")?;
        let mut focus = Command::new("xdotool");
        let _ = run(focus.args(["windowfocus", &window]));
        thread::sleep(Duration::from_millis(300));
        // MainMenu's authored prototype action at the verified 1280x720 layout.
        run(Command::new("xdotool").args([
            "mousemove",
            "--sync",
            "--window",
            &window,
            "640",
            "318",
            "click",
            "1",
        ]))?;
        wait_for(
            &mut child,
            &log,
            "MATCH_CONTENT_READY map=map.prototype.rts_vertical_slice units=3 objective=objective.central_control",
            Duration::from_secs(15),
        )?;
        wait_for(
            &mut child,
            &log,
            "MATCH_LIFECYCLE phase=4",
            Duration::from_secs(10),
        )?;
        wait_for(
            &mut child,
            &log,
            "MATCH_SIMULATION_STEP first=1",
            Duration::from_secs(10),
        )?;
        thread::sleep(Duration::from_millis(500));
        if child.try_wait().map_err(|e| e.to_string())?.is_some() {
            return Err(
                "the packaged process exited between rejection and canonical Active retry".into(),
            );
        }
        let retried = fs::read_to_string(&log).map_err(|e| e.to_string())?;
        let retry_start = retried
            .rfind("MATCH_PARTICIPANTS_ADMITTED map=map.prototype.rts_vertical_slice")
            .ok_or("canonical retry did not admit the production participant roster")?;
        let rejected_attempt = &retried[..retry_start];
        if rejected_attempt.contains("RTS_ORDER_ADMITTED")
            || rejected_attempt.contains("MATCH_SIMULATION_STEP")
        {
            return Err(
                "rejected participant fixture executed gameplay before canonical retry".into(),
            );
        }
        let retry_log = &retried[retry_start..];
        for marker in [
            "MATCH_PARTICIPANTS_ADMITTED map=map.prototype.rts_vertical_slice playable=2 local_player=1 local_faction=faction.player unaligned_system=1",
            "MATCH_AUTHORITATIVE_UNIT index=0 entity=1 archetype=unit.recon_vehicle owner=1 faction=faction.player slot=1",
            "MATCH_AUTHORITATIVE_UNIT index=1 entity=2 archetype=unit.mechanized_fireteam owner=1 faction=faction.player slot=1",
            "MATCH_AUTHORITATIVE_UNIT index=2 entity=3 archetype=unit.opposing_patrol owner=2 faction=faction.opposition slot=2",
            "MATCH_AUTHORITATIVE_OBJECTIVE id=objective.central_control",
            "MATCH_CONTENT_READY map=map.prototype.rts_vertical_slice units=3 objective=objective.central_control",
        ] {
            if !retry_log.contains(marker) {
                return Err(format!(
                    "canonical retry is missing fresh authoritative mapping `{marker}`"
                ));
            }
        }
        let phase_markers = [
            "MATCH_LIFECYCLE phase=1",
            "MATCH_LIFECYCLE phase=2",
            "MATCH_LIFECYCLE phase=3",
            "MATCH_LIFECYCLE phase=4",
        ];
        let mut phase_cursor = retry_start;
        for phase in phase_markers {
            let offset = retried[phase_cursor..].find(phase).ok_or(format!(
                "canonical retry lacks legal lifecycle phase `{phase}`"
            ))?;
            phase_cursor += offset + phase.len();
        }
        if retried.matches("MATCH_ADMISSION_REJECTED").count() != 1 {
            return Err("same-session retry generated another rejection or fixture selection was not one-shot".into());
        }
        screenshot(&window, &evidence, "02-valid-match-after-rejection")?;
        run(Command::new("xdotool").args(["key", "Escape"]))?;
        wait_for(
            &mut child,
            &log,
            "LoadMap: /Game/Maps/MainMenu",
            Duration::from_secs(10),
        )?;
        wait_for(
            &mut child,
            &log,
            "Load map complete /Game/Maps/MainMenu",
            Duration::from_secs(10),
        )?;
        thread::sleep(Duration::from_millis(500));
        screenshot(&window, &evidence, "03-menu-after-valid-retry")?;
        Ok(())
    })();
    let _ = child.kill();
    let _ = child.wait();
    result?;
    let report = format!(
        "requirement\tverdict\tdetail\nBUILD\tRECORDED\t{}\nFIXTURE\tRECORDED\t{} ({})\nPID\tRECORDED\t{}\nREJECTION\tPASS\t{} with atomic zero-state diagnostic and rendered MainMenu recovery\nACTIVE_RETRY\tPASS\tcanonical match reached lifecycle Active and executed its first simulation step in the same process\nPAC-07\tPASS\trendered recoverable rejection stayed in the usable shell\nPAC-09\tPASS\tprototype button started the canonical valid match in the same process after one-shot invalid fixture rejection\n",
        revision, fixture, fixture_map, process_id, diagnostic
    );
    fs::write(evidence.join("results.tsv"), report).map_err(|e| e.to_string())?;
    println!(
        "PASS PAC-07/PAC-09: rendered recovery and same-session valid retry; evidence {}",
        evidence.display()
    );
    Ok(())
}
