use politika_version::{
    build_identity, current_tag, header, identity_json, load_manifest, repo_root,
    validate_release, validate_tag, write_if_changed,
};
use std::env;
use std::path::PathBuf;

fn run() -> Result<(), String> {
    let mut args = env::args().skip(1);
    let command = args.next().ok_or(
        "usage: cargo run --manifest-path tools/version/Cargo.toml -- <print|check|identity|generate> [--release] [--tag vVERSION] [--output PATH]"
    )?;
    let mut release = false;
    let mut tag_override: Option<String> = None;
    let mut output: Option<PathBuf> = None;
    while let Some(option) = args.next() {
        match option.as_str() {
            "--release" if !release => release = true,
            "--tag" if tag_override.is_none() => {
                tag_override = Some(args.next().ok_or("--tag needs a value")?);
            }
            "--output" if output.is_none() => {
                output = Some(PathBuf::from(args.next().ok_or("--output needs a path")?));
            }
            _ => return Err(format!("unrecognized or repeated option: {option}")),
        }
    }
    if !matches!(command.as_str(), "print" | "check" | "identity" | "generate") {
        return Err(format!("unknown command: {command}"));
    }
    if command == "print" && (release || tag_override.is_some() || output.is_some()) {
        return Err("print takes no options".into());
    }
    if command != "generate" && output.is_some() {
        return Err("--output is only supported with generate".into());
    }

    let root = repo_root();
    let data = load_manifest(&root)?;
    if command == "print" {
        println!("{}", data.game_version);
        return Ok(());
    }
    let tag = tag_override.or_else(current_tag);
    validate_tag(&data, tag.as_deref())?;
    if release {
        validate_release(&root, &data, tag.as_deref())?;
    }
    let build = build_identity(&root);
    match command.as_str() {
        "check" => println!(
            "POLITIKA {} · build {} · git {} · protocol {} · save {} · persistence {} · world {}",
            data.game_version, build.build_number, build.git_revision,
            data.network_protocol, data.save_schema, data.persistence_schema, data.world_schema
        ),
        "identity" => println!("{}", identity_json(&data, &build)),
        "generate" => {
            let output = output.unwrap_or_else(|| root.join("build/generated/PolitikaVersion.generated.h"));
            let output = if output.is_absolute() { output } else { root.join(output) };
            write_if_changed(&output, &header(&data, &build))?;
            println!("{}", output.display());
        }
        _ => unreachable!(),
    }
    Ok(())
}

fn main() {
    if let Err(error) = run() {
        eprintln!("version error: {error}");
        std::process::exit(2);
    }
}
