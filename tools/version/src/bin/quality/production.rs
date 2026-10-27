//! Production metadata and documentation checks for POLITIKA.
//! A valid planning registry is NOT evidence that its content is implemented.
use super::{Check, Verdict, safe_relative_path};
use serde_json::Value;
use std::collections::BTreeSet;
use std::fs;
use std::path::Path;

fn load_json(root: &Path, relative: &str) -> Result<Value, String> {
    let source = fs::read_to_string(root.join(relative))
        .map_err(|e| format!("{relative}: cannot read: {e}"))?;
    let value: Value = serde_json::from_str(&source)
        .map_err(|e| format!("{relative}: invalid JSON: {e}"))?;
    if value["schema_version"].as_u64() != Some(1) {
        return Err(format!("{relative}: expected schema_version 1"));
    }
    Ok(value)
}

fn value_string<'a>(value: &'a Value, field: &str) -> Result<&'a str, String> {
    let text = value[field].as_str()
        .ok_or_else(|| format!("{field} must be a string"))?;
    if text.trim().is_empty() {
        Err(format!("{field} must be non-empty"))
    } else {
        Ok(text)
    }
}

fn unique_id(value: &Value, ids: &mut BTreeSet<String>) -> Result<String, String> {
    let id = value_string(value, "id")?;
    if !id.bytes().all(|c| c.is_ascii_lowercase() || c == b'_' || c.is_ascii_digit())
        || !id.bytes().next().is_some_and(|c| c.is_ascii_lowercase())
    {
        return Err(format!("invalid semantic ID {id}"));
    }
    if !ids.insert(id.to_owned()) {
        return Err(format!("duplicate ID {id}"));
    }
    Ok(id.to_owned())
}

fn list<'a>(value: &'a Value, key: &str) -> Result<&'a Vec<Value>, String> {
    value[key].as_array().ok_or_else(|| format!("{key} must be an array"))
}

fn is_null_or_missing(value: &Value, field: &str) -> bool {
    value.get(field).is_none_or(Value::is_null)
}

fn check_historical(value: &Value) -> Result<usize, String> {
    let entries = list(value, "decisions")?;
    let mut ids = BTreeSet::new();
    for entry in entries {
        let id = unique_id(entry, &mut ids)?;
        let status = value_string(entry, "status")?;
        if !matches!(status, "proposed" | "needs_review" | "approved" | "rejected") {
            return Err(format!("{id}: invalid historical decision status {status}"));
        }
        value_string(entry, "scope")?;
        value_string(entry, "rationale")?;
        let sources = list(entry, "source_refs")?;
        if status == "approved" && (sources.is_empty()
            || !sources.iter().all(|s| s.as_str().is_some_and(|x| !x.trim().is_empty()))
            || value_string(entry, "reviewer").is_err())
        {
            return Err(format!("{id}: approved decisions need sources and reviewer"));
        }
    }
    Ok(entries.len())
}

fn check_maps(root: &Path, value: &Value, historical: &Value) -> Result<usize, String> {
    let entries = list(value, "sources")?;
    let csv = fs::read_to_string(root.join("Archive/MAP_CATALOG.csv"))
        .map_err(|e| format!("cannot read map source catalogue: {e}"))?;
    let decisions = list(historical, "decisions")?;
    let mut ids = BTreeSet::new();
    for entry in entries {
        let id = value_string(entry, "source_id")?;
        if !ids.insert(id.to_owned()) {
            return Err(format!("duplicate map source {id}"));
        }
        if entry["source_catalog"].as_str() != Some("Archive/MAP_CATALOG.csv") {
            return Err(format!("{id}: unrecognized map catalogue"));
        }
        // Map IDs are the first, quoted CSV column; commas/quotes are not legal in IDs.
        if id.contains(',') || id.contains('"')
            || !csv.lines().any(|line| line.starts_with(&format!("\"{id}\",")))
        {
            return Err(format!("{id}: source ID not present in Archive/MAP_CATALOG.csv"));
        }
        value_string(entry, "intended_use")?;
        value_string(entry, "period_status")?;
        let status = value_string(entry, "production_status")?;
        if !matches!(status, "research_only" | "needs_review" | "approved" | "imported") {
            return Err(format!("{id}: invalid map production status {status}"));
        }
        if matches!(status, "approved" | "imported") {
            let decision = value_string(entry, "historical_decision_id")?;
            if !decisions.iter().any(|d| d["id"].as_str() == Some(decision)
                && d["status"].as_str() == Some("approved")) {
                return Err(format!("{id}: production map requires an approved historical decision"));
            }
            if status == "imported" {
                let asset = value_string(entry, "derived_asset")?;
                if !safe_relative_path(asset) || !root.join(asset).is_file() {
                    return Err(format!("{id}: imported asset file does not exist or is unsafe"));
                }
            }
        }
    }
    Ok(entries.len())
}

fn check_audio(root: &Path, value: &Value) -> Result<usize, String> {
    let entries = list(value, "events")?;
    let mut ids = BTreeSet::new();
    for entry in entries {
        let id = unique_id(entry, &mut ids)?;
        if !matches!(value_string(entry, "bus")?, "ui" | "sfx" | "ambience" | "music" | "voice" | "broadcast") {
            return Err(format!("{id}: unknown audio bus"));
        }
        if !matches!(value_string(entry, "role")?, "one_shot" | "loop" | "music" | "stream") {
            return Err(format!("{id}: unknown audio role"));
        }
        let status = value_string(entry, "status")?;
        if !matches!(status, "planned" | "authored" | "imported") {
            return Err(format!("{id}: invalid audio status"));
        }
        if status == "planned" && !is_null_or_missing(entry, "asset") {
            return Err(format!("{id}: a planned audio event must not claim an asset"));
        }
        if status != "planned" {
            let asset = value_string(entry, "asset")?;
            if !safe_relative_path(asset) || !root.join(asset).is_file() {
                return Err(format!("{id}: audio asset does not exist or has unsafe path"));
            }
            value_string(entry, "rights_status")?;
        }
    }
    Ok(entries.len())
}

fn check_performance(value: &Value) -> Result<usize, String> {
    let fixtures = list(value, "fixtures")?;
    let mut ids = BTreeSet::new();
    for entry in fixtures {
        let id = unique_id(entry, &mut ids)?;
        let status = value_string(entry, "status")?;
        if !matches!(status, "planned" | "implemented" | "measured") {
            return Err(format!("{id}: invalid performance status"));
        }
        let metrics = list(entry, "metrics")?;
        if metrics.is_empty() {
            return Err(format!("{id}: no metrics declared"));
        }
        let mut labels = BTreeSet::new();
        for metric in metrics {
            let label = metric.as_str()
                .filter(|s| !s.is_empty())
                .ok_or_else(|| format!("{id}: invalid metric"))?;
            if !labels.insert(label) {
                return Err(format!("{id}: duplicate metric {label}"));
            }
        }
        if status == "planned" && (!is_null_or_missing(entry, "map_id")
            || !is_null_or_missing(entry, "unit_count")
            || !is_null_or_missing(entry, "seed"))
        {
            return Err(format!("{id}: planned fixture cannot imply an existing test scene"));
        }
        if status != "planned" && (value_string(entry, "map_id").is_err()
            || entry["unit_count"].as_u64().is_none()
            || entry["seed"].as_u64().is_none())
        {
            return Err(format!("{id}: implemented fixture requires map, seed and unit count"));
        }
        // The 'measured' label is a declaration, not independent proof; actual
        // numeric results belong to engine-generated diagnostic artifacts.
    }
    Ok(fixtures.len())
}

const MANDATORY_RELEASE_GATES: [&str; 7] = [
    "engine_build", "first_skirmish", "gameplay_regressions",
    "content_and_rights", "performance", "accessibility_and_inputs", "packaged_release",
];

fn check_release(value: &Value) -> Result<usize, String> {
    let gates = list(value, "gates")?;
    let mut ids = BTreeSet::new();
    for entry in gates {
        let id = unique_id(entry, &mut ids)?;
        if !MANDATORY_RELEASE_GATES.contains(&id.as_str()) {
            return Err(format!("{id}: unknown release gate"));
        }
        let status = value_string(entry, "status")?;
        if !matches!(status, "blocked" | "fail" | "pass") {
            return Err(format!("{id}: invalid release gate verdict"));
        }
        let evidence = list(entry, "evidence")?;
        if status == "pass" && (evidence.is_empty()
            || !evidence.iter().all(|item| item.as_str().is_some_and(|s| !s.trim().is_empty())))
        {
            return Err(format!("{id}: cannot claim PASS without evidence references"));
        }
        value_string(entry, "note")?;
    }
    if ids.len() != MANDATORY_RELEASE_GATES.len() {
        return Err("release ledger must contain all seven required unique gates".to_owned());
    }
    Ok(gates.len())
}

fn check_index(root: &Path) -> Result<usize, String> {
    let index = fs::read_to_string(root.join("Docs/DOCUMENTATION_INDEX.md"))
        .map_err(|e| format!("cannot read documentation index: {e}"))?;
    let mut count = 0;
    for segment in index.split("](").skip(1) {
        let path = segment.split(')').next().unwrap_or("");
        if path.is_empty() || path.starts_with("http://") || path.starts_with("https://")
            || path.starts_with('#') { continue; }
        let path = path.split('#').next().unwrap_or("");
        let mut relative = std::path::PathBuf::from("Docs");
        for component in Path::new(path).components() {
            match component {
                std::path::Component::Normal(piece) => relative.push(piece),
                std::path::Component::ParentDir => { relative.pop(); },
                std::path::Component::CurDir => {},
                _ => return Err(format!("unsafe index reference: {path}")),
            }
        }
        if !root.join(&relative).exists() {
            return Err(format!("documentation index has dangling link: {path}"));
        }
        count += 1;
    }
    if count == 0 {
        return Err("documentation index has no local links".to_owned());
    }
    Ok(count)
}

fn check_result(id: &str, result: Result<usize, String>, what: &str) -> Check {
    match result {
        Ok(count) => Check::new(id, Verdict::Pass, format!("validated {count} {what}")),
        Err(error) => Check::new(id, Verdict::Fail, error),
    }
}

/// Strictly structural/source audit; not a claim that assets, performance or releases exist.
pub(super) fn production_audit(root: &Path) -> Vec<Check> {
    let mut checks = Vec::new();
    let historical = load_json(root, "Data/historical_decisions.json");
    checks.push(check_result("history.decisions",
        historical.as_ref().map_err(Clone::clone).and_then(check_historical),
        "historical decisions"));
    let maps = load_json(root, "Data/map_production.json");
    checks.push(check_result("maps.sources",
        maps.as_ref().map_err(Clone::clone).and_then(|map| {
            historical.as_ref().map_err(Clone::clone)
                .and_then(|history| check_maps(root, map, history))
        }), "map source references"));
    checks.push(check_result("audio.events",
        load_json(root, "Data/audio_events.json")
            .and_then(|value| check_audio(root, &value)), "audio event definitions"));
    checks.push(check_result("performance.fixtures",
        load_json(root, "Data/performance_cases.json")
            .and_then(|value| check_performance(&value)), "planned performance fixtures"));
    checks.push(check_result("release.ledger_schema",
        load_json(root, "Data/release_matrix.json")
            .and_then(|value| check_release(&value)), "release gate records"));
    checks.push(check_result("docs.index_links", check_index(root),
        "documentation index links"));
    checks
}

/// A release stage is intentionally BLOCKED without verified in-engine evidence.
/// Data-ledger PASS labels alone do not certify a release.
pub(super) fn release_gate_checks(root: &Path) -> Vec<Check> {
    let value = match load_json(root, "Data/release_matrix.json") {
        Ok(value) => value,
        Err(error) => return vec![Check::new("release.readiness", Verdict::Fail, error)],
    };
    if let Err(error) = check_release(&value) {
        return vec![Check::new("release.readiness", Verdict::Fail, error)];
    }
    let gates = match list(&value, "gates") {
        Ok(gates) => gates,
        Err(error) => return vec![Check::new("release.readiness", Verdict::Fail, error)],
    };
    gates.iter().map(|gate| {
        let id = gate["id"].as_str().unwrap_or("unknown");
        let status = gate["status"].as_str().unwrap_or("blocked");
        let note = gate["note"].as_str().unwrap_or("");
        let verdict = match status {
            "blocked" => Verdict::Blocked,
            "fail" => Verdict::Fail,
            "pass" => Verdict::Inconclusive, // ledger assertion is not independent evidence
            _ => Verdict::Fail,
        };
        let suffix = if status == "pass" {
            "ledger claims PASS; independent package/test evidence still required"
        } else { note };
        Check::new(&format!("release.{id}"), verdict, suffix)
    }).collect()
}

/// Report honest performance readiness; planned fixtures are not benchmarks.
pub(super) fn performance_checks(root: &Path) -> Vec<Check> {
    let value = match load_json(root, "Data/performance_cases.json") {
        Ok(value) => value,
        Err(error) => return vec![Check::new("performance.readiness", Verdict::Fail, error)],
    };
    if let Err(error) = check_performance(&value) {
        return vec![Check::new("performance.readiness", Verdict::Fail, error)];
    }
    let fixtures = match list(&value, "fixtures") {
        Ok(fixtures) => fixtures,
        Err(error) => return vec![Check::new("performance.readiness", Verdict::Fail, error)],
    };
    fixtures.iter().map(|fixture| {
        let id = fixture["id"].as_str().unwrap_or("unknown");
        let state = fixture["status"].as_str().unwrap_or("planned");
        let verdict = if state == "planned" {
            Verdict::Blocked
        } else {
            Verdict::Inconclusive
        };
        Check::new(
            &format!("performance.{id}"), verdict,
            if state == "planned" {
                "no executable RTS benchmark fixture or measurements yet"
            } else {
                "fixture declaration cannot independently verify measured Unreal performance"
            },
        )
    }).collect()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn registry_files_are_structurally_consistent() {
        let checks = production_audit(&super::super::repo_root());
        assert!(checks.iter().all(|c| c.verdict == Verdict::Pass),
            "production manifest checks: {checks:?}");
    }

    #[test]
    fn rejects_duplicate_audio_events() {
        let root = super::super::repo_root();
        let mut doc = load_json(&root, "Data/audio_events.json").unwrap();
        let dupe = doc["events"][0].clone();
        doc["events"].as_array_mut().unwrap().push(dupe);
        assert!(check_audio(&root, &doc).is_err());
    }

    #[test]
    fn rejects_unknown_map_archive_source() {
        let root = super::super::repo_root();
        let mut doc = load_json(&root, "Data/map_production.json").unwrap();
        doc["sources"][0]["source_id"] = Value::from("POL-MAP-NONEXISTENT");
        let history = load_json(&root, "Data/historical_decisions.json").unwrap();
        assert!(check_maps(&root, &doc, &history).is_err());
    }

    #[test]
    fn rejects_empty_release_pass_evidence() {
        let root = super::super::repo_root();
        let mut doc = load_json(&root, "Data/release_matrix.json").unwrap();
        doc["gates"][0]["status"] = Value::from("pass");
        assert!(check_release(&doc).is_err());
    }

    #[test]
    fn unverified_release_is_blocked() {
        let checks = release_gate_checks(&super::super::repo_root());
        assert_eq!(checks.len(), 7);
        assert!(checks.iter().all(|check| check.verdict == Verdict::Blocked));
    }

    #[test]
    fn current_benchmarks_are_unexecuted() {
        let checks = performance_checks(&super::super::repo_root());
        assert_eq!(checks.len(), 4);
        assert!(checks.iter().all(|check| check.verdict == Verdict::Blocked));
    }

    #[test]
    fn planned_audio_cannot_claim_files() {
        let root = super::super::repo_root();
        let mut doc = load_json(&root, "Data/audio_events.json").unwrap();
        doc["events"][0]["asset"] = Value::from("invented.wav");
        assert!(check_audio(&root, &doc).is_err());
    }
}
