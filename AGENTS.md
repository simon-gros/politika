# POLITIKA — Contributor and Coding-Agent Rules

POLITIKA is an alternate-history RTS set in 1980s Yugoslavia. The repository is at a foundational documentation/design stage; Unreal Engine 5.8.3 is the proposed runtime, not yet evidence of a playable build. Primary planned targets are Linux, Windows, SteamOS/Steam Deck and Android; consoles and Apple are possible later expansions.

The authoritative version lives in [version.json](version.json). Follow [Versioning & Release Management](Docs/VERSIONING_RELEASE_MANAGEMENT_ROADMAP.md).

- AI agents may increment game_version and dev/alpha/beta/rc counters when *implemented and verified* milestones justify them, using the smallest defensible SemVer forward step, updating CHANGELOG.md and recording the validation evidence. No extra per-bump approval is needed.
- Never advance versions for routine commits, documentation-only work, history archives, aesthetic references, speculative gameplay mechanics or target-platform announcements. Do not claim a shipped release before an actual validated package exists.
- Keep network_protocol, save_schema, persistence_schema and world_schema separate. Increment only on real incompatible changes to an implemented contract and add migration/regression tests in the same change.
- Never rewrite a published version/tag, decrease compatibility counters, commit build numbers or make an independently maintained Unreal version string.
- Use Rust/Cargo exclusively for project versioning tools. For version changes run `cargo run --quiet --manifest-path tools/version/Cargo.toml -- check`, `cargo test --manifest-path tools/version/Cargo.toml --all-targets`, and `cargo fmt --manifest-path tools/version/Cargo.toml -- --check`. Record actual Cargo validation results rather than assuming the checks passed.
- Run `cargo run --locked --quiet --manifest-path tools/version/Cargo.toml --bin quality -- audit` after changing faction registry metadata or local toolchain contracts. Run `cargo run --locked --quiet --manifest-path tools/version/Cargo.toml --bin quality -- core` after changing the C++ RTS rules kernel. Run `bash tools/unreal.sh doctor` when evaluating a local Unreal checkout; missing engine/project prerequisites must be reported as BLOCKED, not PASS. Follow [Development Foundations](Docs/DEVELOPMENT_FOUNDATIONS.md) and never claim Unreal gameplay validation from Rust metadata tests alone.
- Integrate the generated identity into Unreal startup/About/logs/build artifacts only once executable systems exist. All platform targets share a game version; platform support must be empirically tested.
- Preserve historical sourcing, copyright and licenses, and distinguish fictional in-game factions from history.

Keep POLITIKA's release process self-contained, with explicit game compatibility identities and no unrelated engine-specific assumptions.
