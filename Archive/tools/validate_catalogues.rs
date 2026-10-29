//! Validate POLITIKA's historical archive CSV schemas and reference indexes.
//!
//! Run from any working directory:
//! cargo run --locked --quiet --manifest-path Archive/tools/Cargo.toml
//!
//! This checks local metadata consistency, not historical accuracy, source
//! availability, dates, copyright, or media licensing. No third-party crates.

use std::collections::BTreeSet;
use std::fs;
use std::path::{Path, PathBuf};
use std::process::ExitCode;

#[derive(Default)]
struct Csv {
    headers: Vec<String>,
    rows: Vec<CsvRow>,
}

struct CsvRow {
    line: usize,
    values: Vec<String>,
}

#[derive(Clone, Copy, PartialEq, Eq)]
enum FieldState {
    Start,
    Unquoted,
    Quoted,
    AfterQuote,
}

fn end_row(
    rows: &mut Vec<(usize, Vec<String>)>,
    cells: &mut Vec<String>,
    cell: &mut String,
    row_start: usize,
) {
    cells.push(std::mem::take(cell));
    rows.push((row_start, std::mem::take(cells)));
}

// RFC 4180-style quoted fields, escaped quotes, CRLF, embedded newlines, and
// UTF-8 BOM. Blank physical lines are ignored, like Python's csv.reader.
// Deliberately reject invalid characters after a closing quote.
fn parse_csv(source: &str) -> Result<Csv, String> {
    let source = source.strip_prefix('\u{feff}').unwrap_or(source);
    let mut chars = source.chars().peekable();
    let mut records = Vec::new();
    let mut cells = Vec::new();
    let mut cell = String::new();
    let mut state = FieldState::Start;
    let (mut line, mut row_start) = (1usize, 1usize);
    let mut active = false;

    while let Some(ch) = chars.next() {
        match state {
            FieldState::Start => match ch {
                ',' => {
                    active = true;
                    cells.push(std::mem::take(&mut cell));
                }
                '"' => {
                    active = true;
                    state = FieldState::Quoted;
                }
                '\r' | '\n' => {
                    if ch == '\r' && chars.peek() == Some(&'\n') {
                        chars.next();
                    }
                    if active {
                        end_row(&mut records, &mut cells, &mut cell, row_start);
                    }
                    active = false;
                    line += 1;
                    row_start = line;
                }
                _ => {
                    active = true;
                    cell.push(ch);
                    state = FieldState::Unquoted;
                }
            },
            FieldState::Unquoted => match ch {
                ',' => {
                    cells.push(std::mem::take(&mut cell));
                    state = FieldState::Start;
                }
                '\r' | '\n' => {
                    if ch == '\r' && chars.peek() == Some(&'\n') {
                        chars.next();
                    }
                    end_row(&mut records, &mut cells, &mut cell, row_start);
                    state = FieldState::Start;
                    active = false;
                    line += 1;
                    row_start = line;
                }
                // Python's default csv.reader is lenient about a quote in an
                // unquoted field; preserve it rather than treating it as syntax.
                _ => cell.push(ch),
            },
            FieldState::Quoted => match ch {
                '"' => state = FieldState::AfterQuote,
                '\r' => {
                    cell.push(ch);
                    if chars.peek() == Some(&'\n') {
                        cell.push(chars.next().expect("peeked LF"));
                    }
                    line += 1;
                }
                '\n' => {
                    cell.push(ch);
                    line += 1;
                }
                _ => cell.push(ch),
            },
            FieldState::AfterQuote => match ch {
                '"' => {
                    cell.push('"');
                    state = FieldState::Quoted;
                }
                ',' => {
                    cells.push(std::mem::take(&mut cell));
                    state = FieldState::Start;
                }
                '\r' | '\n' => {
                    if ch == '\r' && chars.peek() == Some(&'\n') {
                        chars.next();
                    }
                    end_row(&mut records, &mut cells, &mut cell, row_start);
                    state = FieldState::Start;
                    active = false;
                    line += 1;
                    row_start = line;
                }
                _ => return Err(format!("line {line}: unexpected character after CSV quote")),
            },
        }
    }

    if state == FieldState::Quoted {
        return Err(format!("line {line}: unterminated quoted CSV field"));
    }
    if active {
        end_row(&mut records, &mut cells, &mut cell, row_start);
    }
    let mut records = records.into_iter();
    let headers = records.next().map(|(_, columns)| columns).unwrap_or_default();
    let rows = records
        .map(|(line, values)| CsvRow { line, values })
        .collect();
    Ok(Csv { headers, rows })
}

fn read_csv(path: &Path) -> Result<Csv, String> {
    let source = fs::read_to_string(path).map_err(|e| e.to_string())?;
    parse_csv(&source)
}

fn field<'a>(csv: &Csv, row: &'a CsvRow, name: &str) -> &'a str {
    csv.headers
        .iter()
        .rposition(|header| header == name)
        .and_then(|index| row.values.get(index))
        .map(String::as_str)
        .unwrap_or("")
}

fn load_catalogue(
    archive: &Path,
    name: &str,
    key: &str,
    expected_columns: usize,
    problems: &mut Vec<String>,
) -> (Csv, BTreeSet<String>) {
    let csv = match read_csv(&archive.join(name)) {
        Ok(csv) => csv,
        Err(error) => {
            problems.push(format!("{name}: failed to read CSV: {error}"));
            return (Csv::default(), BTreeSet::new());
        }
    };
    if csv.headers.len() != expected_columns || !csv.headers.iter().any(|h| h == key) {
        problems.push(format!(
            "{name}: expected {expected_columns} named columns including {key}; found {}",
            csv.headers.len()
        ));
    }
    let mut ids = BTreeSet::new();
    let mut duplicates = BTreeSet::new();
    for row in &csv.rows {
        if row.values.len() != csv.headers.len() {
            problems.push(format!(
                "{name}:{}: malformed number of CSV columns",
                row.line
            ));
        }
        let id = field(&csv, row, key);
        if !id.starts_with("POL-") {
            problems.push(format!(
                "{name}:{}: missing or non-POL identifier: {id:?}",
                row.line
            ));
        }
        if !ids.insert(id.to_string()) {
            duplicates.insert(id.to_string());
        }
        if matches!(name, "CATALOG.csv" | "MAP_CATALOG.csv")
            && !field(&csv, row, "source_url").starts_with("https://")
            && !field(&csv, row, "source_url").starts_with("http://")
        {
            problems.push(format!(
                "{name}:{}: source_url is absent or invalid",
                row.line
            ));
        }
    }
    if !duplicates.is_empty() {
        problems.push(format!(
            "{name}: duplicate IDs: {}",
            duplicates.into_iter().collect::<Vec<_>>().join(", ")
        ));
    }
    (csv, ids)
}

fn markdown_ids(archive: &Path, path: &str, prefix: &str, problems: &mut Vec<String>) -> Vec<String> {
    let source = match fs::read_to_string(archive.join(path)) {
        Ok(source) => source,
        Err(error) => {
            problems.push(format!("{path}: cannot read: {error}"));
            return Vec::new();
        }
    };
    source
        .lines()
        .filter_map(|line| {
            let rest = line.strip_prefix("| \u{60}")?;
            let (id, tail) = rest.split_once('\u{60}')?;
            (id.starts_with(prefix) && tail.starts_with(" |")).then(|| id.to_string())
        })
        .collect()
}

fn check_sets(
    label: &str,
    expected: &BTreeSet<String>,
    actual: &BTreeSet<String>,
    problems: &mut Vec<String>,
) {
    let missing: Vec<_> = expected.difference(actual).collect();
    let extras: Vec<_> = actual.difference(expected).collect();
    if !missing.is_empty() {
        problems.push(format!(
            "{label}: {} missing IDs: {}",
            missing.len(),
            missing.iter().take(8).map(|s| s.as_str()).collect::<Vec<_>>().join(", ")
        ));
    }
    if !extras.is_empty() {
        problems.push(format!(
            "{label}: {} unexpected IDs: {}",
            extras.len(),
            extras.iter().take(8).map(|s| s.as_str()).collect::<Vec<_>>().join(", ")
        ));
    }
}

fn check_duplicate_index(label: &str, ids: &[String], problems: &mut Vec<String>) {
    let mut seen = BTreeSet::new();
    let mut duplicates = BTreeSet::new();
    for id in ids {
        if !seen.insert(id) {
            duplicates.insert(id.as_str());
        }
    }
    if !duplicates.is_empty() {
        problems.push(format!(
            "{label}: duplicate row identifiers: {}",
            duplicates.into_iter().take(8).collect::<Vec<_>>().join(", ")
        ));
    }
}

struct Report {
    master: usize,
    maps: usize,
    political: usize,
    global_index: usize,
    map_index: usize,
    problems: Vec<String>,
}

fn validate_archive(archive: &Path) -> Report {
    let mut problems = Vec::new();
    let (master_csv, master) = load_catalogue(archive, "CATALOG.csv", "ref_id", 22, &mut problems);
    let (map_csv, maps) = load_catalogue(archive, "MAP_CATALOG.csv", "map_id", 17, &mut problems);
    let (political_csv, political) =
        load_catalogue(archive, "POLITICAL_EVENTS.csv", "ref_id", 18, &mut problems);

    match read_csv(&archive.join("MAP_RELATIONSHIPS.csv")) {
        Ok(relationships) => {
            for row in &relationships.rows {
                if row.values.len() != relationships.headers.len() {
                    problems.push(format!(
                        "MAP_RELATIONSHIPS.csv:{}: malformed CSV row",
                        row.line
                    ));
                }
                for name in ["map_id", "related_map_id"] {
                    if !maps.contains(field(&relationships, row, name)) {
                        problems.push(format!(
                            "MAP_RELATIONSHIPS.csv:{}: unresolved {name}",
                            row.line
                        ));
                    }
                }
                if field(&relationships, row, "map_id")
                    == field(&relationships, row, "related_map_id")
                {
                    problems.push(format!("MAP_RELATIONSHIPS.csv:{}: self-reference", row.line));
                }
            }
        }
        Err(error) => problems.push(format!(
            "MAP_RELATIONSHIPS.csv: failed to read: {error}"
        )),
    }

    let intersection: BTreeSet<String> = master.intersection(&maps).cloned().collect();
    check_sets("Master versus map catalogue", &maps, &intersection, &mut problems);
    let intersection: BTreeSet<String> = master.intersection(&political).cloned().collect();
    check_sets(
        "Master versus political catalogue",
        &political,
        &intersection,
        &mut problems,
    );

    for row in &map_csv.rows {
        let id = field(&map_csv, row, "map_id");
        if !id.is_empty() && !id.starts_with("POL-MAP-") {
            problems.push(format!("MAP_CATALOG.csv: invalid map prefix: {id}"));
        }
        let status = field(&map_csv, row, "period_status");
        let year = field(&map_csv, row, "map_year");
        let first_four: String = year.chars().take(4).collect();
        if status.starts_with("modern-")
            && first_four.len() == 4
            && first_four.chars().all(|c| c.is_ascii_digit())
            && first_four.parse::<u32>().is_ok_and(|number| number < 1990)
        {
            problems.push(format!(
                "MAP_CATALOG.csv: modern map with pre-1990 creation year: {id}"
            ));
        }
    }

    let global_ids = markdown_ids(
        archive,
        "media/reference-only/INDEX.md",
        "POL-",
        &mut problems,
    );
    let map_ids = markdown_ids(
        archive,
        "media/reference-only/maps/INDEX.md",
        "POL-MAP-",
        &mut problems,
    );
    check_sets(
        "Unified reference index",
        &master,
        &global_ids.iter().cloned().collect(),
        &mut problems,
    );
    check_sets(
        "Map reference index",
        &maps,
        &map_ids.iter().cloned().collect(),
        &mut problems,
    );
    check_duplicate_index("global index", &global_ids, &mut problems);
    check_duplicate_index("map index", &map_ids, &mut problems);

    Report {
        master: master_csv.rows.len(),
        maps: map_csv.rows.len(),
        political: political_csv.rows.len(),
        global_index: global_ids.len(),
        map_index: map_ids.len(),
        problems,
    }
}

fn archive_dir() -> PathBuf {
    Path::new(env!("CARGO_MANIFEST_DIR"))
        .parent()
        .expect("Archive/tools has parent")
        .to_path_buf()
}

fn main() -> ExitCode {
    let report = validate_archive(&archive_dir());
    println!(
        "Archive: {} master records, {} map records, {} political event records; {} unified index entries, {} map index entries.",
        report.master, report.maps, report.political, report.global_index, report.map_index
    );
    if !report.problems.is_empty() {
        for problem in report.problems {
            eprintln!("FAIL: {problem}");
        }
        return ExitCode::from(1);
    }
    println!("PASS: CSV schemas, unique identifiers and index cross-references.");
    println!("NOTE: Source links, map publication dates and media rights require separate human review.");
    ExitCode::SUCCESS
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::time::{SystemTime, UNIX_EPOCH};

    #[test]
    fn csv_handles_bom_escaped_quotes_embedded_newlines_and_crlf() {
        let csv = parse_csv(
            "\u{feff}id,title\r\n\"POL-1\",\"a, \"\"quoted\"\" value\r\non two lines\"\r\n",
        ).unwrap();
        assert_eq!(csv.headers, ["id", "title"]);
        assert_eq!(csv.rows.len(), 1);
        assert_eq!(csv.rows[0].line, 2);
        assert_eq!(field(&csv, &csv.rows[0], "title"), "a, \"quoted\" value\r\non two lines");
    }

    #[test]
    fn csv_rejects_unterminated_quotes_and_preserves_empty_fields() {
        assert!(parse_csv("id,title\nPOL-1,\"unterminated").is_err());
        let csv = parse_csv("id,title,\nPOL-1,,\n").unwrap();
        assert_eq!(csv.headers.len(), 3);
        assert_eq!(csv.rows[0].values, ["POL-1", "", ""]);
    }

    fn catalogue(key: &str, columns: usize, extra: &[&str], identifiers: &[&str]) -> String {
        let mut names = vec![key.to_string()];
        names.extend(extra.iter().map(|name| name.to_string()));
        while names.len() < columns {
            names.push(format!("extra_{}", names.len()));
        }
        let mut result = format!("{}\n", names.join(","));
        for id in identifiers {
            let values: Vec<_> = names
                .iter()
                .map(|name| match name.as_str() {
                    "ref_id" | "map_id" => *id,
                    "source_url" => "https://example.org/reference",
                    "map_year" => "1981",
                    "period_status" => "period-original",
                    _ => "",
                })
                .collect();
            result.push_str(&values.join(","));
            result.push('\n');
        }
        result
    }

    fn fixture() -> PathBuf {
        let stamp = SystemTime::now().duration_since(UNIX_EPOCH).unwrap().as_nanos();
        let archive = std::env::temp_dir().join(format!(
            "politika-archive-validator-{}-{stamp}",
            std::process::id()
        ));
        fs::create_dir_all(archive.join("media/reference-only/maps")).unwrap();
        let map_ids = ["POL-MAP-A", "POL-MAP-B"];
        fs::write(
            archive.join("CATALOG.csv"),
            catalogue("ref_id", 22, &["source_url"], &["POL-MAP-A", "POL-MAP-B", "POL-POL-A"]),
        ).unwrap();
        fs::write(
            archive.join("MAP_CATALOG.csv"),
            catalogue(
                "map_id", 17, &["source_url", "map_year", "period_status"], &map_ids,
            ),
        ).unwrap();
        fs::write(
            archive.join("POLITICAL_EVENTS.csv"),
            catalogue("ref_id", 18, &[], &["POL-POL-A"]),
        ).unwrap();
        fs::write(
            archive.join("MAP_RELATIONSHIPS.csv"),
            "map_id,related_map_id\nPOL-MAP-A,POL-MAP-B\n",
        ).unwrap();
        fs::write(
            archive.join("media/reference-only/INDEX.md"),
            "| \u{60}POL-MAP-A\u{60} | map |\n| \u{60}POL-MAP-B\u{60} | map |\n| \u{60}POL-POL-A\u{60} | event |\n",
        ).unwrap();
        fs::write(
            archive.join("media/reference-only/maps/INDEX.md"),
            "| \u{60}POL-MAP-A\u{60} | map |\n| \u{60}POL-MAP-B\u{60} | map |\n",
        ).unwrap();
        archive
    }

    #[test]
    fn valid_archive_passes_all_cross_references() {
        let archive = fixture();
        let report = validate_archive(&archive);
        assert!(report.problems.is_empty(), "{:?}", report.problems);
        assert_eq!((report.master, report.maps, report.political), (3, 2, 1));
        fs::remove_dir_all(archive).unwrap();
    }

    #[test]
    fn bad_relationship_duplicate_index_and_modern_year_fail() {
        let archive = fixture();
        fs::write(
            archive.join("MAP_RELATIONSHIPS.csv"),
            "map_id,related_map_id\nPOL-MAP-A,POL-MAP-A\nPOL-MAP-A,POL-MAP-MISSING\n",
        ).unwrap();
        fs::write(
            archive.join("media/reference-only/maps/INDEX.md"),
            "| \u{60}POL-MAP-A\u{60} | map |\n| \u{60}POL-MAP-A\u{60} | duplicate |\n",
        ).unwrap();
        let mut csv = fs::read_to_string(archive.join("MAP_CATALOG.csv")).unwrap();
        csv = csv.replace("1981,period-original", "1981,modern-reconstruction");
        fs::write(archive.join("MAP_CATALOG.csv"), csv).unwrap();
        let report = validate_archive(&archive);
        let problems = report.problems.join("\n");
        assert!(problems.contains("self-reference"), "{problems}");
        assert!(problems.contains("unresolved related_map_id"), "{problems}");
        assert!(problems.contains("duplicate row identifiers"), "{problems}");
        assert!(problems.contains("modern map with pre-1990"), "{problems}");
        fs::remove_dir_all(archive).unwrap();
    }
}
