#!/usr/bin/env bash
set -euo pipefail

project_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
prefix="${HOWLINUX_PREFIX:-${HOME}/.local}"
build_dir="${HOWLINUX_BUILD_DIR:-${project_dir}/build-install}"

if [[ "$prefix" != /* ]]; then
  echo "HOWLINUX_PREFIX must be an absolute path: $prefix" >&2
  exit 2
fi

for tool in cmake c++; do
  command -v "$tool" >/dev/null 2>&1 || {
    echo "Missing required tool: $tool" >&2
    echo "Ubuntu/Debian: sudo apt install build-essential cmake libyaml-cpp-dev" >&2
    echo "Arch Linux: sudo pacman -Syu --needed base-devel cmake yaml-cpp" >&2
    exit 1
  }
done

if ! cmake --find-package -DNAME=yaml-cpp -DCOMPILER_ID=GNU -DLANGUAGE=CXX -DMODE=EXIST >/dev/null 2>&1; then
  echo "Could not find yaml-cpp development files." >&2
  echo "Ubuntu/Debian: sudo apt install libyaml-cpp-dev" >&2
  echo "Arch Linux: sudo pacman -Syu --needed yaml-cpp" >&2
  exit 1
fi

cmake -S "$project_dir" -B "$build_dir" -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$prefix" \
  -DHOWLINUX_STATIC_YAML_CPP="${HOWLINUX_STATIC_YAML_CPP:-AUTO}"
cmake --build "$build_dir" --parallel
ctest --test-dir "$build_dir" --output-on-failure
cmake --install "$build_dir"

installed_binary="${prefix}/bin/howlinux"
if [[ ! -x "$installed_binary" ]]; then
  echo "Installation failed: $installed_binary is not executable." >&2
  exit 1
fi
"$installed_binary" validate >/dev/null

echo "Installed and validated howlinux at $installed_binary"
if [[ ":${PATH}:" != *":${prefix}/bin:"* ]]; then
  echo "The install directory is not in PATH for this shell."
  echo "Run: export PATH=\"${prefix}/bin:\$PATH\""
fi
