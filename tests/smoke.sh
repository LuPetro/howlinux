#!/usr/bin/env bash
set -euo pipefail

binary="$1"
knowledge="$2"
smoke_dir="$(mktemp -d)"
stdout_file="${smoke_dir}/stdout"
stderr_file="${smoke_dir}/stderr"
trap 'rm -rf -- "$smoke_dir"' EXIT

expect_success() {
  "$@" >"$stdout_file" 2>"$stderr_file"
}

expect_code() {
  local expected="$1"
  shift
  set +e
  "$@" >"$stdout_file" 2>"$stderr_file"
  local actual=$?
  set -e
  if [[ "$actual" -ne "$expected" ]]; then
    echo "Expected exit code $expected, got $actual: $*" >&2
    cat "$stderr_file" >&2 || true
    exit 1
  fi
}

expect_success "$binary" --knowledge "$knowledge" list
grep -q 'Knowledge entries: 38' "$stdout_file"

expect_success "$binary" --knowledge "$knowledge" validate
grep -q 'Validation status: valid' "$stdout_file"

expect_success "$binary" --knowledge "$knowledge" rename folder
grep -q 'Rename a folder' "$stdout_file"

expect_success "$binary" --knowledge "$knowledge" --explain 'what does chmod 755 mean'
grep -q 'Ranking explanation' "$stdout_file"

expect_success "$binary" --knowledge "$knowledge" --json 'extract tar.gz'
grep -q '"status":"confident"' "$stdout_file"

expect_success "$binary" --knowledge "$knowledge" show rename-folder
grep -q 'mv -i -- OLD_NAME NEW_NAME' "$stdout_file"

expect_success "$binary" --knowledge "$knowledge" 'copy a folder recursively'
grep -q 'Copy files and directories safely' "$stdout_file"

expect_success "$binary" --knowledge "$knowledge" 'filesystem is full'
grep -q 'Investigate disk space usage' "$stdout_file"

expect_success "$binary" --knowledge "$knowledge" 'follow service logs'
grep -q 'Read and follow Linux logs' "$stdout_file"

expect_code 1 "$binary" --knowledge "$knowledge" 'unknown thing'
expect_code 2 "$binary" --definitely-invalid

echo 'howlinux smoke tests passed'
