#!/usr/bin/env bash
# Compile and run POLITIKA's engine-independent C++20 rules regression suite.
# This fallback needs a C++ compiler, not Rust/Cargo or Unreal Engine.
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
source_file="$repo_root/Tests/Core/RulesTests.cpp"
include_dir="$repo_root/Source/PolitikaCore"
compiler="${CXX:-c++}"

if [[ ! -f "$source_file" || ! -f "$include_dir/Rules.h" ]]; then
  echo "BLOCKED: portable rules sources are missing from $repo_root." >&2
  exit 3
fi
if ! command -v "$compiler" >/dev/null 2>&1; then
  echo "BLOCKED: C++20 compiler '$compiler' is unavailable (set CXX to its executable)." >&2
  exit 3
fi

scratch="$(mktemp -d "${TMPDIR:-/tmp}/politika-core-tests.XXXXXXXX")"
trap 'rm -rf -- "$scratch"' EXIT

"$compiler" -std=c++20 -Wall -Wextra -Werror -pedantic \
  -I "$include_dir" "$source_file" -o "$scratch/politika-core-tests"
"$scratch/politika-core-tests"
echo "PASS: portable C++20 rules suite compiled (warnings as errors) and executed."
