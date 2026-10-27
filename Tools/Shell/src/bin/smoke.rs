//! Process-level shell regression checks. Isolated -UserDir fixtures, never player data.
//! Usage: cargo run --manifest-path Tools/Shell/Cargo.toml --bin smoke -- BINARY [UPROJECT]
#[cfg(unix)]
use std::os::unix::fs::PermissionsExt;
use std::{
    env, fs,
    path::{Path, PathBuf},
    process::{Command, Stdio},
    thread,
    time::{Duration, Instant, SystemTime, UNIX_EPOCH},
};

type Check = Result<(), String>;
fn require(ok: bool, why: &str) -> Check {
    if ok { Ok(()) } else { Err(why.into()) }
}
fn read(path: &Path) -> Result<Vec<u8>, String> {
    fs::read(path).map_err(|e| format!("{}: {e}", path.display()))
}
fn write(path: &Path, data: &[u8]) -> Check {
    fs::create_dir_all(path.parent().unwrap()).map_err(|e| e.to_string())?;
    fs::write(path, data).map_err(|e| e.to_string())
}
const SETTINGS: &[u8] = b"[/Script/Politika.PolitikaUserSettings]\nVersion=5\nResolutionSizeX=1600\nResolutionSizeY=900\nFullscreenMode=2\nFrameRateLimit=47.0\n; preserve this exact fixture\n";
struct Runner {
    binary: PathBuf,
    project: Option<PathBuf>,
    root: PathBuf,
}
impl Runner {
    fn settings(&self, dir: &Path) -> PathBuf {
        dir.join(if self.project.is_some() {
            "Saved/Config/LinuxEditor/GameUserSettings.ini"
        } else {
            "Saved/Config/Linux/GameUserSettings.ini"
        })
    }
    fn launch(
        &self,
        dir: &Path,
        label: &str,
        extra: &[String],
        network_off: bool,
        code: i32,
    ) -> Result<String, String> {
        fs::create_dir_all(dir).map_err(|e| e.to_string())?;
        let logfile = dir.join(format!("{label}.log"));
        let console = fs::File::create(dir.join(format!("{label}.console.log")))
            .map_err(|e| e.to_string())?;
        let mut cmd = if network_off {
            let mut c = Command::new("unshare");
            c.args(["--user", "--map-current-user", "--net", "--"])
                .arg(&self.binary);
            c
        } else {
            Command::new(&self.binary)
        };
        if let Some(project) = &self.project {
            cmd.arg(project).arg("-game");
        }
        cmd.args(extra)
            .args([
                "-nullrhi",
                "-unattended",
                "-nosound",
                "-NoSplash",
                "-ExecCmds=Quit",
            ])
            .arg(format!("-UserDir={}/", dir.display()))
            .arg(format!("-ABSLOG={}", logfile.display()))
            .stdout(Stdio::from(console.try_clone().map_err(|e| e.to_string())?))
            .stderr(Stdio::from(console));
        let mut child = cmd.spawn().map_err(|e| e.to_string())?;
        let start = Instant::now();
        let status = loop {
            if let Some(status) = child.try_wait().map_err(|e| e.to_string())? {
                break status;
            }
            if start.elapsed() > Duration::from_secs(90) {
                let _ = child.kill();
                let _ = child.wait();
                return Err(format!("{label}: timeout (not a pass)"));
            }
            thread::sleep(Duration::from_millis(100));
        };
        require(
            status.code() == Some(code),
            &format!(
                "{label}: expected exit {code}, got {status}; inspect {}",
                logfile.display()
            ),
        )?;
        let log = fs::read_to_string(&logfile).map_err(|e| e.to_string())?;
        if code == 0 {
            require(
                log.contains("\"event\":\"ui_skipped\""),
                "world/player did not finish startup",
            )?;
            require(
                !log.contains("\"event\":\"startup_fatal\""),
                "unexpected fatal event",
            )?;
        }
        Ok(log)
    }
    fn run(&self, name: &str) -> Check {
        let dir = self.root.join(name);
        let settings = self.settings(&dir);
        let profile = dir.join("Saved/SaveGames/PolitikaLocalProfile.sav");
        let sentinel = dir.join("Saved/SaveGames/CampaignPreservationSentinel.sav");
        match name {
            "first-run-and-restart" => {
                let log = self.launch(&dir, "first", &[], false, 0)?;
                require(
                    log.contains("LoadMap: /Game/Maps/MainMenu"),
                    "wrong default map",
                )?;
                require(
                    log.contains("\"outcome\":\"persistent\""),
                    "profile was not persisted",
                )?;
                let original = read(&profile)?;
                self.launch(&dir, "restart", &[], false, 0)?;
                require(
                    read(&profile)? == original,
                    "local profile changed on restart",
                )
            }
            "explicit-map" => {
                let log =
                    self.launch(&dir, "run", &["/Game/Maps/OfflineSandbox".into()], false, 0)?;
                require(
                    log.contains("Game class is 'PolitikaOfflineGameMode'"),
                    "explicit map override did not select sandbox",
                )
            }
            "safe-preserves-settings-and-saves" | "reset-preserves-saves" => {
                self.launch(&dir, "seed", &[], false, 0)?;
                let original_profile = read(&profile)?;
                write(&settings, SETTINGS)?;
                write(&sentinel, b"campaign preservation fixture")?;
                let safe = name.starts_with("safe");
                let flag = if safe {
                    "-PolitikaSafeMode"
                } else {
                    "-PolitikaResetUserConfig"
                };
                let log = self.launch(&dir, "run", &[flag.into()], false, 0)?;
                require(read(&profile)? == original_profile, "profile bytes changed")?;
                require(
                    read(&sentinel)? == b"campaign preservation fixture",
                    "save sentinel changed",
                )?;
                if safe {
                    require(
                        log.contains("\"event\":\"settings_overlay\""),
                        "safe overlay not active",
                    )?;
                    require(
                        read(&settings)? == SETTINGS,
                        "safe mode mutated original settings",
                    )
                } else {
                    require(
                        log.contains("\"outcome\":\"backed_up\""),
                        "no verified reset event",
                    )?;
                    let backups: Vec<_> = fs::read_dir(settings.parent().unwrap())
                        .map_err(|e| e.to_string())?
                        .filter_map(Result::ok)
                        .filter(|e| {
                            e.file_name()
                                .to_string_lossy()
                                .starts_with("GameUserSettings.ini.backup-")
                        })
                        .collect();
                    require(backups.len() == 1, "reset must produce exactly one backup")?;
                    require(
                        read(&backups[0].path())? == SETTINGS,
                        "backup differs from source",
                    )?;
                    require(
                        !settings.exists() || read(&settings)? != SETTINGS,
                        "reset left original preferences active",
                    )
                }
            }
            "corrupt-settings" => {
                let invalid = b"[/Script/Politika.PolitikaUserSettings]\nVersion=999\nResolutionSizeX=broken\n";
                write(&settings, invalid)?;
                let log = self.launch(&dir, "run", &[], false, 0)?;
                require(
                    log.contains("\"code\":\"InvalidUserSettings\""),
                    "missing settings recovery",
                )?;
                require(
                    read(&settings)? == invalid,
                    "corrupt settings were overwritten",
                )
            }
            "corrupt-profile" => {
                write(&profile, b"deliberately invalid profile")?;
                let log = self.launch(&dir, "run", &[], false, 0)?;
                require(
                    log.contains("\"code\":\"ProfileInvalid\""),
                    "missing profile recovery",
                )?;
                require(
                    read(&profile)? == b"deliberately invalid profile",
                    "corrupt profile was overwritten",
                )
            }
            "profile-checksum-failure" | "profile-truncation" => {
                self.launch(&dir, "seed", &[], false, 0)?;
                let mut damaged = read(&profile)?;
                if name == "profile-truncation" {
                    damaged.truncate(damaged.len() / 2);
                } else {
                    let last = damaged.len() - 1;
                    damaged[last] ^= 0x40;
                }
                write(&profile, &damaged)?;
                let log = self.launch(&dir, "run", &[], false, 0)?;
                require(
                    log.contains("\"code\":\"ProfileInvalid\""),
                    "missing corrupt-envelope recovery",
                )?;
                require(
                    read(&profile)? == damaged,
                    "damaged profile was overwritten",
                )
            }
            "profile-write-failure" => {
                write(&dir.join("Saved/SaveGames"), b"block directory creation")?;
                let log = self.launch(&dir, "run", &[], false, 0)?;
                require(
                    log.contains("\"code\":\"ProfileWriteFailed\""),
                    "missing profile-write recovery",
                )?;
                require(
                    log.contains("\"outcome\":\"temporary\""),
                    "no temporary offline profile",
                )
            }
            "invalid-switch-cancels-reset" | "safe-wins-over-reset" => {
                write(&settings, SETTINGS)?;
                let invalid = name.starts_with("invalid");
                let flags = vec![
                    "-PolitikaResetUserConfig".into(),
                    if invalid {
                        "-PolitikaOffline=invalid".into()
                    } else {
                        "-PolitikaSafeMode".into()
                    },
                ];
                let log = self.launch(&dir, "run", &flags, false, 0)?;
                require(
                    log.contains("\"code\":\"InvalidLaunchOptions\""),
                    "missing launch recovery",
                )?;
                require(
                    !log.contains("\"event\":\"settings_reset\""),
                    "invalid/conflicting flags reset preferences",
                )?;
                if invalid {
                    require(
                        log.contains("\"reset_requested\":false"),
                        "malformed switch did not cancel reset",
                    )?;
                    let saved = String::from_utf8_lossy(&read(&settings)?).into_owned();
                    require(
                        saved.contains("FrameRateLimit=47"),
                        "invalid switch lost saved preferences",
                    )
                } else {
                    require(read(&settings)? == SETTINGS, "original settings changed")
                }
            }
            "no-network" => {
                let log = self.launch(&dir, "run", &[], true, 0)?;
                require(
                    log.contains("\"outcome\":\"persistent\""),
                    "offline first-run failed",
                )
            }
            "fatal-override-conflict" => {
                write(&settings, SETTINGS)?;
                let external = dir.join("external.ini");
                write(&external, SETTINGS)?;
                let log = self.launch(
                    &dir,
                    "run",
                    &[
                        "-PolitikaSafeMode".into(),
                        format!("-GameUserSettingsINI={}", external.display()),
                    ],
                    false,
                    70,
                )?;
                require(
                    log.contains("\"event\":\"startup_fatal\"")
                        && log.contains("\"code\":\"ConflictingSettingsOverride\""),
                    "fatal diagnostic not flushed",
                )?;
                require(
                    !log.contains("\"event\":\"ui_skipped\""),
                    "fatal run reached world startup",
                )?;
                require(
                    read(&settings)? == SETTINGS && read(&external)? == SETTINGS,
                    "fatal conflict altered settings",
                )
            }
            "backup-write-failure" => {
                write(&settings, SETTINGS)?;
                #[cfg(unix)]
                fs::set_permissions(
                    settings.parent().unwrap(),
                    fs::Permissions::from_mode(0o555),
                )
                .map_err(|e| e.to_string())?;
                let outcome =
                    self.launch(&dir, "run", &["-PolitikaResetUserConfig".into()], false, 0);
                #[cfg(unix)]
                fs::set_permissions(
                    settings.parent().unwrap(),
                    fs::Permissions::from_mode(0o755),
                )
                .map_err(|e| e.to_string())?;
                let log = outcome?;
                require(
                    log.contains("\"code\":\"SettingsBackupFailed\""),
                    "missing backup-failure recovery",
                )?;
                require(
                    read(&settings)? == SETTINGS,
                    "failed backup changed original settings",
                )
            }
            _ => Err("unknown check".into()),
        }
    }
}
fn main() -> Result<(), String> {
    let mut args = env::args_os().skip(1);
    let binary = PathBuf::from(args.next().ok_or("usage: smoke BINARY [UPROJECT]")?)
        .canonicalize()
        .map_err(|e| e.to_string())?;
    let project = args.next().map(PathBuf::from);
    let epoch = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_millis();
    let root = PathBuf::from(env!("CARGO_MANIFEST_DIR"))
        .join(format!("../../Saved/ShellChecks/smoke-{epoch}"))
        .canonicalize()
        .unwrap_or_else(|_| {
            PathBuf::from(env!("CARGO_MANIFEST_DIR"))
                .join(format!("../../Saved/ShellChecks/smoke-{epoch}"))
        });
    fs::create_dir_all(&root).map_err(|e| e.to_string())?;
    let runner = Runner {
        binary,
        project,
        root: root.canonicalize().map_err(|e| e.to_string())?,
    };
    let names = [
        "first-run-and-restart",
        "explicit-map",
        "safe-preserves-settings-and-saves",
        "reset-preserves-saves",
        "corrupt-settings",
        "corrupt-profile",
        "profile-checksum-failure",
        "profile-truncation",
        "profile-write-failure",
        "invalid-switch-cancels-reset",
        "safe-wins-over-reset",
        "no-network",
        "fatal-override-conflict",
        "backup-write-failure",
    ];
    let mut failed = 0;
    let mut report = String::from("check\tverdict\tdetail\n");
    for name in names {
        let (verdict, detail) = match runner.run(name) {
            Ok(()) => ("PASS", String::new()),
            Err(e) => {
                failed += 1;
                ("FAIL", e)
            }
        };
        println!("{verdict} {name}: {detail}");
        report.push_str(&format!("{name}\t{verdict}\t{detail}\n"));
        fs::write(runner.root.join("results.tsv"), &report).map_err(|e| e.to_string())?;
    }
    println!(
        "{} / {} passed; evidence: {}",
        names.len() - failed,
        names.len(),
        runner.root.display()
    );
    require(failed == 0, "one or more shell checks failed")
}
