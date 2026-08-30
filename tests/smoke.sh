#!/usr/bin/env bash
set -euo pipefail

binary="$1"
knowledge="$2"

expect_success() {
  "$@" >/tmp/howlinux-smoke-output 2>/tmp/howlinux-smoke-error
}

expect_code() {
  local expected="$1"
  shift
  set +e
  "$@" >/tmp/howlinux-smoke-output 2>/tmp/howlinux-smoke-error
  local actual=$?
  set -e
  if [[ "$actual" -ne "$expected" ]]; then
    echo "Expected exit code $expected, got $actual: $*" >&2
    cat /tmp/howlinux-smoke-error >&2 || true
    exit 1
  fi
}

expect_success "$binary" --knowledge "$knowledge" list
grep -q 'Knowledge entries: 5' /tmp/howlinux-smoke-output

expect_success "$binary" --knowledge "$knowledge" validate
grep -q 'Validation status: valid' /tmp/howlinux-smoke-output

expect_success "$binary" --knowledge "$knowledge" rename folder
grep -q 'Rename a folder' /tmp/howlinux-smoke-output

expect_success "$binary" --knowledge "$knowledge" --explain 'what does chmod 755 mean'
grep -q 'Ranking explanation' /tmp/howlinux-smoke-output

expect_success "$binary" --knowledge "$knowledge" --json 'extract tar.gz'
grep -q '"status":"confident"' /tmp/howlinux-smoke-output

expect_success "$binary" --knowledge "$knowledge" show rename-folder
grep -q 'mv OLD_NAME NEW_NAME' /tmp/howlinux-smoke-output

expect_code 1 "$binary" --knowledge "$knowledge" 'unknown thing'
expect_code 2 "$binary" --definitely-invalid

echo 'howlinux smoke tests passed'
