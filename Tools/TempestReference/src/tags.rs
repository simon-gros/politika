use std::{
    fs,
    path::{Path, PathBuf},
};

#[derive(Clone, Debug)]
pub struct Tag {
    pub side: String,
    pub tag: String,
    pub source: String,
    pub line: usize,
}

pub fn ini_files(root: &Path) -> Result<Vec<PathBuf>, String> {
    if !root.is_dir() {
        return Err(format!("{}: config root is missing", root.display()));
    }
    fn walk(path: &Path, out: &mut Vec<PathBuf>) -> Result<(), String> {
        let entries = fs::read_dir(path).map_err(|e| format!("{}: {e}", path.display()))?;
        for entry in entries {
            let p = entry.map_err(|e| e.to_string())?.path();
            if p.is_dir() {
                walk(&p, out)?;
            } else if p.extension().is_some_and(|e| e.eq_ignore_ascii_case("ini"))
                && (p
                    .components()
                    .any(|c| c.as_os_str().eq_ignore_ascii_case("Tags"))
                    || p.file_name()
                        .is_some_and(|n| n.eq_ignore_ascii_case("DefaultGameplayTags.ini")))
            {
                out.push(p);
            }
        }
        Ok(())
    }
    let mut out = Vec::new();
    walk(root, &mut out)?;
    out.sort();
    if out.is_empty() {
        return Err(format!("{}: no tag INI inputs found", root.display()));
    }
    Ok(out)
}

pub fn read_tags(root: &Path, side: &str) -> Result<Vec<Tag>, String> {
    let mut out = Vec::new();
    for file in ini_files(root)? {
        let text = fs::read_to_string(&file).map_err(|e| format!("{}: {e}", file.display()))?;
        for (i, line) in text.lines().enumerate() {
            if !line.contains("GameplayTagList") {
                continue;
            }
            let Some(start) = line.find("Tag=\"") else {
                return Err(format!(
                    "{}:{}: malformed GameplayTagList entry: missing Tag=\" value",
                    file.display(),
                    i + 1
                ));
            };
            let value = &line[start + 5..];
            let Some(end) = value.find('"') else {
                return Err(format!(
                    "{}:{}: malformed GameplayTagList entry: unterminated tag",
                    file.display(),
                    i + 1
                ));
            };
            let tag = &value[..end];
            if tag.is_empty() {
                return Err(format!(
                    "{}:{}: malformed empty gameplay tag",
                    file.display(),
                    i + 1
                ));
            }
            out.push(Tag {
                side: side.into(),
                tag: tag.into(),
                source: file.to_string_lossy().into_owned(),
                line: i + 1,
            });
        }
    }
    out.sort_by(|a, b| {
        a.side
            .cmp(&b.side)
            .then(a.source.cmp(&b.source))
            .then(a.line.cmp(&b.line))
    });
    Ok(out)
}

pub fn tag_csv(client: &[Tag], server: &[Tag]) -> String {
    let mut out =
        String::from("side,tag,source_config_path,source_line,reviewer_disposition,rationale\n");
    for tag in client.iter().chain(server) {
        let fields = [
            &tag.side,
            &tag.tag,
            &tag.source,
            &tag.line.to_string(),
            &"unreviewed".to_string(),
            &"Discovery inventory only; semantics and adoption remain unreviewed.".to_string(),
        ];
        out.push_str(
            &fields
                .iter()
                .map(|f| crate::registry::csv(f))
                .collect::<Vec<_>>()
                .join(","),
        );
        out.push('\n');
    }
    out
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn parses_only_explicit_tag_list_rows_and_preserves_case() {
        let dir = std::env::temp_dir().join(format!("tempest-tags-{}", std::process::id()));
        let tags = dir.join("Tags");
        fs::create_dir_all(&tags).unwrap();
        let file = tags.join("test.ini");
        fs::write(
            &file,
            "[/Script/X]\nGameplayTagList=(Tag=\"Definition.Unit.MixedCase\",DevComment=\"\")\n",
        )
        .unwrap();
        let got = read_tags(&dir, "client").unwrap();
        assert_eq!(got[0].tag, "Definition.Unit.MixedCase");
        fs::remove_dir_all(dir).unwrap();
    }

    #[test]
    fn preserves_unusual_but_structurally_quoted_tags_for_review() {
        let dir = std::env::temp_dir().join(format!("tempest-tags-unusual-{}", std::process::id()));
        let tags = dir.join("Tags");
        fs::create_dir_all(&tags).unwrap();
        fs::write(
            tags.join("test.ini"),
            "GameplayTagList=(Tag=\"Definition.CIV.Unit..Unarmed\",DevComment=\"\")\n",
        )
        .unwrap();
        let got = read_tags(&dir, "client").unwrap();
        assert_eq!(got[0].tag, "Definition.CIV.Unit..Unarmed");
        fs::remove_dir_all(dir).unwrap();
    }

    #[test]
    fn malformed_tag_entry_fails_with_line_context() {
        let dir = std::env::temp_dir().join(format!("tempest-tags-bad-{}", std::process::id()));
        let tags = dir.join("Tags");
        fs::create_dir_all(&tags).unwrap();
        fs::write(
            tags.join("test.ini"),
            "GameplayTagList=(Tag=\"unterminated)\n",
        )
        .unwrap();
        let error = read_tags(&dir, "client").unwrap_err();
        assert!(error.contains(":1: malformed GameplayTagList"));
        fs::remove_dir_all(dir).unwrap();
    }
}
