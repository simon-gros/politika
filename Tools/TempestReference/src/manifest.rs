use std::{
    fs,
    io::Write,
    path::{Path, PathBuf},
    process::{Command, Stdio},
};

#[derive(Debug)]
pub struct ResultManifest {
    pub hash_lines: String,
    pub json: String,
}

pub fn build(
    inputs: &[PathBuf],
    asset_rows: usize,
    tag_rows: usize,
    duplicates: usize,
    expected_input_hashes: &str,
) -> Result<ResultManifest, String> {
    if inputs.is_empty() {
        return Err("no input files to hash".into());
    }
    let lines = hash_inputs(inputs)?;
    if lines != expected_input_hashes {
        return Err("source inputs changed while indexing; no complete manifest written".into());
    }
    let mut json_inputs = Vec::new();
    for path in inputs {
        let output = Command::new("sha256sum")
            .arg(path)
            .output()
            .map_err(|e| format!("could not run sha256sum for {}: {e}", path.display()))?;
        if !output.status.success() {
            return Err(format!("sha256sum failed for {}", path.display()));
        }
        let line = String::from_utf8(output.stdout).map_err(|e| e.to_string())?;
        let (hash, _) = line
            .trim_end()
            .split_once("  ")
            .ok_or_else(|| format!("unexpected sha256sum output for {}", path.display()))?;
        let name = path.to_string_lossy().into_owned();
        json_inputs.push(format!(
            "{{\"path\":\"{}\",\"sha256\":\"{}\"}}",
            escape(&name),
            hash
        ));
    }
    let root = Path::new(env!("CARGO_MANIFEST_DIR"));
    let source_files = [
        "Cargo.toml",
        "Cargo.lock",
        "src/main.rs",
        "src/crosswalk.rs",
        "src/registry.rs",
        "src/tags.rs",
        "src/manifest.rs",
    ];
    let mut source_fingerprints = Vec::new();
    let mut source_revision_material = String::new();
    for relative in source_files {
        let path = root.join(relative);
        if !path.is_file() {
            return Err(format!("tool source missing: {}", path.display()));
        }
        let hash = hash_file(&path)?;
        source_revision_material.push_str(&format!("{relative} {hash}\n"));
        source_fingerprints.push(format!(
            "{{\"path\":\"{}\",\"sha256\":\"{}\"}}",
            escape(relative),
            hash
        ));
    }
    let tool_revision = hash_bytes(source_revision_material.as_bytes())?;
    let json = format!(
        "{{\n  \"tool\": \"tempest-reference\",\n  \"version\": \"{}\",\n  \"tool_revision_sha256\": \"{}\",\n  \"tool_sources\": [{}],\n  \"filters\": \"queries.tsv\",\n  \"inputs\": [{}],\n  \"counts\": {{\"selected_registry_rows\": {}, \"tag_entries\": {}, \"duplicate_registry_rows\": {}}},\n  \"limits\": [\"Registry paths and tag configuration are discovery evidence only.\", \"No runtime semantics, license, reuse rights, or UE 5.8.3 loadability are asserted.\", \"Case variants are preserved exactly.\"]\n}}\n",
        env!("CARGO_PKG_VERSION"),
        tool_revision,
        source_fingerprints.join(", "),
        json_inputs.join(", "),
        asset_rows,
        tag_rows,
        duplicates
    );
    Ok(ResultManifest {
        hash_lines: lines,
        json,
    })
}

pub fn hash_inputs(inputs: &[PathBuf]) -> Result<String, String> {
    if inputs.is_empty() {
        return Err("no input files to hash".into());
    }
    let mut lines = String::new();
    for path in inputs {
        if !path.is_file() {
            return Err(format!("input missing: {}", path.display()));
        }
        let output = Command::new("sha256sum")
            .arg(path)
            .output()
            .map_err(|e| format!("could not run sha256sum for {}: {e}", path.display()))?;
        if !output.status.success() {
            return Err(format!("sha256sum failed for {}", path.display()));
        }
        let line = String::from_utf8(output.stdout).map_err(|e| e.to_string())?;
        let (hash, _) = line
            .trim_end()
            .split_once("  ")
            .ok_or_else(|| format!("unexpected sha256sum output for {}", path.display()))?;
        lines.push_str(&format!("{hash}  {}\n", path.display()));
    }
    Ok(lines)
}

pub fn hash_file(path: &Path) -> Result<String, String> {
    let output = Command::new("sha256sum")
        .arg(path)
        .output()
        .map_err(|e| format!("could not hash {}: {e}", path.display()))?;
    if !output.status.success() {
        return Err(format!("sha256sum failed for {}", path.display()));
    }
    let text = String::from_utf8(output.stdout).map_err(|e| e.to_string())?;
    text.split_whitespace()
        .next()
        .map(str::to_string)
        .ok_or_else(|| format!("unexpected sha256sum output for {}", path.display()))
}

fn hash_bytes(bytes: &[u8]) -> Result<String, String> {
    let mut child = Command::new("sha256sum")
        .arg("-")
        .stdin(Stdio::piped())
        .stdout(Stdio::piped())
        .spawn()
        .map_err(|e| format!("could not run sha256sum: {e}"))?;
    child
        .stdin
        .take()
        .ok_or("sha256sum stdin unavailable")?
        .write_all(bytes)
        .map_err(|e| format!("could not hash tool revision: {e}"))?;
    let output = child
        .wait_with_output()
        .map_err(|e| format!("could not finish tool revision hash: {e}"))?;
    if !output.status.success() {
        return Err("sha256sum failed for tool revision".into());
    }
    String::from_utf8(output.stdout)
        .map_err(|e| e.to_string())?
        .split_whitespace()
        .next()
        .map(str::to_string)
        .ok_or_else(|| "unexpected sha256sum output for tool revision".into())
}

fn escape(s: &str) -> String {
    s.replace('\\', "\\\\")
        .replace('"', "\\\"")
        .replace('\n', "\\n")
        .replace('\r', "\\r")
}

pub fn write_atomic(path: &Path, data: &[u8]) -> Result<(), String> {
    let temp = path.with_extension(format!(
        "{}.tmp",
        path.extension().and_then(|e| e.to_str()).unwrap_or("out")
    ));
    fs::write(&temp, data).map_err(|e| format!("{}: {e}", temp.display()))?;
    fs::rename(&temp, path).map_err(|e| format!("{} -> {}: {e}", temp.display(), path.display()))
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn json_escape_handles_quotes_and_newlines() {
        assert_eq!(escape("a\"b\nc"), "a\\\"b\\nc");
    }

    #[test]
    fn missing_hash_input_fails_closed() {
        let path =
            std::env::temp_dir().join(format!("missing-tempest-input-{}", std::process::id()));
        let _ = fs::remove_file(&path);
        assert!(
            build(&[path], 0, 0, 0, "")
                .unwrap_err()
                .contains("input missing")
        );
    }

    #[test]
    fn unchanged_inputs_have_deterministic_manifest_and_hashes() {
        let path = std::env::temp_dir().join(format!("tempest-hash-input-{}", std::process::id()));
        fs::write(&path, "stable input\n").unwrap();
        let input_hashes = hash_inputs(std::slice::from_ref(&path)).unwrap();
        let first = build(std::slice::from_ref(&path), 3, 4, 0, &input_hashes).unwrap();
        let second = build(std::slice::from_ref(&path), 3, 4, 0, &input_hashes).unwrap();
        assert_eq!(first.hash_lines, second.hash_lines);
        assert_eq!(first.json, second.json);
        assert!(build(std::slice::from_ref(&path), 3, 4, 0, "stale").is_err());
        fs::remove_file(path).unwrap();
    }
}
