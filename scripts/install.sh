#!/usr/bin/env bash
set -euo pipefail

project_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
prefix="${HOWLINUX_PREFIX:-${HOME}/.local}"
build_dir="${HOWLINUX_BUILD_DIR:-${project_dir}/build-install}"

for tool in cmake c++; do
  command -v "$tool" >/dev/null 2>&1 || {
    echo "Missing required tool: $tool" >&2
    echo "Ubuntu/Debian: sudo apt install build-essential cmake libyaml-cpp-dev" >&2
    exit 1
  }
done

if ! cmake --find-package -DNAME=yaml-cpp -DCOMPILER_ID=GNU -DLANGUAGE=CXX -DMODE=EXIST >/dev/null 2>&1; then
  echo "Could not find yaml-cpp development files." >&2
  echo "Ubuntu/Debian: sudo apt install libyaml-cpp-dev" >&2
  exit 1
fi

cmake -S "$project_dir" -B "$build_dir" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$prefix"
cmake --build "$build_dir" --parallel
ctest --test-dir "$build_dir" --output-on-failure
cmake --install "$build_dir"

echo "Installed howlinux to $prefix/bin/howlinux"
if [[ ":${PATH}:" != *":${prefix}/bin:"* ]]; then
  echo "Add ${prefix}/bin to PATH if the command is not found."
fi
