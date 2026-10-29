mod manifest;
mod registry;
mod tags;

use std::{env, path::PathBuf, process::ExitCode};

fn run() -> Result<(), String> {
    let mut client_registry = None;
    let mut server_registry = None;
    let mut client_config = None;
    let mut server_config = None;
    let mut queries = None;
    let mut output = None;
    let mut args = env::args_os().skip(1);
    while let Some(key) = args.next() {
        if key == "--help" || key == "-h" {
            println!(
                "tempest-reference --client-registry FILE --server-registry FILE --client-config DIR --server-config DIR --queries FILE --output DIR\nReads only exported TSV/INI metadata. Output must be /mnt/data/CommandAndConquer/UM/Tempest/politika-transfer."
            );
            return Ok(());
        }
        let value = args
            .next()
            .ok_or_else(|| format!("Missing value for {}", key.to_string_lossy()))?;
        let slot = match key.to_str() {
            Some("--client-registry") => &mut client_registry,
            Some("--server-registry") => &mut server_registry,
            Some("--client-config") => &mut client_config,
            Some("--server-config") => &mut server_config,
            Some("--queries") => &mut queries,
            Some("--output") => &mut output,
            _ => return Err(format!("Unknown option: {}", key.to_string_lossy())),
        };
        *slot = Some(PathBuf::from(value));
    }
    let required = |value: Option<PathBuf>, name: &str| {
        value.ok_or_else(|| format!("Missing required {name}"))
    };
    let client_registry = required(client_registry, "--client-registry")?;
    let server_registry = required(server_registry, "--server-registry")?;
    let client_config = required(client_config, "--client-config")?;
    let server_config = required(server_config, "--server-config")?;
    let queries = required(queries, "--queries")?;
    let output = required(output, "--output")?;
    const ALLOWED: &str = "/mnt/data/CommandAndConquer/UM/Tempest/politika-transfer";
    if output != PathBuf::from(ALLOWED) {
        return Err(format!("Output is restricted to {ALLOWED}"));
    }
    let query_data = registry::read_queries(&queries)?;
    let mut inputs = vec![
        client_registry.clone(),
        server_registry.clone(),
        queries.clone(),
    ];
    inputs.extend(tags::ini_files(&client_config)?);
    inputs.extend(tags::ini_files(&server_config)?);
    inputs.sort();
    inputs.dedup();
    let initial_hashes = manifest::hash_inputs(&inputs)?;
    let client_records = registry::read_registry(&client_registry, "client", &query_data)?;
    let server_records = registry::read_registry(&server_registry, "server", &query_data)?;
    let client_tags = tags::read_tags(&client_config, "client")?;
    let server_tags = tags::read_tags(&server_config, "server")?;
    let duplicate_count =
        registry::duplicate_count(&client_records) + registry::duplicate_count(&server_records);
    let asset_csv = registry::asset_csv(&client_records, &server_records);
    let tag_csv = tags::tag_csv(&client_tags, &server_tags);
    let manifest = manifest::build(
        &inputs,
        client_records.len() + server_records.len(),
        client_tags.len() + server_tags.len(),
        duplicate_count,
        &initial_hashes,
    )?;
    std::fs::create_dir_all(&output).map_err(|e| format!("Cannot create output directory: {e}"))?;
    manifest::write_atomic(&output.join("asset-index.csv"), asset_csv.as_bytes())?;
    manifest::write_atomic(&output.join("tag-index.csv"), tag_csv.as_bytes())?;
    manifest::write_atomic(
        &output.join("inputs.sha256"),
        manifest.hash_lines.as_bytes(),
    )?;
    manifest::write_atomic(&output.join("run-manifest.json"), manifest.json.as_bytes())?;
    println!(
        "Indexed {} registry rows ({} duplicate rows) and {} tags; {} hashed inputs.",
        client_records.len() + server_records.len(),
        duplicate_count,
        client_tags.len() + server_tags.len(),
        inputs.len()
    );
    Ok(())
}

fn main() -> ExitCode {
    match run() {
        Ok(()) => ExitCode::SUCCESS,
        Err(error) => {
            eprintln!("tempest-reference: {error}");
            ExitCode::FAILURE
        }
    }
}
