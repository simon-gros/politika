//! Offline POLITIKA repository quality and environment checks.
//! Uses Rust, the canonical version manifest, and design-stage content metadata.
//! Never interprets missing gameplay/Unreal builds as successful engine tests.
#[path = "quality/production.rs"]
mod production;

use politika_version::{build_identity, load_manifest, repo_root};
use serde::{Deserialize, Serialize};
use std::collections::{BTreeMap, BTreeSet};
use std::env;
use std::fs;
use std::path::{Component, Path, PathBuf};
use std::process::{Command, ExitCode};

#[derive(Clone, Copy, Debug, Eq, PartialEq, Serialize)]
#[serde(rename_all = "SCREAMING_SNAKE_CASE")]
enum Verdict {
    Pass,
    Fail,
    Blocked,
    InfraError,
    Inconclusive,
}

impl Verdict {
    fn label(self) -> &'static str {
        match self {
            Self::Pass => "PASS",
            Self::Fail => "FAIL",
            Self::Blocked => "BLOCKED",
            Self::InfraError => "INFRA_ERROR",
            Self::Inconclusive => "INCONCLUSIVE",
        }
    }
}

#[derive(Debug, Serialize)]
struct Check {
    id: String,
    verdict: Verdict,
    detail: String,
}

impl Check {
    fn new(id: &str, verdict: Verdict, detail: impl Into<String>) -> Self {
        Self {
            id: id.to_owned(),
            verdict,
            detail: detail.into(),
        }
    }
}

#[derive(Debug, Serialize)]
struct Report {
    schema_version: u32,
    stage: String,
    game_version: String,
    git_revision: String,
    git_dirty: bool,
    checks: Vec<Check>,
    verdict_counts: BTreeMap<String, usize>,
}

impl Report {
    fn new(root: &Path, stage: &str, checks: Vec<Check>) -> Self {
        let identity = build_identity(root);
        let mut verdict_counts = BTreeMap::new();
        for check in &checks {
            *verdict_counts
                .entry(check.verdict.label().to_owned())
                .or_insert(0) += 1;
        }
        Self {
            schema_version: 1,
            stage: stage.to_owned(),
            game_version: load_manifest(root)
                .map(|manifest| manifest.game_version)
                .unwrap_or_else(|_| "unknown".to_owned()),
            git_revision: identity.git_revision,
            git_dirty: identity.git_dirty,
            checks,
            verdict_counts,
        }
    }

    fn code(&self) -> u8 {
        if self.checks.iter().any(|c| c.verdict == Verdict::Fail) {
            1
        } else if self.checks.iter().any(|c| c.verdict == Verdict::InfraError) {
            2
        } else if self.checks.iter().any(|c| c.verdict == Verdict::Blocked) {
            3
        } else if self
            .checks
            .iter()
            .any(|c| c.verdict == Verdict::Inconclusive)
        {
            4
        } else {
            0
        }
    }
}

#[derive(Clone, Debug, Deserialize)]
#[serde(deny_unknown_fields)]
struct Registry {
    schema_version: u32,
    purpose: String,
    factions: Vec<Faction>,
}

#[derive(Clone, Debug, Deserialize)]
#[serde(deny_unknown_fields)]
struct Faction {
    id: String,
    name: String,
    role: String,
    description: String,
    logo: String,
    banner: String,
}

fn safe_relative_path(s: &str) -> bool {
    let path = Path::new(s);
    !s.is_empty()
        && !path.is_absolute()
        && path
            .components()
            .all(|part| matches!(part, Component::Normal(_)))
}

fn check_registry(root: &Path, text: &str) -> Check {
    let data: Registry = match serde_json::from_str(text) {
        Ok(value) => value,
        Err(error) => return Check::new("content.registry", Verdict::Fail, error.to_string()),
    };
    let mut errors = Vec::new();
    if data.schema_version != 1 {
        errors.push("expected content registry schema_version=1".to_owned());
    }
    if data.purpose.trim().is_empty() {
        errors.push("missing registry purpose".to_owned());
    }
    let mut ids = BTreeSet::new();
    let mut names = BTreeSet::new();
    let mut playable = 0;
    let mut npc = 0;
    for faction in &data.factions {
        if !faction
            .id
            .bytes()
            .all(|c| c.is_ascii_lowercase() || c == b'_')
            || faction.id.is_empty()
        {
            errors.push(format!("invalid faction id: {}", faction.id));
        }
        if !ids.insert(faction.id.as_str()) {
            errors.push(format!("duplicate faction id: {}", faction.id));
        }
        if faction.name.trim().is_empty() || !names.insert(faction.name.as_str()) {
            errors.push(format!("empty or duplicate name for {}", faction.id));
        }
        match faction.role.as_str() {
            "playable" => playable += 1,
            "npc" => npc += 1,
            _ => errors.push(format!("unknown role for {}", faction.id)),
        }
        for (field, value) in [
            ("description", &faction.description),
            ("logo", &faction.logo),
            ("banner", &faction.banner),
        ] {
            if !safe_relative_path(value) {
                errors.push(format!("unsafe {field} path for {}", faction.id));
            } else if !root.join(value).is_file() {
                errors.push(format!("missing {field} file for {}: {value}", faction.id));
            }
        }
    }
    let expected = [
        ("atlantic_concord", "The Atlantic Concord", "playable"),
        ("red_directorate", "The Red Directorate", "playable"),
        ("iron_ascendancy", "The Iron Ascendancy", "playable"),
        ("meridian_pact", "The Meridian Pact", "playable"),
        ("unaligned", "The Unaligned", "npc"),
    ];
    if data.factions.len() != expected.len() || playable != 4 || npc != 1 {
        errors.push("expected exactly four playable factions and one NPC faction".to_owned());
    }
    for (id, name, role) in expected {
        if !data
            .factions
            .iter()
            .any(|f| f.id == id && f.name == name && f.role == role)
        {
            errors.push(format!("missing or incorrect faction contract: {id}"));
        }
    }
    if errors.is_empty() {
        Check::new(
            "content.registry",
            Verdict::Pass,
            "five valid faction records, existing referenced files and a 4:1 playable/NPC split",
        )
    } else {
        Check::new("content.registry", Verdict::Fail, errors.join("; "))
    }
}

#[derive(Debug, Eq, PartialEq)]
struct EnginePin {
    version: String,
    tag: String,
    project: String,
    commit: String,
}

fn parse_engine_pin(text: &str) -> Result<EnginePin, String> {
    let mut map = BTreeMap::new();
    for line in text.lines().map(str::trim) {
        if line.is_empty() || line.starts_with('#') {
            continue;
        }
        let (key, value) = line
            .split_once('=')
            .ok_or_else(|| format!("invalid toolchain setting: {line}"))?;
        if !matches!(
            key,
            "UNREAL_VERSION" | "UNREAL_TAG" | "UNREAL_COMMIT" | "UNREAL_PROJECT"
        ) {
            return Err(format!("unknown toolchain setting: {key}"));
        }
        if map
            .insert(key.to_owned(), value.trim().to_owned())
            .is_some()
        {
            return Err(format!("duplicate toolchain setting: {key}"));
        }
    }
    let get = |name: &str| {
        map.get(name)
            .cloned()
            .ok_or_else(|| format!("missing setting: {name}"))
    };
    let pin = EnginePin {
        version: get("UNREAL_VERSION")?,
        tag: get("UNREAL_TAG")?,
        project: get("UNREAL_PROJECT")?,
        commit: get("UNREAL_COMMIT")?,
    };
    if pin.version != "5.8.3" {
        return Err(format!(
            "expected pinned UE 5.8.3, received {}",
            pin.version
        ));
    }
    if pin.tag != "5.8.3-release" {
        return Err("engine version tag does not match 5.8.3-release".to_owned());
    }
    if pin.commit.len() != 40 || !pin.commit.bytes().all(|c| c.is_ascii_hexdigit()) {
        return Err("UNREAL_COMMIT must be a verified 40-character Git SHA".to_owned());
    }
    if !safe_relative_path(&pin.project) || !pin.project.ends_with(".uproject") {
        return Err("UNREAL_PROJECT must be a safe relative .uproject path".to_owned());
    }
    Ok(pin)
}

fn read_pin(root: &Path) -> Result<EnginePin, String> {
    let source = fs::read_to_string(root.join("tools/unreal-version.env"))
        .map_err(|error| format!("cannot read Unreal pin: {error}"))?;
    parse_engine_pin(&source)
}

/// Keys bound to a continuous camera axis must not also issue an order: the axis still
/// fires on the order press and moves the camera by a frame of pan speed.
fn check_input_bindings(input_ini: &str, sources: &[(String, String)]) -> Check {
    let mapping_key = |line: &str, prefix: &str| -> Option<String> {
        let body = line.trim().strip_prefix(prefix)?;
        let start = body.find("Key=")? + "Key=".len();
        let key = body[start..].split([',', ')']).next()?.trim();
        (!key.is_empty()).then(|| key.to_owned())
    };
    let axis_keys: BTreeSet<String> = input_ini
        .lines()
        .filter_map(|line| mapping_key(line, "+AxisMappings="))
        .collect();
    let mut orders: Vec<(String, String)> = input_ini
        .lines()
        .filter_map(|line| {
            let key = mapping_key(line, "+ActionMappings=")?;
            Some((key, "Config/DefaultInput.ini ActionMappings".to_owned()))
        })
        .collect();
    for (path, text) in sources {
        for chunk in text.split("BindKey(EKeys::").skip(1) {
            let key: String = chunk
                .chars()
                .take_while(|c| c.is_ascii_alphanumeric() || *c == '_')
                .collect();
            orders.push((key, format!("{path} BindKey")));
        }
    }
    // One semantic action per chord (GRP-19): a key with modifiers forms its own chord, and a
    // BindKey source file counts as one owner however many events it binds on that key.
    let mut owners: BTreeMap<String, BTreeSet<String>> = BTreeMap::new();
    for line in input_ini.lines() {
        let Some(key) = mapping_key(line, "+ActionMappings=") else {
            continue;
        };
        let action = line
            .split("ActionName=\"")
            .nth(1)
            .and_then(|rest| rest.split('"').next())
            .unwrap_or("?")
            .to_owned();
        let chord: String = ["bShift", "bCtrl", "bAlt", "bCmd"]
            .iter()
            .filter(|flag| line.contains(&format!("{flag}=True")))
            .map(|flag| format!("{}+", &flag[1..]))
            .collect::<String>()
            + &key;
        owners.entry(chord).or_default().insert(action);
    }
    for (key, origin) in orders
        .iter()
        .filter(|(_, origin)| origin.ends_with("BindKey"))
    {
        owners
            .entry(key.clone())
            .or_default()
            .insert(origin.clone());
    }
    let shared: Vec<String> = owners
        .iter()
        .filter(|(_, actions)| actions.len() > 1)
        .map(|(chord, actions)| {
            format!(
                "{chord} ({})",
                actions.iter().cloned().collect::<Vec<_>>().join(" + ")
            )
        })
        .collect();
    if !shared.is_empty() {
        return Check::new(
            "input.bindings",
            Verdict::Fail,
            format!("one chord drives several actions: {}", shared.join(", ")),
        );
    }
    if axis_keys.is_empty() {
        return Check::new(
            "input.bindings",
            Verdict::Fail,
            "Config/DefaultInput.ini declares no axis mappings",
        );
    }
    let conflicts: Vec<String> = orders
        .iter()
        .filter(|(key, _)| axis_keys.contains(key))
        .map(|(key, origin)| format!("{key} ({origin})"))
        .collect();
    if conflicts.is_empty() {
        Check::new(
            "input.bindings",
            Verdict::Pass,
            format!(
                "{} order bindings share no key with {} axis keys",
                orders.len(),
                axis_keys.len()
            ),
        )
    } else {
        Check::new(
            "input.bindings",
            Verdict::Fail,
            format!(
                "order bindings reuse camera axis keys: {}",
                conflicts.join(", ")
            ),
        )
    }
}

fn read_input_bindings(root: &Path) -> Check {
    let input_ini = match fs::read_to_string(root.join("Config/DefaultInput.ini")) {
        Ok(text) => text,
        Err(error) => {
            return Check::new(
                "input.bindings",
                Verdict::Fail,
                format!("cannot read Config/DefaultInput.ini: {error}"),
            );
        }
    };
    let source_dir = root.join("Source/Politika/Private");
    let mut sources = Vec::new();
    let entries = match fs::read_dir(&source_dir) {
        Ok(entries) => entries,
        Err(error) => {
            return Check::new(
                "input.bindings",
                Verdict::Fail,
                format!("cannot read {}: {error}", source_dir.display()),
            );
        }
    };
    for entry in entries.flatten() {
        let path = entry.path();
        if path.extension().is_some_and(|ext| ext == "cpp") {
            if let Ok(text) = fs::read_to_string(&path) {
                let name = path
                    .strip_prefix(root)
                    .unwrap_or(&path)
                    .display()
                    .to_string();
                sources.push((name, text));
            }
        }
    }
    sources.sort();
    check_input_bindings(&input_ini, &sources)
}

fn audit(root: &Path) -> Vec<Check> {
    let mut checks = Vec::new();
    match load_manifest(root) {
        Ok(data) => checks.push(Check::new(
            "version.manifest",
            Verdict::Pass,
            format!(
                "canonical version {} has valid compatibility counters",
                data.game_version
            ),
        )),
        Err(error) => checks.push(Check::new("version.manifest", Verdict::Fail, error)),
    }
    match fs::read_to_string(root.join("Data/content_manifest.json")) {
        Ok(text) => checks.push(check_registry(root, &text)),
        Err(error) => checks.push(Check::new(
            "content.registry",
            Verdict::Fail,
            format!("cannot read content registry: {error}"),
        )),
    }
    match read_pin(root) {
        Ok(pin) => checks.push(Check::new(
            "engine.version_pin",
            Verdict::Pass,
            format!("configured Unreal {} ({})", pin.version, pin.tag),
        )),
        Err(error) => checks.push(Check::new("engine.version_pin", Verdict::Fail, error)),
    }
    checks.push(read_input_bindings(root));
    checks.extend(production::production_audit(root));
    checks
}

fn git_rev(root: &Path) -> Option<String> {
    let result = Command::new("git")
        .arg("-C")
        .arg(root)
        .args(["rev-parse", "HEAD"])
        .output()
        .ok()?;
    if result.status.success() {
        String::from_utf8(result.stdout)
            .ok()
            .map(|s| s.trim().to_owned())
    } else {
        None
    }
}

fn preflight(root: &Path) -> Vec<Check> {
    let mut checks = Vec::new();
    let pin = match read_pin(root) {
        Ok(pin) => pin,
        Err(error) => {
            return vec![Check::new("engine.toolchain", Verdict::Fail, error)];
        }
    };
    let project = env::var_os("POLITIKA_UPROJECT")
        .map(PathBuf::from)
        .unwrap_or_else(|| root.join(&pin.project));
    checks.push(if project.is_file() {
        Check::new(
            "engine.project",
            Verdict::Pass,
            format!("project: {}", project.display()),
        )
    } else {
        Check::new(
            "engine.project",
            Verdict::Blocked,
            format!(
                "no Unreal project at {}; source project not yet available",
                project.display()
            ),
        )
    });
    let engine =
        env::var_os("UNREAL_ENGINE_DIR").or_else(|| env::var_os("POLITIKA_UNREAL_ENGINE_DIR"));
    let engine = match engine {
        Some(path) if !path.is_empty() => PathBuf::from(path),
        _ => {
            checks.push(Check::new(
                "engine.install",
                Verdict::Blocked,
                "set UNREAL_ENGINE_DIR to an installed Unreal 5.8.3 source checkout",
            ));
            return checks;
        }
    };
    let build_json = engine.join("Engine/Build/Build.version");
    let source = match fs::read_to_string(&build_json) {
        Ok(source) => source,
        Err(error) => {
            checks.push(Check::new(
                "engine.install",
                Verdict::Blocked,
                format!("cannot read {}: {error}", build_json.display()),
            ));
            return checks;
        }
    };
    let version: serde_json::Value = match serde_json::from_str(&source) {
        Ok(value) => value,
        Err(error) => {
            checks.push(Check::new(
                "engine.install",
                Verdict::Fail,
                error.to_string(),
            ));
            return checks;
        }
    };
    let actual = format!(
        "{}.{}.{}",
        version["MajorVersion"], version["MinorVersion"], version["PatchVersion"]
    );
    checks.push(if actual == pin.version {
        Check::new(
            "engine.install",
            Verdict::Pass,
            format!("found Unreal {actual}"),
        )
    } else {
        Check::new(
            "engine.install",
            Verdict::Fail,
            format!("expected Unreal {}, found {actual}", pin.version),
        )
    });
    checks.push(match git_rev(&engine) {
        Some(actual) if actual.eq_ignore_ascii_case(&pin.commit) => Check::new(
            "engine.revision",
            Verdict::Pass,
            "installed engine matches the pinned source SHA",
        ),
        Some(actual) => Check::new(
            "engine.revision",
            Verdict::Fail,
            format!(
                "Unreal source revision mismatch: expected {}, found {actual}",
                pin.commit
            ),
        ),
        None => Check::new(
            "engine.revision",
            Verdict::Blocked,
            "cannot verify source SHA: engine is not an accessible Git checkout",
        ),
    });
    checks
}

/// Compile and execute the platform-neutral C++20 rules regression suite.
/// This checks real command-admission decisions, not Unreal gameplay integration.
fn core_tests(root: &Path) -> Vec<Check> {
    let compiler = env::var("CXX").unwrap_or_else(|_| "c++".to_owned());
    let output_path = root.join("build/diagnostics/politika-core-tests");
    if let Some(parent) = output_path.parent() {
        if let Err(error) = fs::create_dir_all(parent) {
            return vec![Check::new(
                "core.compile",
                Verdict::InfraError,
                format!("cannot create C++ test output directory: {error}"),
            )];
        }
    }
    let result = Command::new(&compiler)
        .arg("-std=c++20")
        .arg("-Wall")
        .arg("-Wextra")
        .arg("-Werror")
        .arg("-pedantic")
        .arg("-I")
        .arg(root.join("Source/PolitikaCore"))
        .arg(root.join("Tests/Core/RulesTests.cpp"))
        .arg("-o")
        .arg(&output_path)
        .current_dir(root)
        .output();
    let compile = match result {
        Ok(compile) => compile,
        Err(error) if error.kind() == std::io::ErrorKind::NotFound => {
            return vec![Check::new(
                "core.compile",
                Verdict::Blocked,
                format!("C++20 compiler {compiler:?} is not installed"),
            )];
        }
        Err(error) => {
            return vec![Check::new(
                "core.compile",
                Verdict::InfraError,
                format!("cannot launch compiler: {error}"),
            )];
        }
    };
    if !compile.status.success() {
        let detail: String = String::from_utf8_lossy(&compile.stderr)
            .chars()
            .take(2000)
            .collect();
        return vec![Check::new("core.compile", Verdict::Fail, detail)];
    }
    let mut checks = vec![Check::new(
        "core.compile",
        Verdict::Pass,
        "C++20 rules tests compiled with warnings treated as errors",
    )];
    match Command::new(&output_path).current_dir(root).output() {
        Ok(execution) if execution.status.success() => {
            checks.push(Check::new(
                "core.command_rules", Verdict::Pass,
                "ownership, selection, typed-order admission and phase ordering, atomic resources, and faction-gating assertions passed",
            ));
        }
        Ok(execution) => {
            let detail: String = String::from_utf8_lossy(&execution.stderr)
                .chars()
                .take(2000)
                .collect();
            checks.push(Check::new(
                "core.command_rules",
                Verdict::Fail,
                format!(
                    "C++ regression executable failed ({}): {detail}",
                    execution.status
                ),
            ));
        }
        Err(error) => checks.push(Check::new(
            "core.command_rules",
            Verdict::InfraError,
            format!("cannot execute C++ tests: {error}"),
        )),
    }
    checks
}

fn execute() -> Result<u8, String> {
    let mut args = env::args().skip(1);
    let stage = args.next().ok_or(
        "usage: cargo run --locked --manifest-path tools/version/Cargo.toml --bin quality -- <audit|core|preflight|performance|release|all> [--output PATH]",
    )?;
    if !matches!(
        stage.as_str(),
        "audit" | "core" | "preflight" | "performance" | "release" | "all"
    ) {
        return Err(format!("invalid quality stage: {stage}"));
    }
    let mut output = None;
    while let Some(argument) = args.next() {
        if argument != "--output" || output.is_some() {
            return Err(format!("unrecognized or repeated argument: {argument}"));
        }
        output = Some(PathBuf::from(
            args.next().ok_or("--output needs a file path")?,
        ));
    }
    let root = repo_root();
    let mut checks = Vec::new();
    if stage == "audit" || stage == "all" {
        checks.extend(audit(&root));
    }
    if stage == "core" || stage == "all" {
        checks.extend(core_tests(&root));
    }
    if stage == "preflight" || stage == "all" {
        checks.extend(preflight(&root));
    }
    if stage == "performance" || stage == "all" {
        checks.extend(production::performance_checks(&root));
    }
    if stage == "release" || stage == "all" {
        checks.extend(production::release_gate_checks(&root));
    }
    let report = Report::new(&root, &stage, checks);
    let target = output.unwrap_or_else(|| root.join(format!("build/diagnostics/{stage}.json")));
    let target = if target.is_absolute() {
        target
    } else {
        root.join(target)
    };
    let contents = serde_json::to_string_pretty(&report)
        .map_err(|error| format!("cannot encode diagnostic report: {error}"))?;
    if let Some(parent) = target.parent() {
        fs::create_dir_all(parent)
            .map_err(|error| format!("cannot create report directory: {error}"))?;
    }
    fs::write(&target, format!("{contents}\n"))
        .map_err(|error| format!("cannot write report: {error}"))?;
    for check in &report.checks {
        println!("{} {}: {}", check.verdict.label(), check.id, check.detail);
    }
    println!("report: {}", target.display());
    Ok(report.code())
}

fn main() -> ExitCode {
    match execute() {
        Ok(code) => ExitCode::from(code),
        Err(message) => {
            eprintln!("quality error: {message}");
            ExitCode::from(2)
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    const VALID_PIN: &str = "UNREAL_VERSION=5.8.3\nUNREAL_TAG=5.8.3-release\nUNREAL_COMMIT=396c9f059903aed5fec78ecd3d437a40c6415368\nUNREAL_PROJECT=UnrealPolitika/Politika.uproject\n";

    fn registry_fixture() -> Registry {
        let source = fs::read_to_string(repo_root().join("Data/content_manifest.json")).unwrap();
        serde_json::from_str(&source).unwrap()
    }

    #[test]
    fn content_registry_passes_for_repository_baseline() {
        let source = fs::read_to_string(repo_root().join("Data/content_manifest.json")).unwrap();
        assert_eq!(check_registry(&repo_root(), &source).verdict, Verdict::Pass);
    }

    #[test]
    fn duplicate_faction_fails() {
        let mut manifest = registry_fixture();
        manifest.factions[1].id = manifest.factions[0].id.clone();
        let json = serde_json::json!({
            "schema_version": manifest.schema_version,
            "purpose": manifest.purpose,
            "factions": manifest.factions.iter().map(|f| serde_json::json!({
                "id":f.id,"name":f.name,"role":f.role,
                "description":f.description,"logo":f.logo,"banner":f.banner
            })).collect::<Vec<_>>()
        })
        .to_string();
        assert_eq!(check_registry(&repo_root(), &json).verdict, Verdict::Fail);
    }

    #[test]
    fn registry_rejects_parent_directory_paths() {
        assert!(!safe_relative_path("../README.md"));
        assert!(!safe_relative_path("/tmp/image.png"));
        assert!(safe_relative_path("Factions/TheUnaligned/description.md"));
    }

    #[test]
    fn engine_pin_is_exact_and_validated() {
        assert_eq!(parse_engine_pin(VALID_PIN).unwrap().version, "5.8.3");
        assert!(parse_engine_pin(&VALID_PIN.replace("5.8.3", "5.8.2")).is_err());
        assert!(parse_engine_pin(&(VALID_PIN.to_owned() + "UNREAL_TAG=bad\n")).is_err());
    }

    #[test]
    fn input_bindings_reject_order_keys_on_pan_axes() {
        let axes = "+AxisMappings=(AxisName=\"PrototypePanForward\",Scale=-1.000000,Key=S)\n\
                    +AxisMappings=(AxisName=\"PrototypePanRight\",Scale=-1.000000,Key=A)\n";
        let action = "+ActionMappings=(ActionName=\"PrototypeStop\",bShift=False,Key=X)\n";
        assert_eq!(
            check_input_bindings(&(axes.to_owned() + action), &[]).verdict,
            Verdict::Pass
        );
        let shared_action = action.replace("Key=X", "Key=S");
        let failed = check_input_bindings(&(axes.to_owned() + &shared_action), &[]);
        assert_eq!(failed.verdict, Verdict::Fail);
        assert!(failed.detail.contains("S (Config/DefaultInput.ini"));
        let source = (
            "Gameplay.cpp".to_owned(),
            "InputComponent->BindKey(EKeys::A, IE_Pressed, this, &Self::AttackMove);".to_owned(),
        );
        let failed = check_input_bindings(axes, &[source]);
        assert_eq!(failed.verdict, Verdict::Fail);
        assert!(failed.detail.contains("A (Gameplay.cpp BindKey)"));
        assert_eq!(check_input_bindings(action, &[]).verdict, Verdict::Fail);
        // A chord drives one action; a modifier makes a distinct chord, and one source file
        // binding pressed and released on the same key is one owner.
        let guard = "+ActionMappings=(ActionName=\"PrototypeGuard\",bShift=False,Key=X)\n";
        let failed = check_input_bindings(&(axes.to_owned() + action + guard), &[]);
        assert_eq!(failed.verdict, Verdict::Fail);
        assert!(failed.detail.contains("X (PrototypeGuard + PrototypeStop)"));
        let shifted = guard.replace("bShift=False", "bShift=True");
        assert_eq!(
            check_input_bindings(&(axes.to_owned() + action + &shifted), &[]).verdict,
            Verdict::Pass
        );
        let mouse = (
            "Gameplay.cpp".to_owned(),
            "BindKey(EKeys::LeftMouseButton, IE_Pressed); BindKey(EKeys::LeftMouseButton, IE_Released);"
                .to_owned(),
        );
        assert_eq!(
            check_input_bindings(&(axes.to_owned() + action), &[mouse]).verdict,
            Verdict::Pass
        );
        let tab = (
            "Other.cpp".to_owned(),
            "BindKey(EKeys::X, IE_Pressed);".to_owned(),
        );
        let failed = check_input_bindings(&(axes.to_owned() + action), &[tab]);
        assert_eq!(failed.verdict, Verdict::Fail);
        assert!(
            failed
                .detail
                .contains("X (Other.cpp BindKey + PrototypeStop)")
        );
    }

    #[test]
    fn audit_passes_current_design_source() {
        assert!(
            audit(&repo_root())
                .iter()
                .all(|c| c.verdict == Verdict::Pass)
        );
    }

    #[test]
    fn report_distinguishes_blocked_from_pass() {
        let blocked = Report::new(
            Path::new("."),
            "preflight",
            vec![Check::new(
                "engine.project",
                Verdict::Blocked,
                "missing project",
            )],
        );
        assert_eq!(blocked.code(), 3);
        assert_eq!(
            serde_json::to_value(&blocked).unwrap()["checks"][0]["verdict"],
            "BLOCKED"
        );
        let failed = Report::new(
            Path::new("."),
            "audit",
            vec![Check::new("content.registry", Verdict::Fail, "bad content")],
        );
        assert_eq!(failed.code(), 1);
    }
}
