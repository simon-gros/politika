use std::{collections::BTreeMap, fs, path::Path};

#[derive(Clone, Debug)]
pub struct Query {
    pub category: String,
    pub class: String,
    pub path: String,
    pub question: String,
}
#[derive(Clone, Debug)]
pub struct Record {
    pub side: String,
    pub source: String,
    pub class: String,
    pub path: String,
    pub name: String,
    pub category: String,
    pub question: String,
}

pub fn read_queries(path: &Path) -> Result<Vec<Query>, String> {
    let text = fs::read_to_string(path).map_err(|e| format!("{}: {e}", path.display()))?;
    let mut rows = Vec::new();
    for (i, line) in text.lines().enumerate() {
        if line.trim().is_empty() || line.starts_with('#') {
            continue;
        }
        let fields: Vec<_> = line.split('\t').collect();
        if fields.len() != 4 || fields.iter().any(|f| f.is_empty()) {
            return Err(format!(
                "{}:{}: expected four non-empty TSV fields",
                path.display(),
                i + 1
            ));
        }
        rows.push(Query {
            category: fields[0].into(),
            class: fields[1].into(),
            path: fields[2].into(),
            question: fields[3].into(),
        });
    }
    if rows.is_empty() {
        return Err(format!("{}: query allowlist is empty", path.display()));
    }
    rows.sort_by(|a, b| {
        a.category
            .cmp(&b.category)
            .then(a.class.cmp(&b.class))
            .then(a.path.cmp(&b.path))
    });
    Ok(rows)
}

pub fn read_registry(path: &Path, side: &str, queries: &[Query]) -> Result<Vec<Record>, String> {
    let text = fs::read_to_string(path).map_err(|e| format!("{}: {e}", path.display()))?;
    let source = path.to_string_lossy().into_owned();
    let mut rows = Vec::new();
    for (i, line) in text.lines().enumerate() {
        if line.is_empty() {
            continue;
        }
        let fields: Vec<_> = line.split('\t').collect();
        if fields.len() != 3 || fields.iter().any(|f| f.is_empty()) {
            return Err(format!(
                "{}:{}: expected exactly three non-empty TSV fields",
                path.display(),
                i + 1
            ));
        }
        if let Some(q) = queries
            .iter()
            .find(|q| fields[0].contains(&q.class) && fields[1].contains(&q.path))
        {
            rows.push(Record {
                side: side.into(),
                source: source.clone(),
                class: fields[0].into(),
                path: fields[1].into(),
                name: fields[2].into(),
                category: q.category.clone(),
                question: q.question.clone(),
            });
        }
    }
    rows.sort_by(|a, b| {
        a.side
            .cmp(&b.side)
            .then(a.class.cmp(&b.class))
            .then(a.path.cmp(&b.path))
            .then(a.name.cmp(&b.name))
    });
    Ok(rows)
}

pub fn duplicate_count(rows: &[Record]) -> usize {
    let mut seen = BTreeMap::new();
    let mut count = 0;
    for r in rows {
        let key = (&r.side, &r.class, &r.path, &r.name);
        if seen.insert(key, ()).is_some() {
            count += 1;
        }
    }
    count
}

pub fn csv(value: &str) -> String {
    if value.contains([',', '"', '\n', '\r']) {
        format!("\"{}\"", value.replace('"', "\"\""))
    } else {
        value.to_string()
    }
}

pub fn asset_csv(client: &[Record], server: &[Record]) -> String {
    let mut out = String::from(
        "side,source_record_path,asset_class,package_object_path,object_name,selected_category,evidence_kind,politika_question\n",
    );
    for r in client.iter().chain(server) {
        let vals = [
            &r.side,
            &r.source,
            &r.class,
            &r.path,
            &r.name,
            &r.category,
            &"registry-path-only".into(),
            &r.question,
        ];
        out.push_str(&vals.iter().map(|v| csv(v)).collect::<Vec<_>>().join(","));
        out.push('\n');
    }
    out
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn csv_quotes_commas_quotes_and_newlines() {
        assert_eq!(csv("a,\"b\n"), "\"a,\"\"b\n\"");
    }
    #[test]
    fn case_variant_paths_are_distinct() {
        let rows = ["/Game/Campaign/Veti", "/Game/Campaign/VETI"].map(|path| Record {
            side: "client".into(),
            source: "x".into(),
            class: "Map".into(),
            path: path.into(),
            name: "n".into(),
            category: "map".into(),
            question: "q".into(),
        });
        assert_eq!(duplicate_count(&rows), 0);
        assert_ne!(rows[0].path, rows[1].path);
    }
    #[test]
    fn duplicate_records_are_counted_not_dropped() {
        let r = Record {
            side: "client".into(),
            source: "x".into(),
            class: "Map".into(),
            path: "/Game/Maps/A".into(),
            name: "A".into(),
            category: "map".into(),
            question: "q".into(),
        };
        assert_eq!(duplicate_count(&[r.clone(), r]), 1);
    }

    #[test]
    fn malformed_registry_reports_file_and_line() {
        let path =
            std::env::temp_dir().join(format!("tempest-registry-{}.tsv", std::process::id()));
        fs::write(&path, "Map\t/Game/Maps/Okay\tOkay\nBad\tOnlyTwoFields\n").unwrap();
        let query = Query {
            category: "map".into(),
            class: "Map".into(),
            path: "/Game".into(),
            question: "q".into(),
        };
        let error = read_registry(&path, "client", &[query]).unwrap_err();
        assert!(error.contains(":2: expected exactly three"));
        fs::remove_file(path).unwrap();
    }

    #[test]
    fn malformed_query_is_rejected_with_line() {
        let path = std::env::temp_dir().join(format!("tempest-query-{}.tsv", std::process::id()));
        fs::write(&path, "category\tclass\tpath\n").unwrap();
        let error = read_queries(&path).unwrap_err();
        assert!(error.contains(":1: expected four"));
        fs::remove_file(path).unwrap();
    }
}
