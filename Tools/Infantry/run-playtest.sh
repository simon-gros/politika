#!/usr/bin/env bash
set -euo pipefail
project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
launcher="${INFANTRY_ARCHIVE_DIR:-$project_root/Saved/Packages}/Linux/Politika.sh"
if [[ ! -x "$launcher" ]]; then
    echo "Missing local play-test package: $launcher" >&2
    exit 1
fi
exec "$launcher" /Game/Maps/RTSVerticalSlice \
    -windowed -ResX=1920 -ResY=1080 \
    "-UserDir=${INFANTRY_USER_DIR:-$project_root/Saved/InfantryDefaultAudit20261028/PlayUser/}" "$@"
