//! Shared POLITIKA version contract and build identity logic.
//! version.json is the only manually maintained game version source.
use semver::Version;
use serde::{Deserialize, Serialize};
use std::env;
use std::fs;
use std::path::{Path, PathBuf};
use std::process::Command;

#[derive(Clone, Debug, Serialize, Deserialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct Manifest {
    pub game_version: String,
    pub network_protocol: u64,
    pub save_schema: u64,
    pub persistence_schema: u64,
    pub world_schema: u64,
}

#[derive(Clone, Debug, Serialize, PartialEq, Eq)]
pub struct BuildIdentity {
    pub build_number: String,
    pub git_revision: String,
    pub git_dirty: bool,
}

pub fn repo_root() -> PathBuf {
    Path::new(env!("CARGO_MANIFEST_DIR"))
        .parent()
        .expect("tools/version has a parent")
        .parent()
        .expect("tools has a parent")
        .to_path_buf()
}

pub fn parse_manifest(source: &str) -> Result<Manifest, String> {
    let data: Manifest =
        serde_json::from_str(source).map_err(|error| format!("invalid version.json: {error}"))?;
    Version::parse(&data.game_version)
        .map_err(|error| format!("invalid SemVer game_version: {error}"))?;
    for (name, value) in [
        ("network_protocol", data.network_protocol),
        ("save_schema", data.save_schema),
        ("persistence_schema", data.persistence_schema),
        ("world_schema", data.world_schema),
    ] {
        if value == 0 {
            return Err(format!("{name} must be a positive integer"));
        }
    }
    Ok(data)
}

pub fn load_manifest(root: &Path) -> Result<Manifest, String> {
    let path = root.join("version.json");
    let source = fs::read_to_string(&path)
        .map_err(|error| format!("cannot read {}: {error}", path.display()))?;
    parse_manifest(&source)
}

fn git(root: &Path, args: &[&str]) -> Option<String> {
    let output = Command::new("git")
        .args(args)
        .current_dir(root)
        .output()
        .ok()?;
    if !output.status.success() {
        return None;
    }
    Some(String::from_utf8(output.stdout).ok()?.trim().to_owned())
}

pub fn build_identity(root: &Path) -> BuildIdentity {
    let build_number = env::var("POLITIKA_BUILD_NUMBER")
        .ok()
        .filter(|s| !s.is_empty())
        .or_else(|| env::var("GITHUB_RUN_NUMBER").ok().filter(|s| !s.is_empty()))
        .unwrap_or_else(|| "local".to_owned());
    let full_revision = env::var("GITHUB_SHA")
        .ok()
        .filter(|s| !s.is_empty())
        .or_else(|| git(root, &["rev-parse", "HEAD"]))
        .unwrap_or_else(|| "unknown".to_owned());
    let git_dirty = git(root, &["status", "--porcelain", "--untracked-files=all"])
        .is_some_and(|status| !status.is_empty());
    let mut git_revision: String = full_revision.chars().take(12).collect();
    if git_dirty {
        git_revision.push_str("-dirty");
    }
    BuildIdentity {
        build_number,
        git_revision,
        git_dirty,
    }
}

pub fn current_tag() -> Option<String> {
    let git_ref = env::var("GITHUB_REF").unwrap_or_default();
    if env::var("GITHUB_REF_TYPE").as_deref() == Ok("tag") || git_ref.starts_with("refs/tags/") {
        env::var("GITHUB_REF_NAME")
            .ok()
            .filter(|value| !value.is_empty())
            .or_else(|| git_ref.strip_prefix("refs/tags/").map(str::to_owned))
    } else {
        None
    }
}

pub fn validate_tag(manifest: &Manifest, tag: Option<&str>) -> Result<(), String> {
    if let Some(actual) = tag {
        let expected = format!("v{}", manifest.game_version);
        if actual != expected {
            return Err(format!("tag {actual:?} does not match {expected}"));
        }
    }
    Ok(())
}

/// In release mode, fail if the checkout is dirty, untracked, missing or
/// detached from the requested tag, even if its display version is correct.
pub fn validate_release(root: &Path, manifest: &Manifest, tag: Option<&str>) -> Result<(), String> {
    validate_tag(manifest, tag)?;
    let tag = tag.ok_or("release builds require an exact vVERSION tag")?;
    if git(root, &["rev-parse", "--is-inside-work-tree"]).as_deref() != Some("true") {
        return Err("release builds require a Git checkout".into());
    }
    let status = git(root, &["status", "--porcelain", "--untracked-files=all"])
        .ok_or("release build cannot inspect Git worktree status")?;
    if !status.is_empty() {
        return Err("release builds require a clean Git worktree".into());
    }
    let head = git(root, &["rev-parse", "HEAD"]).ok_or("release builds require a Git revision")?;
    let tag_commit = git(root, &["rev-list", "-n", "1", tag])
        .ok_or("release builds require a valid Git version tag")?;
    if head != tag_commit {
        return Err("release tag must resolve to the checked-out commit".into());
    }
    if let Ok(github_sha) = env::var("GITHUB_SHA") {
        if !github_sha.is_empty() && github_sha != head {
            return Err("GITHUB_SHA does not match the checked-out commit".into());
        }
    }
    Ok(())
}

pub fn header(manifest: &Manifest, build: &BuildIdentity) -> String {
    let quote = |s: &str| serde_json::to_string(s).expect("strings are JSON serializable");
    format!(
        "// Generated from version.json by tools/version (Rust). Do not edit.\n\
         #pragma once\n\
         #include <cstdint>\n\
         \n\
         namespace politika::version {{\n\
         inline constexpr char GAME_VERSION[] = {};\n\
         inline constexpr std::int64_t NETWORK_PROTOCOL = {};\n\
         inline constexpr std::int64_t SAVE_SCHEMA = {};\n\
         inline constexpr std::int64_t PERSISTENCE_SCHEMA = {};\n\
         inline constexpr std::int64_t WORLD_SCHEMA = {};\n\
         inline constexpr char BUILD_NUMBER[] = {};\n\
         inline constexpr char GIT_REVISION[] = {};\n\
         }} // namespace politika::version\n",
        quote(&manifest.game_version),
        manifest.network_protocol,
        manifest.save_schema,
        manifest.persistence_schema,
        manifest.world_schema,
        quote(&build.build_number),
        quote(&build.git_revision)
    )
}

pub fn identity_json(manifest: &Manifest, build: &BuildIdentity) -> String {
    let mut value = serde_json::to_value(manifest).expect("manifest can be serialized");
    let map = value.as_object_mut().expect("manifest is a JSON object");
    map.insert(
        "build_number".to_owned(),
        serde_json::json!(build.build_number),
    );
    map.insert(
        "git_revision".to_owned(),
        serde_json::json!(build.git_revision),
    );
    map.insert("git_dirty".to_owned(), serde_json::json!(build.git_dirty));
    serde_json::to_string_pretty(&value).expect("identity can be serialized")
}

pub fn write_if_changed(path: &Path, content: &str) -> Result<(), String> {
    if fs::read_to_string(path).ok().as_deref() == Some(content) {
        return Ok(());
    }
    if let Some(parent) = path.parent() {
        fs::create_dir_all(parent)
            .map_err(|error| format!("cannot create {}: {error}", parent.display()))?;
    }
    fs::write(path, content).map_err(|error| format!("cannot write {}: {error}", path.display()))
}

#[cfg(test)]
mod tests {
    use super::*;

    const GOOD: &str = r#"{
        "game_version":"0.1.0-dev.0",
        "network_protocol":1,"save_schema":1,"persistence_schema":1,"world_schema":1
    }"#;

    #[test]
    fn baseline_version_manifest() {
        let data = parse_manifest(GOOD).unwrap();
        assert_eq!(data.game_version, "0.1.0-dev.0");
        assert_eq!(data.network_protocol, 1);
    }

    #[test]
    fn accepts_semver_examples() {
        let values = [
            "0.1.0-dev.0",
            "0.1.0-alpha.1",
            "0.1.0-beta.2",
            "0.1.0-rc.1",
            "0.1.0",
            "0.1.1",
            "1.0.0",
            "1.2.3+ci.42.gabc",
            "1.2.3-alpha.1+ci.7",
        ];
        for version in values {
            parse_manifest(&GOOD.replace("0.1.0-dev.0", version)).unwrap();
        }
    }

    #[test]
    fn rejects_invalid_semver() {
        for version in [
            "",
            "v0.1.0",
            "01.0.0",
            "1.02.3",
            "1.2",
            "1.2.3-01",
            "1.2.3-alpha.01",
            "1.2.3+",
            "1.2.3-",
        ] {
            assert!(
                parse_manifest(&GOOD.replace("0.1.0-dev.0", version)).is_err(),
                "{version}"
            );
        }
    }

    #[test]
    fn rejects_missing_fields_wrong_types_zero_counters_and_unknown_fields() {
        for json in [
            r#"{"game_version":"0.1.0"}"#,
            &GOOD.replace(r#""save_schema":1,"#, ""),
            &GOOD.replace(r#""world_schema":1"#, r#""world_schema":0"#),
            &GOOD.replace(r#""world_schema":1"#, r#""world_schema":true"#),
            &GOOD.replace(r#""world_schema":1"#, r#""world_schema":1.2"#),
            &GOOD.replace(r#""world_schema":1"#, r#""world_schema":"1""#),
            &GOOD.replace(r#""world_schema":1"#, r#""world_schema":-1"#),
            &GOOD.replace(r#""world_schema":1"#, r#""world_schema":null"#),
            &GOOD.replace(r#""world_schema":1"#, r#""world_schema":1,"other":1"#),
        ] {
            assert!(parse_manifest(json).is_err(), "{json}");
        }
    }

    #[test]
    fn matching_tag_is_required_for_versioned_release() {
        let data = parse_manifest(GOOD).unwrap();
        assert!(validate_tag(&data, None).is_ok());
        assert!(validate_tag(&data, Some("v0.1.0-dev.0")).is_ok());
        assert!(validate_tag(&data, Some("v0.2.0")).is_err());
        assert!(validate_release(Path::new("."), &data, None).is_err());
    }

    #[test]
    fn header_contains_stable_fields_and_is_deterministic() {
        let data = parse_manifest(GOOD).unwrap();
        let identity = BuildIdentity {
            build_number: "37".into(),
            git_revision: "abc123-dirty".into(),
            git_dirty: true,
        };
        let result = header(&data, &identity);
        for field in [
            "GAME_VERSION[] = \"0.1.0-dev.0\"",
            "NETWORK_PROTOCOL = 1",
            "SAVE_SCHEMA = 1",
            "PERSISTENCE_SCHEMA = 1",
            "WORLD_SCHEMA = 1",
            "BUILD_NUMBER[] = \"37\"",
            "GIT_REVISION[] = \"abc123-dirty\"",
        ] {
            assert!(result.contains(field), "missing: {field}");
        }
        assert_eq!(result, header(&data, &identity));
        let json: serde_json::Value =
            serde_json::from_str(&identity_json(&data, &identity)).unwrap();
        assert_eq!(json["git_dirty"], true);
    }

    #[test]
    fn embedded_manifest_is_valid() {
        let result = load_manifest(&repo_root()).unwrap();
        assert_eq!(result.game_version, "0.1.0-dev.0");
    }
}
