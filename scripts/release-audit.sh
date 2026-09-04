#!/usr/bin/env bash
set -euo pipefail

project_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
audit_dir="$(mktemp -d)"

cleanup() {
  rm -rf -- "$audit_dir"
}
trap cleanup EXIT

debug_dir="${audit_dir}/debug"
release_dir="${audit_dir}/release"
install_prefix="${audit_dir}/installed"
package_prefix="${audit_dir}/package"

bash -n "${project_dir}/scripts/install.sh" \
  "${project_dir}/scripts/evaluate-ranking.sh" \
  "${project_dir}/scripts/release-audit.sh" \
  "${project_dir}/tests/smoke.sh"

cmake -S "$project_dir" -B "$debug_dir" -DCMAKE_BUILD_TYPE=Debug
cmake --build "$debug_dir" --parallel
ctest --test-dir "$debug_dir" --output-on-failure
"${debug_dir}/howlinux" validate "${project_dir}/knowledge"

cmake -S "$project_dir" -B "$release_dir" -DCMAKE_BUILD_TYPE=Release
cmake --build "$release_dir" --parallel
ctest --test-dir "$release_dir" --output-on-failure
"${release_dir}/howlinux" validate "${project_dir}/knowledge"

test "$("${release_dir}/howlinux" --version)" = "howlinux 1.1.0"
"${project_dir}/scripts/evaluate-ranking.sh" \
  "${release_dir}/howlinux" "${project_dir}/knowledge"
if ldd "${release_dir}/howlinux" | grep -q libyaml-cpp; then
  echo "Release binary must not depend on a shared yaml-cpp ABI." >&2
  exit 1
fi

cmake --install "$release_dir" --prefix "$install_prefix"
(
  cd "$audit_dir"
  "${install_prefix}/bin/howlinux" validate
)

cmake --build "$release_dir" --target package
archive="$(find "$release_dir" -maxdepth 1 -name 'howlinux-1.1.0-*.tar.gz' -print -quit)"
if [[ -z "$archive" ]]; then
  echo "Release archive was not created." >&2
  exit 1
fi

tar -tzf "$archive" >"${audit_dir}/archive-files"
grep -qx 'bin/howlinux' "${audit_dir}/archive-files"
grep -qx 'share/howlinux/knowledge/concepts.yaml' "${audit_dir}/archive-files"
grep -qx 'share/doc/howlinux/LICENSE' "${audit_dir}/archive-files"
grep -qx 'share/doc/howlinux/THIRD_PARTY_NOTICES.md' "${audit_dir}/archive-files"
grep -qx 'share/doc/howlinux/CHANGELOG.md' "${audit_dir}/archive-files"
grep -qx 'share/doc/howlinux/docs/releases/v1.1.0.md' "${audit_dir}/archive-files"

mkdir "$package_prefix"
tar -xzf "$archive" -C "$package_prefix"
(
  cd "$audit_dir"
  "${package_prefix}/bin/howlinux" validate
)

sha256sum "$archive"
echo "Release audit passed."
