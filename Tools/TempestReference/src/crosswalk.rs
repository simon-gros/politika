use std::{
    collections::{BTreeMap, BTreeSet},
    fs,
    path::{Path, PathBuf},
};

const DISPOSITIONS: &[&str] = &["adopt-concept", "redefine", "defer", "exclude"];
const CONFIDENCE: &[&str] = &["documented", "observed", "registry-only", "unknown"];

#[derive(Default)]
struct CatalogRefs {
    assets: BTreeMap<String, String>,
    tags: BTreeMap<String, String>,
}

pub fn run(args: &[std::ffi::OsString]) -> Result<(), String> {
    let mut values = BTreeMap::new();
    let mut i = 0;
    while i < args.len() {
        let key = args[i]
            .to_str()
            .ok_or("crosswalk option is not valid UTF-8")?;
        if !key.starts_with("--") {
            return Err(format!("Unexpected argument: {key}"));
        }
        if ![
            "--asset-index",
            "--tag-index",
            "--manifest",
            "--categories",
            "--relationships",
        ]
        .contains(&key)
        {
            return Err(format!("Unknown crosswalk option: {key}"));
        }
        let value = args
            .get(i + 1)
            .ok_or_else(|| format!("Missing value for {key}"))?
            .to_string_lossy()
            .into_owned();
        if values
            .insert(key.to_string(), PathBuf::from(value))
            .is_some()
        {
            return Err(format!("Duplicate option: {key}"));
        }
        i += 2;
    }
    let required = |key: &str| {
        values
            .get(key)
            .cloned()
            .ok_or_else(|| format!("Missing required {key}"))
    };
    let asset_index = required("--asset-index")?;
    let tag_index = required("--tag-index")?;
    let manifest = required("--manifest")?;
    let categories = required("--categories")?;
    let relationships = required("--relationships")?;

    let asset_hash = crate::manifest::hash_file(&asset_index)?;
    let tag_hash = crate::manifest::hash_file(&tag_index)?;
    let reference_hash = crate::manifest::hash_file(&manifest)?;
    let refs = catalog_refs(&asset_index, &tag_index)?;
    let category_rows = parse_file(&categories)?;
    let relationship_rows = parse_file(&relationships)?;
    let category_count = validate_categories(
        &category_rows,
        &refs,
        &reference_hash,
        &asset_hash,
        &tag_hash,
    )?;
    let relationship_count = validate_relationships(
        &relationship_rows,
        &refs,
        &reference_hash,
        &asset_hash,
        &tag_hash,
    )?;
    println!(
        "Crosswalk valid: {category_count} category rows, {relationship_count} relationship rows; reference manifest {reference_hash}."
    );
    Ok(())
}

fn catalog_refs(asset_index: &PathBuf, tag_index: &PathBuf) -> Result<CatalogRefs, String> {
    let mut refs = CatalogRefs::default();
    let assets = parse_file(asset_index)?;
    let asset_columns = columns(&assets[0])?;
    let side = column(&asset_columns, "side")?;
    let path = column(&asset_columns, "package_object_path")?;
    let class = column(&asset_columns, "asset_class")?;
    for (line, row) in assets.iter().enumerate().skip(1) {
        let side = value(row, side, asset_index, line + 1)?;
        let path = value(row, path, asset_index, line + 1)?;
        let class = value(row, class, asset_index, line + 1)?;
        refs.assets
            .insert(asset_ref(side, class, path), class.to_string());
    }
    let tags = parse_file(tag_index)?;
    let tag_columns = columns(&tags[0])?;
    let side = column(&tag_columns, "side")?;
    let tag = column(&tag_columns, "tag")?;
    let path = column(&tag_columns, "source_config_path")?;
    for (line, row) in tags.iter().enumerate().skip(1) {
        let side = value(row, side, tag_index, line + 1)?;
        let path = value(row, path, tag_index, line + 1)?;
        let tag = value(row, tag, tag_index, line + 1)?;
        refs.tags.insert(tag_ref(side, path, tag), tag.to_string());
    }
    Ok(refs)
}

fn validate_categories(
    rows: &[Vec<String>],
    refs: &CatalogRefs,
    reference_hash: &str,
    asset_hash: &str,
    tag_hash: &str,
) -> Result<usize, String> {
    let header = rows.first().ok_or("category crosswalk is empty")?;
    let cols = columns(header)?;
    let required = [
        "source_ref",
        "source_label",
        "category_family",
        "politika_name",
        "disposition",
        "evidence_type",
        "project_owner",
        "design_gap",
        "rationale",
        "reference_manifest_sha256",
        "asset_index_sha256",
        "tag_index_sha256",
    ];
    for name in required {
        column(&cols, name)?;
    }
    let mut seen = BTreeSet::new();
    for (line, row) in rows.iter().enumerate().skip(1) {
        let source_ref = field(row, &cols, "source_ref", "category", line + 1)?;
        let source_label = field(row, &cols, "source_label", "category", line + 1)?;
        let family = field(row, &cols, "category_family", "category", line + 1)?;
        let name = field(row, &cols, "politika_name", "category", line + 1)?;
        let disposition = field(row, &cols, "disposition", "category", line + 1)?;
        let evidence = field(row, &cols, "evidence_type", "category", line + 1)?;
        let owner = field(row, &cols, "project_owner", "category", line + 1)?;
        let gap = field(row, &cols, "design_gap", "category", line + 1)?;
        let rationale = field(row, &cols, "rationale", "category", line + 1)?;
        check_identity(row, &cols, line + 1, reference_hash, asset_hash, tag_hash)?;
        ensure_ref(source_ref, refs).map_err(|e| format!("category row {}: {e}", line + 1))?;
        if known_label(source_ref, refs) != Some(source_label) {
            return Err(format!(
                "category row {}: source_label does not match the referenced inventory entry",
                line + 1
            ));
        }
        if !DISPOSITIONS.contains(&disposition) {
            return Err(format!(
                "category row {}: invalid disposition {disposition:?}",
                line + 1
            ));
        }
        if family.is_empty() || evidence.is_empty() || rationale.trim().is_empty() {
            return Err(format!(
                "category row {}: category, evidence and rationale are required",
                line + 1
            ));
        }
        if matches!(disposition, "adopt-concept" | "redefine") {
            if !valid_politika_name(name) {
                return Err(format!(
                    "category row {}: invalid POLITIKA name/ID {name:?}",
                    line + 1
                ));
            }
            if owner.trim().is_empty() || gap.trim().is_empty() {
                return Err(format!(
                    "category row {}: retained concept requires project owner and concrete design gap",
                    line + 1
                ));
            }
            validate_owner(owner).map_err(|e| format!("category row {}: {e}", line + 1))?;
        }
        let key = format!("{source_ref}\t{family}");
        if !seen.insert(key) {
            return Err(format!(
                "category row {}: duplicate source/category row",
                line + 1
            ));
        }
    }
    Ok(rows.len().saturating_sub(1))
}

fn validate_relationships(
    rows: &[Vec<String>],
    refs: &CatalogRefs,
    reference_hash: &str,
    asset_hash: &str,
    tag_hash: &str,
) -> Result<usize, String> {
    let header = rows.first().ok_or("relationship crosswalk is empty")?;
    let cols = columns(header)?;
    for name in [
        "source_family_a",
        "source_family_b",
        "evidence_refs",
        "relation",
        "evidence_type",
        "confidence",
        "disposition",
        "project_owner",
        "design_gap",
        "limitation",
        "reference_manifest_sha256",
        "asset_index_sha256",
        "tag_index_sha256",
    ] {
        column(&cols, name)?;
    }
    let mut seen = BTreeSet::new();
    for (line, row) in rows.iter().enumerate().skip(1) {
        let a = field(row, &cols, "source_family_a", "relationship", line + 1)?;
        let b = field(row, &cols, "source_family_b", "relationship", line + 1)?;
        let evidence_refs = field(row, &cols, "evidence_refs", "relationship", line + 1)?;
        let relation = field(row, &cols, "relation", "relationship", line + 1)?;
        let evidence = field(row, &cols, "evidence_type", "relationship", line + 1)?;
        let confidence = field(row, &cols, "confidence", "relationship", line + 1)?;
        let disposition = field(row, &cols, "disposition", "relationship", line + 1)?;
        let owner = field(row, &cols, "project_owner", "relationship", line + 1)?;
        let gap = field(row, &cols, "design_gap", "relationship", line + 1)?;
        let limitation = field(row, &cols, "limitation", "relationship", line + 1)?;
        check_identity(row, &cols, line + 1, reference_hash, asset_hash, tag_hash)?;
        if a.trim().is_empty()
            || b.trim().is_empty()
            || relation.trim().is_empty()
            || evidence.trim().is_empty()
        {
            return Err(format!(
                "relationship row {}: family names, relation, and evidence are required",
                line + 1
            ));
        }
        if !CONFIDENCE.contains(&confidence) {
            return Err(format!(
                "relationship row {}: invalid confidence {confidence:?}",
                line + 1
            ));
        }
        if !DISPOSITIONS.contains(&disposition) {
            return Err(format!(
                "relationship row {}: invalid disposition {disposition:?}",
                line + 1
            ));
        }
        if limitation.trim().is_empty() {
            return Err(format!(
                "relationship row {}: limitation is required",
                line + 1
            ));
        }
        if matches!(disposition, "adopt-concept" | "redefine")
            && (owner.trim().is_empty() || gap.trim().is_empty())
        {
            return Err(format!(
                "relationship row {}: retained concept requires project owner and concrete design gap",
                line + 1
            ));
        }
        if matches!(disposition, "adopt-concept" | "redefine") {
            validate_owner(owner).map_err(|e| format!("relationship row {}: {e}", line + 1))?;
        }
        let parsed_refs: Vec<_> = evidence_refs.split(';').collect();
        if parsed_refs.is_empty() || parsed_refs.iter().any(|r| r.trim().is_empty()) {
            return Err(format!(
                "relationship row {}: evidence_refs must contain exact source references",
                line + 1
            ));
        }
        for reference in &parsed_refs {
            ensure_ref(reference, refs)
                .map_err(|e| format!("relationship row {}: {e}", line + 1))?;
        }
        let key = format!("{}\t{}", parsed_refs.join(";"), relation);
        if !seen.insert(key) {
            return Err(format!(
                "relationship row {}: duplicate relationship row",
                line + 1
            ));
        }
    }
    Ok(rows.len().saturating_sub(1))
}

fn check_identity(
    row: &[String],
    cols: &BTreeMap<String, usize>,
    line: usize,
    reference_hash: &str,
    asset_hash: &str,
    tag_hash: &str,
) -> Result<(), String> {
    for (name, expected) in [
        ("reference_manifest_sha256", reference_hash),
        ("asset_index_sha256", asset_hash),
        ("tag_index_sha256", tag_hash),
    ] {
        let actual = field(row, cols, name, "crosswalk", line)?;
        if actual != expected {
            return Err(format!(
                "crosswalk row {line}: stale {name}; expected {expected}, found {actual}"
            ));
        }
    }
    Ok(())
}

fn ensure_ref(reference: &str, refs: &CatalogRefs) -> Result<(), String> {
    let parts: Vec<_> = reference.splitn(4, '|').collect();
    match parts.as_slice() {
        ["asset", side, class, path] if refs.assets.contains_key(&asset_ref(side, class, path)) => {
            Ok(())
        }
        ["tag", side, path, tag] if refs.tags.contains_key(&tag_ref(side, path, tag)) => Ok(()),
        ["asset", ..] => Err(format!(
            "unknown or case-mismatched asset reference {reference:?}"
        )),
        ["tag", _, _, _] => Err(format!(
            "unknown or case-mismatched tag reference {reference:?}"
        )),
        _ => Err(format!(
            "malformed source reference {reference:?}; expected asset|side|path or tag|side|path|tag"
        )),
    }
}

fn known_label<'a>(reference: &str, refs: &'a CatalogRefs) -> Option<&'a str> {
    match reference.splitn(4, '|').collect::<Vec<_>>().as_slice() {
        ["asset", side, class, path] => refs
            .assets
            .get(&asset_ref(side, class, path))
            .map(String::as_str),
        ["tag", side, path, tag] => refs.tags.get(&tag_ref(side, path, tag)).map(String::as_str),
        _ => None,
    }
}

fn asset_ref(side: &str, class: &str, path: &str) -> String {
    format!("asset|{side}|{class}|{path}")
}
fn tag_ref(side: &str, path: &str, tag: &str) -> String {
    format!("tag|{side}|{path}|{tag}")
}

fn valid_politika_name(value: &str) -> bool {
    if value.is_empty() {
        return false;
    }
    value.split('.').all(|segment| {
        !segment.is_empty()
            && segment
                .bytes()
                .all(|b| b.is_ascii_lowercase() || b.is_ascii_digit() || b == b'_')
    })
}

fn validate_owner(owner: &str) -> Result<(), String> {
    let document = owner.split_whitespace().next().unwrap_or_default();
    if document.is_empty() {
        return Err("project owner document is empty".into());
    }
    let root = Path::new(env!("CARGO_MANIFEST_DIR")).join("../..");
    if !root.join(document).is_file() {
        return Err(format!("project owner document does not exist: {document}"));
    }
    Ok(())
}

fn parse_file(path: &PathBuf) -> Result<Vec<Vec<String>>, String> {
    let text = fs::read_to_string(path).map_err(|e| format!("{}: {e}", path.display()))?;
    let rows = parse_csv(&text).map_err(|e| format!("{}: {e}", path.display()))?;
    if rows.is_empty() {
        return Err(format!("{}: CSV has no header", path.display()));
    }
    let width = rows[0].len();
    if width == 0 || rows.iter().any(|r| r.len() != width) {
        return Err(format!("{}: inconsistent CSV column count", path.display()));
    }
    Ok(rows)
}

fn parse_csv(text: &str) -> Result<Vec<Vec<String>>, String> {
    let mut rows = Vec::new();
    let mut row = Vec::new();
    let mut field = String::new();
    let mut chars = text.chars().peekable();
    let mut quoted = false;
    let mut closed_quote = false;
    while let Some(ch) = chars.next() {
        if quoted {
            if ch == '"' {
                if chars.peek() == Some(&'"') {
                    chars.next();
                    field.push('"');
                } else {
                    quoted = false;
                    closed_quote = true;
                }
            } else {
                field.push(ch);
            }
            continue;
        }
        if closed_quote && ch != ',' && ch != '\n' && ch != '\r' {
            return Err("unexpected character after closing quote".into());
        }
        match ch {
            '"' if field.is_empty() && !closed_quote => quoted = true,
            '"' => return Err("quote inside unquoted field".into()),
            ',' => {
                row.push(std::mem::take(&mut field));
                closed_quote = false;
            }
            '\n' => {
                row.push(std::mem::take(&mut field));
                rows.push(std::mem::take(&mut row));
                closed_quote = false;
            }
            '\r' if chars.peek() == Some(&'\n') => {
                chars.next();
                row.push(std::mem::take(&mut field));
                rows.push(std::mem::take(&mut row));
                closed_quote = false;
            }
            '\r' => return Err("bare carriage return outside quoted field".into()),
            _ => field.push(ch),
        }
    }
    if quoted {
        return Err("unterminated quoted field".into());
    }
    if !field.is_empty() || !row.is_empty() || closed_quote {
        row.push(field);
        rows.push(row);
    }
    Ok(rows)
}

fn columns(header: &[String]) -> Result<BTreeMap<String, usize>, String> {
    let mut out = BTreeMap::new();
    for (i, name) in header.iter().enumerate() {
        if name.is_empty() || out.insert(name.clone(), i).is_some() {
            return Err(format!("empty or duplicate CSV column {name:?}"));
        }
    }
    Ok(out)
}

fn column(cols: &BTreeMap<String, usize>, name: &str) -> Result<usize, String> {
    cols.get(name)
        .copied()
        .ok_or_else(|| format!("missing CSV column {name:?}"))
}

fn value<'a>(
    row: &'a [String],
    index: usize,
    path: &PathBuf,
    line: usize,
) -> Result<&'a str, String> {
    row.get(index)
        .map(String::as_str)
        .ok_or_else(|| format!("{}:{}: field missing", path.display(), line))
}

fn field<'a>(
    row: &'a [String],
    cols: &BTreeMap<String, usize>,
    name: &str,
    kind: &str,
    line: usize,
) -> Result<&'a str, String> {
    let index = column(cols, name).map_err(|e| format!("{kind} header: {e}"))?;
    row.get(index)
        .map(String::as_str)
        .ok_or_else(|| format!("{kind} row {line}: missing field {name}"))
}

#[cfg(test)]
mod tests {
    use super::*;

    const CATEGORY_HEADER: [&str; 12] = [
        "source_ref",
        "source_label",
        "category_family",
        "politika_name",
        "disposition",
        "evidence_type",
        "project_owner",
        "design_gap",
        "rationale",
        "reference_manifest_sha256",
        "asset_index_sha256",
        "tag_index_sha256",
    ];
    const REL_HEADER: [&str; 13] = [
        "source_family_a",
        "source_family_b",
        "evidence_refs",
        "relation",
        "evidence_type",
        "confidence",
        "disposition",
        "project_owner",
        "design_gap",
        "limitation",
        "reference_manifest_sha256",
        "asset_index_sha256",
        "tag_index_sha256",
    ];
    fn refs() -> CatalogRefs {
        CatalogRefs {
            assets: [
                (
                    asset_ref("client", "TedUnitDefinitionAsset", "/Game/A"),
                    "TedUnitDefinitionAsset".into(),
                ),
                (
                    asset_ref("client", "TedUnitDefinitionAsset", "/Game/a"),
                    "TedUnitDefinitionAsset".into(),
                ),
                (
                    asset_ref("client", "TedWeaponDefinitionAsset", "/Game/B"),
                    "TedWeaponDefinitionAsset".into(),
                ),
            ]
            .into_iter()
            .collect(),
            tags: [(
                tag_ref("client", "/Config/CommandTags.ini", "CommandType.Move"),
                "CommandType.Move".into(),
            )]
            .into_iter()
            .collect(),
        }
    }
    fn category(
        source: &str,
        disposition: &str,
        name: &str,
        owner: &str,
        gap: &str,
        rationale: &str,
        manifest: &str,
    ) -> Vec<String> {
        [
            source,
            "TedUnitDefinitionAsset",
            "unit-definition",
            name,
            disposition,
            "registry-path-only",
            owner,
            gap,
            rationale,
            manifest,
            "asset-hash",
            "tag-hash",
        ]
        .map(str::to_string)
        .to_vec()
    }

    #[test]
    fn csv_parser_round_trips_quotes_commas_and_embedded_newlines() {
        let parsed =
            parse_csv("a,b\n\"quoted, value\",\"line one\nline two with \"\"quote\"\"\"\n")
                .unwrap();
        assert_eq!(
            parsed[1],
            ["quoted, value", "line one\nline two with \"quote\""]
        );
    }
    #[test]
    fn politika_names_follow_project_stable_id_shape() {
        for valid in [
            "unit.recon_vehicle",
            "map.prototype.rts_vertical_slice",
            "weapon_2.main",
        ] {
            assert!(valid_politika_name(valid));
        }
        for invalid in [
            "Unit.Recon",
            "unit..vehicle",
            "unit.recon-vehicle",
            "tempest:unit",
        ] {
            assert!(!valid_politika_name(invalid));
        }
    }
    #[test]
    fn case_variant_asset_paths_are_exact_and_distinct() {
        let refs = refs();
        assert!(ensure_ref("asset|client|TedUnitDefinitionAsset|/Game/A", &refs).is_ok());
        assert!(ensure_ref("asset|client|TedUnitDefinitionAsset|/Game/a", &refs).is_ok());
        assert!(ensure_ref("asset|client|TedUnitDefinitionAsset|/game/a", &refs).is_err());
    }
    #[test]
    fn category_rejects_invalid_disposition_unknown_path_empty_rationale_and_stale_manifest() {
        let refs = refs();
        let header = CATEGORY_HEADER.map(str::to_string).to_vec();
        let row = category(
            "asset|client|TedUnitDefinitionAsset|/Game/A",
            "defer",
            "",
            "",
            "",
            "kept for review",
            "manifest-hash",
        );
        assert!(
            validate_categories(
                &[header.clone(), row],
                &refs,
                "manifest-hash",
                "asset-hash",
                "tag-hash"
            )
            .is_ok()
        );
        let mut wrong_label = category(
            "asset|client|TedUnitDefinitionAsset|/Game/A",
            "defer",
            "",
            "",
            "",
            "reason",
            "manifest-hash",
        );
        wrong_label[1] = "TedWeaponDefinitionAsset".into();
        assert!(
            validate_categories(
                &[header.clone(), wrong_label],
                &refs,
                "manifest-hash",
                "asset-hash",
                "tag-hash"
            )
            .unwrap_err()
            .contains("source_label")
        );
        let mut bad = category(
            "asset|client|TedUnitDefinitionAsset|/Game/A",
            "accept",
            "",
            "",
            "",
            "reason",
            "manifest-hash",
        );
        assert!(
            validate_categories(
                &[header.clone(), bad.clone()],
                &refs,
                "manifest-hash",
                "asset-hash",
                "tag-hash"
            )
            .unwrap_err()
            .contains("invalid disposition")
        );
        bad = category(
            "asset|client|TedUnitDefinitionAsset|/GAME/A",
            "defer",
            "",
            "",
            "",
            "reason",
            "manifest-hash",
        );
        assert!(
            validate_categories(
                &[header.clone(), bad],
                &refs,
                "manifest-hash",
                "asset-hash",
                "tag-hash"
            )
            .unwrap_err()
            .contains("unknown or case-mismatched")
        );
        bad = category(
            "asset|client|TedUnitDefinitionAsset|/Game/A",
            "defer",
            "",
            "",
            "",
            " ",
            "manifest-hash",
        );
        assert!(
            validate_categories(
                &[header.clone(), bad],
                &refs,
                "manifest-hash",
                "asset-hash",
                "tag-hash"
            )
            .unwrap_err()
            .contains("rationale")
        );
        bad = category(
            "asset|client|TedUnitDefinitionAsset|/Game/A",
            "defer",
            "",
            "",
            "",
            "reason",
            "stale",
        );
        assert!(
            validate_categories(
                &[header.clone(), bad],
                &refs,
                "manifest-hash",
                "asset-hash",
                "tag-hash"
            )
            .unwrap_err()
            .contains("stale reference_manifest_sha256")
        );
    }
    #[test]
    fn category_rejects_duplicate_rows_and_requires_owner_gap_for_retained_items() {
        let refs = refs();
        let header = CATEGORY_HEADER.map(str::to_string).to_vec();
        let row = category(
            "asset|client|TedUnitDefinitionAsset|/Game/A",
            "redefine",
            "unit.archetype",
            "Docs/GAMEPLAY_CONTENT.md",
            "named scenario requires role-specific capability",
            "POLITIKA schema remains authoritative",
            "manifest-hash",
        );
        assert!(
            validate_categories(
                &[header.clone(), row.clone()],
                &refs,
                "manifest-hash",
                "asset-hash",
                "tag-hash"
            )
            .is_ok()
        );
        assert!(
            validate_categories(
                &[header.clone(), row.clone(), row],
                &refs,
                "manifest-hash",
                "asset-hash",
                "tag-hash"
            )
            .unwrap_err()
            .contains("duplicate")
        );
        let no_owner = category(
            "asset|client|TedUnitDefinitionAsset|/Game/A",
            "adopt-concept",
            "unit.archetype",
            "",
            "",
            "reason",
            "manifest-hash",
        );
        assert!(
            validate_categories(
                &[header, no_owner],
                &refs,
                "manifest-hash",
                "asset-hash",
                "tag-hash"
            )
            .unwrap_err()
            .contains("owner and concrete design gap")
        );
        let header = CATEGORY_HEADER.map(str::to_string).to_vec();
        let bad_owner = category(
            "asset|client|TedUnitDefinitionAsset|/Game/A",
            "redefine",
            "unit.archetype",
            "Docs/DOES_NOT_EXIST.md",
            "feature gap",
            "reason",
            "manifest-hash",
        );
        assert!(
            validate_categories(
                &[header, bad_owner],
                &refs,
                "manifest-hash",
                "asset-hash",
                "tag-hash"
            )
            .unwrap_err()
            .contains("owner document does not exist")
        );
    }

    #[test]
    fn category_rejects_unknown_asset_and_tag_refs_with_case_sensitive_identity() {
        let refs = refs();
        let header = CATEGORY_HEADER.map(str::to_string).to_vec();
        let valid = category(
            "asset|client|TedUnitDefinitionAsset|/Game/A",
            "defer",
            "",
            "",
            "",
            "research lead only",
            "manifest-hash",
        );
        for (reference, label) in [
            (
                "asset|client|TedUnitDefinitionAsset|/Game/Missing",
                "TedUnitDefinitionAsset",
            ),
            (
                "asset|client|TedUnitDefinitionAsset|/game/a",
                "TedUnitDefinitionAsset",
            ),
            (
                "tag|client|/Config/CommandTags.ini|CommandType.Missing",
                "CommandType.Missing",
            ),
            (
                "tag|client|/config/CommandTags.ini|CommandType.Move",
                "CommandType.Move",
            ),
            (
                "tag|client|/Config/CommandTags.ini|commandtype.Move",
                "commandtype.Move",
            ),
        ] {
            let mut row = valid.clone();
            row[0] = reference.into();
            row[1] = label.into();
            assert!(
                validate_categories(
                    &[header.clone(), row],
                    &refs,
                    "manifest-hash",
                    "asset-hash",
                    "tag-hash"
                )
                .unwrap_err()
                .contains("unknown or case-mismatched")
            );
        }
    }

    #[test]
    fn category_rejects_invalid_retained_identifier_and_each_stale_fingerprint() {
        let refs = refs();
        let header = CATEGORY_HEADER.map(str::to_string).to_vec();
        let valid = category(
            "asset|client|TedUnitDefinitionAsset|/Game/A",
            "redefine",
            "unit.archetype",
            "Docs/GAMEPLAY_CONTENT.md",
            "named role taxonomy is not yet authored",
            "POLITIKA definitions remain authoritative",
            "manifest-hash",
        );
        let mut bad_name = valid.clone();
        bad_name[3] = "TedUnitDefinitionAsset".into();
        assert!(
            validate_categories(
                &[header.clone(), bad_name],
                &refs,
                "manifest-hash",
                "asset-hash",
                "tag-hash"
            )
            .unwrap_err()
            .contains("invalid POLITIKA name/ID")
        );
        for (index, field) in [
            (9, "reference_manifest_sha256"),
            (10, "asset_index_sha256"),
            (11, "tag_index_sha256"),
        ] {
            let mut stale = valid.clone();
            stale[index] = "stale".into();
            assert!(
                validate_categories(
                    &[header.clone(), stale],
                    &refs,
                    "manifest-hash",
                    "asset-hash",
                    "tag-hash"
                )
                .unwrap_err()
                .contains(&format!("stale {field}"))
            );
        }
    }

    #[test]
    fn relationship_rejects_unknown_refs_and_allows_exact_tag_refs() {
        let refs = refs();
        let header = REL_HEADER.map(str::to_string).to_vec();
        let row = vec![
            "CommandType".into(),
            "typed command admission".into(),
            "tag|client|/Config/CommandTags.ini|CommandType.Move".into(),
            "Taxonomy and eligibility relation not established".into(),
            "tag-inventory-only".into(),
            "unknown".into(),
            "defer".into(),
            "".into(),
            "".into(),
            "Tag presence does not establish an admission rule.".into(),
            "manifest-hash".into(),
            "asset-hash".into(),
            "tag-hash".into(),
        ];
        assert_eq!(
            validate_relationships(
                &[header.clone(), row.clone()],
                &refs,
                "manifest-hash",
                "asset-hash",
                "tag-hash"
            )
            .unwrap(),
            1
        );
        let mut stale = row;
        stale[2] = "asset|client|TedUnitDefinitionAsset|/Game/Missing".into();
        assert!(
            validate_relationships(
                &[header, stale],
                &refs,
                "manifest-hash",
                "asset-hash",
                "tag-hash"
            )
            .unwrap_err()
            .contains("unknown or case-mismatched")
        );
    }

    #[test]
    fn relationship_rejects_bad_rows_duplicates_and_each_stale_fingerprint() {
        let refs = refs();
        let header = REL_HEADER.map(str::to_string).to_vec();
        let valid = vec![
            "CommandType".into(),
            "typed command admission".into(),
            "tag|client|/Config/CommandTags.ini|CommandType.Move".into(),
            "Tag presence does not establish admission semantics.".into(),
            "tag-inventory-only".into(),
            "unknown".into(),
            "exclude".into(),
            "".into(),
            "".into(),
            "POLITIKA's coordinator remains authoritative.".into(),
            "manifest-hash".into(),
            "asset-hash".into(),
            "tag-hash".into(),
        ];
        let validate = |rows: &[Vec<String>]| {
            validate_relationships(rows, &refs, "manifest-hash", "asset-hash", "tag-hash")
        };
        assert_eq!(validate(&[header.clone(), valid.clone()]).unwrap(), 1);
        assert!(
            validate(&[header.clone(), valid.clone(), valid.clone()])
                .unwrap_err()
                .contains("duplicate")
        );

        let mut missing_owner = valid.clone();
        missing_owner[6] = "redefine".into();
        assert!(
            validate(&[header.clone(), missing_owner])
                .unwrap_err()
                .contains("retained concept requires project owner and concrete design gap")
        );

        let mut nonexistent_owner = valid.clone();
        nonexistent_owner[6] = "redefine".into();
        nonexistent_owner[7] = "Docs/DOES_NOT_EXIST.md".into();
        nonexistent_owner[8] = "named design gap".into();
        assert!(
            validate(&[header.clone(), nonexistent_owner])
                .unwrap_err()
                .contains("owner document does not exist")
        );

        for (index, value, expected) in [
            (
                2,
                "tag|client|/Config/CommandTags.ini|Missing",
                "unknown or case-mismatched",
            ),
            (
                2,
                "tag|client|/config/CommandTags.ini|CommandType.Move",
                "unknown or case-mismatched",
            ),
            (2, "", "exact source references"),
            (5, "unsupported", "invalid confidence"),
            (6, "accept", "invalid disposition"),
            (9, " ", "limitation is required"),
        ] {
            let mut bad = valid.clone();
            bad[index] = value.into();
            assert!(
                validate(&[header.clone(), bad])
                    .unwrap_err()
                    .contains(expected)
            );
        }
        for (index, field) in [
            (10, "reference_manifest_sha256"),
            (11, "asset_index_sha256"),
            (12, "tag_index_sha256"),
        ] {
            let mut stale = valid.clone();
            stale[index] = "stale".into();
            assert!(
                validate(&[header.clone(), stale])
                    .unwrap_err()
                    .contains(&format!("stale {field}"))
            );
        }
    }
}
