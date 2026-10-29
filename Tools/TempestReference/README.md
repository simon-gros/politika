# Tempest reference indexer (offline)

This standalone Rust utility filters the existing client/server Asset Registry TSV exports and extracted tag INI files. It requires the host `sha256sum` executable for reproducible input/source fingerprints. It never reads Unreal package payloads, launches the game, contacts the network, or writes into POLITIKA `Content/`.

The explicit filters live in `queries.tsv`. They are discovery queries, not semantic classifications. Registry paths and tag strings stay verbatim, including case variants and unusual tag spellings. A selected row is not proof of runtime behavior, import suitability, licensing, or compatibility.

Run from the POLITIKA repository root:

```sh
cargo test --locked --manifest-path Tools/TempestReference/Cargo.toml
cargo fmt --manifest-path Tools/TempestReference/Cargo.toml -- --check
cargo run --locked --manifest-path Tools/TempestReference/Cargo.toml -- \
  --client-registry /mnt/data/CommandAndConquer/UM/Tempest/work/client-asset-registry-records.tsv \
  --server-registry /mnt/data/CommandAndConquer/UM/Tempest/work/server-asset-registry-records.tsv \
  --client-config /mnt/data/CommandAndConquer/UM/Tempest/extracted/client/PakFiles/pakchunk0-Windows/Tempest/Config \
  --server-config /mnt/data/CommandAndConquer/UM/Tempest/extracted/server/PakFiles/pakchunk0-WindowsServer/Tempest/Config \
  --queries Tools/TempestReference/queries.tsv \
  --output /mnt/data/CommandAndConquer/UM/Tempest/politika-transfer
```

Outputs are restricted to the external UM transfer directory: `asset-index.csv`, `tag-index.csv`, `inputs.sha256`, and `run-manifest.json`. Re-run after changing any input or filter; compare hashes before treating generated rows as a reviewed subset.

TR-2 crosswalk files live beside those generated catalogs as `category-crosswalk.csv` and `relationship-crosswalk.csv`. Asset source references use `asset|side|class|exact-package-path`; tag references use `tag|side|exact-config-path|exact-tag`. Crosswalks contain explicit POLITIKA-owned dispositions, owner/gap rationale, evidence limits, and hashes for the exact source manifest and catalogs. Validate them read-only after editing:

The current crosswalk is an agent-prepared draft for project-owner review, not Simon-approved implementation authority. `redefine` rows express bounded design comparisons only; they do not schedule or authorize runtime work.

```sh
cargo run --locked --manifest-path Tools/TempestReference/Cargo.toml -- validate-crosswalk \
  --asset-index /mnt/data/CommandAndConquer/UM/Tempest/politika-transfer/asset-index.csv \
  --tag-index /mnt/data/CommandAndConquer/UM/Tempest/politika-transfer/tag-index.csv \
  --manifest /mnt/data/CommandAndConquer/UM/Tempest/politika-transfer/run-manifest.json \
  --categories /mnt/data/CommandAndConquer/UM/Tempest/politika-transfer/category-crosswalk.csv \
  --relationships /mnt/data/CommandAndConquer/UM/Tempest/politika-transfer/relationship-crosswalk.csv
```
