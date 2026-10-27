//! Thin UE process orchestration. Runtime switches/settings remain exclusively in C++.
use std::{
    env,
    ffi::OsString,
    fs,
    path::PathBuf,
    process::{Command, ExitCode},
};

fn invoke(program: PathBuf, args: &[OsString]) -> Result<(), String> {
    let status = Command::new(&program)
        .args(args)
        .status()
        .map_err(|error| format!("Could not run {}: {error}", program.display()))?;
    if status.success() {
        Ok(())
    } else {
        Err(format!("{} exited with {status}", program.display()))
    }
}

fn run() -> Result<(), String> {
    let mut args = env::args_os().skip(1);
    let action = args.next().unwrap_or_else(|| OsString::from("help"));
    if action == "help" || action == "--help" {
        println!(
            "POLITIKA shell build helper (Linux host)\n\
            Set UE_ROOT to the Unreal Engine 5.8.3 directory.\n\
            Commands: build-editor | maps | prepare | build-game | package | editor | run [-- UE arguments]\n\
            prepare = build-editor + maps. package = prepare + Linux Development BuildCookRun.\n\
            Existing maps are preserved. Packages go to Saved/Packages. No runtime options are interpreted here."
        );
        return Ok(());
    }
    if !cfg!(target_os = "linux") {
        return Err("This helper currently supports a Linux build host; use native UBT/UAT for other hosts.".into());
    }
    let engine = PathBuf::from(
        env::var_os("UNREAL_ENGINE_DIR")
            .or_else(|| env::var_os("UE_ROOT"))
            .ok_or("Set UNREAL_ENGINE_DIR (or UE_ROOT) to your UE 5.8.3 directory.")?,
    )
    .canonicalize()
    .map_err(|e| format!("UE_ROOT is inaccessible: {e}"))?;
    let root = PathBuf::from(env!("CARGO_MANIFEST_DIR"))
        .join("../..")
        .canonicalize()
        .map_err(|e| format!("Project root is inaccessible: {e}"))?;
    let project = root.join("Politika.uproject");
    // Reuse the repository's exact-source-pin check; do not maintain another engine contract.
    if matches!(
        action.to_str(),
        Some("build-editor" | "build-game" | "prepare" | "package" | "maps")
    ) {
        let status = Command::new("cargo")
            .current_dir(&root)
            .env("UNREAL_ENGINE_DIR", &engine)
            .args([
                "run",
                "--locked",
                "--quiet",
                "--manifest-path",
                "tools/version/Cargo.toml",
                "--bin",
                "quality",
                "--",
                "preflight",
            ])
            .status()
            .map_err(|e| format!("Engine preflight failed to start: {e}"))?;
        if !status.success() {
            return Err(format!("Engine preflight did not pass: {status}"));
        }
    }
    let build = engine.join("Engine/Build/BatchFiles/Linux/Build.sh");
    let editor = engine.join("Engine/Binaries/Linux/UnrealEditor");
    let editor_cmd = engine.join("Engine/Binaries/Linux/UnrealEditor-Cmd");
    if !project.is_file() || !build.is_file() || !editor_cmd.is_file() {
        return Err(
            "Expected Politika.uproject and UE Linux build/editor executables were not found."
                .into(),
        );
    }
    let mut extra: Vec<_> = args.collect();
    if extra.first().is_some_and(|s| s == "--") {
        extra.remove(0);
    }
    if action != "run" && action != "editor" && !extra.is_empty() {
        return Err("Only run/editor accept trailing UE arguments.".into());
    }
    let jobs = env::var("POLITIKA_BUILD_JOBS").unwrap_or_else(|_| "6".into());
    let jobs: u32 = jobs
        .parse()
        .map_err(|_| "POLITIKA_BUILD_JOBS must be an integer from 1 to 64.")?;
    if !(1..=64).contains(&jobs) {
        return Err("POLITIKA_BUILD_JOBS must be an integer from 1 to 64.".into());
    }
    let compile = |target: &str| {
        invoke(
            build.clone(),
            &[
                target.into(),
                "Linux".into(),
                "Development".into(),
                project.clone().into_os_string(),
                "-WaitMutex".into(),
                format!("-MaxParallelActions={jobs}").into(),
            ],
        )
    };
    let maps = || {
        invoke(
            editor_cmd.clone(),
            &[
                project.clone().into_os_string(),
                "-run=PolitikaCreateShell".into(),
                "-unattended".into(),
                "-nosplash".into(),
                "-nullrhi".into(),
                "-nosound".into(),
            ],
        )
    };
    match action.to_str() {
        Some("build-editor") => compile("PolitikaEditor"),
        Some("build-game") => compile("Politika"),
        Some("maps") => maps(),
        Some("prepare") => {
            compile("PolitikaEditor")?;
            maps()
        }
        Some("package") => {
            compile("PolitikaEditor")?;
            maps()?;
            invoke(
                engine.join("Engine/Build/BatchFiles/RunUAT.sh"),
                &[
                    "BuildCookRun".into(),
                    format!("-project={}", project.display()).into(),
                    "-noP4".into(),
                    "-platform=Linux".into(),
                    "-clientconfig=Development".into(),
                    "-build".into(),
                    "-cook".into(),
                    "-stage".into(),
                    "-pak".into(),
                    "-iostore".into(),
                    "-archive".into(),
                    "-unattended".into(),
                    "-utf8output".into(),
                    format!(
                        "-archivedirectory={}",
                        root.join("Saved/Packages").display()
                    )
                    .into(),
                    format!("-UbtArgs=-MaxParallelActions={jobs}").into(),
                ],
            )?;
            let identity = Command::new("cargo")
                .current_dir(&root)
                .args([
                    "run",
                    "--locked",
                    "--quiet",
                    "--manifest-path",
                    "tools/version/Cargo.toml",
                    "--",
                    "identity",
                ])
                .output()
                .map_err(|e| format!("Build identity failed: {e}"))?;
            if !identity.status.success() {
                return Err("Build identity generation failed after packaging.".into());
            }
            let archive = root.join("Saved/Packages/Linux");
            if !archive.join("Politika.sh").is_file() {
                return Err(
                    "UAT returned success without the expected Linux archive launcher.".into(),
                );
            }
            fs::write(archive.join("BuildIdentity.json"), identity.stdout)
                .map_err(|e| format!("Could not write package identity: {e}"))?;
            fs::copy(
                root.join("tools/unreal-version.env"),
                archive.join("unreal-version.env"),
            )
            .map_err(|e| format!("Could not archive engine pin: {e}"))?;
            Ok(())
        }
        Some("editor") | Some("run") => {
            let mut launch_args = vec![project.into_os_string()];
            if action == "run" {
                launch_args.push("-game".into());
            }
            launch_args.extend(extra);
            invoke(editor, &launch_args)
        }
        _ => Err("Unknown helper command. Use help.".into()),
    }
}

fn main() -> ExitCode {
    match run() {
        Ok(()) => ExitCode::SUCCESS,
        Err(error) => {
            eprintln!("politika-shell: {error}");
            ExitCode::FAILURE
        }
    }
}
