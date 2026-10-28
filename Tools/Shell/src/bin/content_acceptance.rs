//! Packaged Linux Development content-admission acceptance checks.
//! Usage: cargo run --locked --manifest-path Tools/Shell/Cargo.toml --bin content_acceptance -- PACKAGE/POLITIKA/Binaries/Linux/Politika
use std::{
    collections::BTreeSet,
    env, fs,
    path::{Path, PathBuf},
    process::{Child, Command, Stdio},
    thread,
    time::{Duration, Instant, SystemTime, UNIX_EPOCH},
};

fn read(path: &Path) -> Result<String, String> {
    fs::read_to_string(path).map_err(|e| format!("{}: {e}", path.display()))
}
fn stop(child: &mut Child) {
    let _ = child.kill(); // Test cleanup after all runtime assertions have been observed.
    let _ = child.wait();
}
fn launch_live(
    binary: &Path,
    dir: &Path,
    name: &str,
    args: &[&str],
    markers: &[&str],
) -> Result<String, String> {
    fs::create_dir_all(dir).map_err(|e| e.to_string())?;
    let log_path = dir.join(format!("{name}.log"));
    let mut child = Command::new(binary)
        .args(args)
        .args(["-nullrhi", "-unattended", "-nosound", "-nosplash"])
        .arg(format!("-UserDir={}/", dir.join(name).display()))
        .arg(format!("-ABSLOG={}", log_path.display()))
        .stdout(Stdio::null())
        .stderr(Stdio::null())
        .spawn()
        .map_err(|e| format!("launch {name}: {e}"))?;
    let start = Instant::now();
    loop {
        if let Some(status) = child.try_wait().map_err(|e| e.to_string())? {
            return Err(format!(
                "{name}: process exited before runtime assertions ({status}); log {}",
                log_path.display()
            ));
        }
        if let Ok(log) = read(&log_path) {
            if markers.iter().all(|marker| log.contains(marker)) {
                stop(&mut child);
                return Ok(log);
            }
        }
        if start.elapsed() >= Duration::from_secs(60) {
            stop(&mut child);
            return Err(format!(
                "{name}: timeout waiting for runtime evidence (not a pass); log {}",
                log_path.display()
            ));
        }
        thread::sleep(Duration::from_millis(100));
    }
}
fn launch_default(binary: &Path, dir: &Path) -> Result<String, String> {
    let log_path = dir.join("default-launch.log");
    let mut child = Command::new(binary)
        .args([
            "-nullrhi",
            "-unattended",
            "-nosound",
            "-nosplash",
            "-ExecCmds=Quit",
        ])
        .arg(format!("-UserDir={}/", dir.join("default").display()))
        .arg(format!("-ABSLOG={}", log_path.display()))
        .stdout(Stdio::null())
        .stderr(Stdio::null())
        .spawn()
        .map_err(|e| e.to_string())?;
    let start = Instant::now();
    loop {
        if let Some(status) = child.try_wait().map_err(|e| e.to_string())? {
            let log = read(&log_path)?;
            if !status.success() || !log.contains("LoadMap: /Game/Maps/MainMenu") {
                return Err(format!(
                    "default launch failed ({status}); see {}",
                    log_path.display()
                ));
            }
            return Ok(log);
        }
        if start.elapsed() >= Duration::from_secs(90) {
            stop(&mut child);
            return Err(format!(
                "default launch timed out (not a pass); see {}",
                log_path.display()
            ));
        }
        thread::sleep(Duration::from_millis(100));
    }
}
fn main() -> Result<(), String> {
    let binary = PathBuf::from(
        env::args_os()
            .nth(1)
            .ok_or("usage: content_acceptance PACKAGED_BINARY")?,
    )
    .canonicalize()
    .map_err(|e| e.to_string())?;
    let fixture = env::args().nth(2).unwrap_or_else(|| "InvalidMapSchema".into());
    let (fixture_map, diagnostic, property, values, asset_name) = match fixture.as_str() {
        "InvalidMapSchema" => ("map.test.invalid_schema", "CONTENT.SCHEMA.UNSUPPORTED", "SchemaVersion", "expected=1|actual=2", "DA_InvalidMapSchema"),
        "InvalidParticipantFaction" => ("map.test.invalid_participant_faction", "CONTENT.FACTION.MISMATCH", "UnitPlacements[0].UnitArchetypeId", "expected=faction.player|actual=faction.opposition", "DA_InvalidParticipantFaction"),
        other => return Err(format!("unknown acceptance fixture `{other}`")),
    };
    let package = binary
        .parent()
        .and_then(Path::parent)
        .and_then(Path::parent)
        .and_then(Path::parent)
        .ok_or("could not determine package root from Binaries/Linux/ executable")?;
    let manifest = read(&package.join("Manifest_UFSFiles_Linux.txt"))?;
    if !manifest.contains(&format!("Politika/Content/Politika/TestFixtures/{asset_name}.uasset")) {
        return Err("BLOCKED: cooked fixture is absent from this package manifest".into());
    }
    let epoch = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .map_err(|e| e.to_string())?
        .as_secs();
    let evidence = package.join(format!("content-acceptance-{epoch}"));
    fs::create_dir_all(&evidence).map_err(|e| e.to_string())?;
    let identity = read(&package.join("BuildIdentity.json"))?;
    let revision = identity
        .lines()
        .find(|line| line.contains("\"git_revision\""))
        .ok_or("BuildIdentity.json has no git_revision")?
        .trim();
    let mut results = Vec::<(&str, &str, String)>::new();
    let selector_argument = format!("-PolitikaTestFixture={fixture}");
    let invalid_args = ["/Game/Maps/RTSVerticalSlice", selector_argument.as_str()];
    let diagnostic_marker = format!("{diagnostic}|error|{fixture_map}|{property}|");
    let rejection_marker = format!("MATCH_ADMISSION_REJECTED fixture={fixture_map} units=0 objective=0 simulation_started=0");
    let state_marker = format!("MATCH_ADMISSION_STATE map={fixture_map} participants=0 local_player=0 unaligned_system=0 units=0 objectives=0 selection=0 pending_commands=0 accepted_sequences=0 simulation_steps=0 content_ready=0 terminal_result=0");
    let recovery_marker = format!("MATCH_ADMISSION_RECOVERY map={fixture_map} route=main_menu");
    let negative_markers = [diagnostic_marker.as_str(), values, rejection_marker.as_str(), state_marker.as_str(), recovery_marker.as_str(), "LoadMap: /Game/Maps/MainMenu"];
    let first = launch_live(
        &binary,
        &evidence,
        "invalid-first",
        &invalid_args,
        &negative_markers,
    )?;
    if first.contains("\"code\":\"InvalidLaunchOptions\"") {
        return Err(
            "Development fixture selector leaked into startup as InvalidLaunchOptions".into(),
        );
    }
    results.push(("PAC-02/03/04/05/06", "PASS", "cooked fixture rejected; zero units/objectives/accepted commands/simulation steps explicitly observed; same-process MainMenu transition observed".into()));
    results.push(("PAC-07", "INCONCLUSIVE", "MainMenu loaded and process remained alive, but -nullrhi cannot establish rendered shell usability".into()));
    let stable_signature = first
        .lines()
        .find(|line| line.contains(&diagnostic_marker))
        .ok_or("missing structured rejection diagnostic")?
        .split("LogPolitikaStartup:")
        .last()
        .unwrap_or("")
        .trim()
        .to_string();
    let second = launch_live(
        &binary,
        &evidence,
        "invalid-repeat",
        &invalid_args,
        &negative_markers,
    )?;
    let repeated_signature = second
        .lines()
        .find(|line| line.contains(&diagnostic_marker))
        .ok_or("missing repeated structured rejection diagnostic")?
        .split("LogPolitikaStartup:")
        .last()
        .unwrap_or("")
        .trim()
        .to_string();
    if stable_signature != repeated_signature {
        return Err("repeated validation diagnostic changed".into());
    }
    results.push((
        "PAC-08",
        "PASS",
        "two packaged rejection runs emitted equivalent validator diagnostics".into(),
    ));
    let unknown = launch_live(
        &binary,
        &evidence,
        "unknown-fixture",
        &[
            "/Game/Maps/RTSVerticalSlice",
            "-PolitikaTestFixture=NotARealFixture",
        ],
        &[
            "MATCH_ADMISSION_SELECTION_FAILED fixture=NotARealFixture reason=unknown_fixture",
            "MATCH_ADMISSION_RECOVERY map=NotARealFixture route=main_menu",
            "LoadMap: /Game/Maps/MainMenu",
        ],
    )?;
    if unknown.contains("MATCH_CONTENT_READY") {
        return Err("unknown fixture silently admitted valid content".into());
    }
    results.push((
        "unknown-fixture",
        "PASS",
        "unknown fixture rejected without fallback and recovered to menu".into(),
    ));
    let valid = launch_live(
        &binary,
        &evidence,
        "valid-after-rejection",
        &["/Game/Maps/RTSVerticalSlice"],
        &[
            "CONTENT_VALIDATION_PASSED map=map.prototype.rts_vertical_slice",
            "MATCH_CONTENT_READY map=map.prototype.rts_vertical_slice units=3 objective=objective.central_control",
        ],
    )?;
    if valid.contains("MATCH_ADMISSION_REJECTED") {
        return Err("valid fixture was rejected after negative run".into());
    }
    results.push((
        "PAC-09",
        "PASS",
        "fresh valid match initialized three units and objective after rejection run".into(),
    ));

    let snapshot_markers = ["MATCH_AUTHORITATIVE_SNAPSHOT_COMPLETE units=3 objectives=1"];
    let first_snapshot = launch_live(
        &binary,
        &evidence,
        "canonical-initialization-first",
        &["/Game/Maps/RTSVerticalSlice"],
        &snapshot_markers,
    )?;
    let second_snapshot = launch_live(
        &binary,
        &evidence,
        "canonical-initialization-repeat",
        &["/Game/Maps/RTSVerticalSlice"],
        &snapshot_markers,
    )?;
    let authoritative_lines = |log: &str| -> Vec<String> {
        log.lines()
            .filter(|line| {
                line.contains("MATCH_AUTHORITATIVE_UNIT ")
                    || line.contains("MATCH_AUTHORITATIVE_OBJECTIVE ")
                    || line.contains("MATCH_AUTHORITATIVE_SNAPSHOT_COMPLETE ")
            })
            .filter_map(|line| {
                line.split_once("MATCH_AUTHORITATIVE_")
                    .map(|(_, snapshot)| format!("MATCH_AUTHORITATIVE_{snapshot}"))
            })
            .collect()
    };
    let first_authoritative = authoritative_lines(&first_snapshot);
    let repeated_authoritative = authoritative_lines(&second_snapshot);
    if first_authoritative != repeated_authoritative {
        return Err("repeated canonical map initialization changed its authoritative snapshot".into());
    }
    let unit_lines: Vec<&String> = first_authoritative
        .iter()
        .filter(|line| line.contains("MATCH_AUTHORITATIVE_UNIT "))
        .collect();
    let objective_lines = first_authoritative
        .iter()
        .filter(|line| line.contains("MATCH_AUTHORITATIVE_OBJECTIVE "))
        .count();
    let entity_ids: BTreeSet<&str> = unit_lines
        .iter()
        .filter_map(|line| line.split_whitespace().find_map(|field| field.strip_prefix("entity=")))
        .collect();
    if unit_lines.len() != 3 || objective_lines != 1 || entity_ids.len() != 3 {
        return Err(format!(
            "authoritative snapshot has unexpected counts/duplicates: units={}, objectives={}, unique_entities={}",
            unit_lines.len(), objective_lines, entity_ids.len()
        ));
    }
    results.push((
        "ST3-07",
        "PASS",
        "two fresh packaged canonical initializations had identical authoritative archetype/owner/slot/position/health/movement/weapon snapshots and objective rules; 3 unique entities and 1 objective, no duplicates".into(),
    ));
    let regression = launch_live(
        &binary,
        &evidence,
        "rts-combined",
        &["/Game/Maps/RTSVerticalSlice", "-RTSAcceptance=combined"],
        &[
            "RTS_ACCEPTANCE_CAPTURE_PAUSE result=PASS",
            "RTS_ACCEPTANCE_ATTACKER_DEATH result=PASS",
        ],
    )?;
    if regression.contains("RTS_ACCEPTANCE_CAPTURE_PAUSE result=FAIL")
        || regression.contains("RTS_ACCEPTANCE_ATTACKER_DEATH result=FAIL")
    {
        return Err("RTS acceptance fixture reported failure".into());
    }
    results.push((
        "RTS-DEV-FIXTURES",
        "PASS",
        "capture-pause and attacker-death lifecycle fixtures passed in the packaged runtime".into(),
    ));
    results.push(("PAC-10", "NOT_RUN", "the full interactive movement/combat/objective regression is not replaced by two Development lifecycle fixtures".into()));
    results.push(("PAC-11", "INCONCLUSIVE", "selector and asset loader are compile-time excluded from Shipping source; no Shipping package was built or inspected".into()));
    let _ = valid;
    let _ = regression;
    launch_default(&binary, &evidence)?;
    results.push((
        "PAC-01",
        "PASS",
        "default packaged launch loaded MainMenu".into(),
    ));
    let mut report = String::from("requirement\tverdict\tdetail\n");
    results.insert(0, ("BUILD", "RECORDED", revision.to_string()));
    for (id, verdict, detail) in &results {
        println!("{verdict} {id}: {detail}");
        report.push_str(&format!("{id}\t{verdict}\t{detail}\n"));
    }
    fs::write(evidence.join("results.tsv"), report).map_err(|e| e.to_string())?;
    println!("Evidence: {}", evidence.display());
    Ok(())
}
