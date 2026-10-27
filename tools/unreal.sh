#!/usr/bin/env bash
# Local Unreal build/test entry point. No background execution or hosted workflows.
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
# shellcheck disable=SC1091
source "$repo_root/tools/unreal-version.env"

usage() {
  echo "Usage: bash tools/unreal.sh {audit|core-test|doctor|build|test} [extra Unreal arguments...]" >&2
}

quality() {
  if ! command -v cargo >/dev/null 2>&1; then
    echo "INFRA_ERROR: Rust Cargo is required (Rust edition 2024, rustc >= 1.85)." >&2
    return 2
  fi
  cargo run --locked --quiet --manifest-path "$repo_root/tools/version/Cargo.toml" --bin quality -- "$@"
}

action="${1:-}"
if [[ -z "$action" ]]; then
  usage
  exit 2
fi
shift
case "$action" in
  audit)
    quality audit "$@"
    ;;
  core-test)
    quality core "$@"
    ;;
  doctor)
    quality preflight "$@"
    ;;
  build|test)
    quality preflight
    engine="${UNREAL_ENGINE_DIR:-${POLITIKA_UNREAL_ENGINE_DIR:-}}"
    project="${POLITIKA_UPROJECT:-$repo_root/$UNREAL_PROJECT}"
    if [[ "$action" == "build" ]]; then
      if [[ "$(uname -s)" != "Linux" ]]; then
        echo "BLOCKED: Linux Unreal build wrapper; use the native Windows Unreal build tools on Windows." >&2
        exit 3
      fi
      script="$engine/Engine/Build/BatchFiles/Linux/Build.sh"
      if [[ ! -f "$script" ]]; then
        echo "BLOCKED: Unreal Build.sh is missing from the verified engine installation." >&2
        exit 3
      fi
      exec bash "$script" PolitikaEditor Linux Development "-Project=$project" -WaitMutex -NoHotReload "$@"
    fi
    editor="$engine/Engine/Binaries/Linux/UnrealEditor-Cmd"
    if [[ ! -x "$editor" ]]; then
      echo "BLOCKED: UnrealEditor-Cmd is not built at $editor." >&2
      exit 3
    fi
    mkdir -p "$repo_root/build/diagnostics/unreal-automation"
    "$editor" "$project" -unattended -nop4 -nullrhi -nosplash \
      "-ExecCmds=Automation RunTests Politika; Quit" \
      "-ReportExportPath=$repo_root/build/diagnostics/unreal-automation" "$@"
    echo "INCONCLUSIVE: inspect the Unreal Automation JSON/XML results before calling gameplay tests passed." >&2
    exit 4
    ;;
  *)
    usage
    exit 2
    ;;
esac
