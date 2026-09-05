#!/usr/bin/env bash
set -euo pipefail

# Exercise discovery outside the checkout and relocation of CPack archives.
build_dir="$(cd -- "$1" && pwd)"
smoke_dir="$(mktemp -d)"
trap 'rm -rf -- "$smoke_dir"' EXIT
unset HOWLINUX_KNOWLEDGE

cmake --install "$build_dir" --prefix "$smoke_dir/installed prefix"
(
  cd "$smoke_dir"
  './installed prefix/bin/howlinux' validate
  './installed prefix/bin/howlinux' 'install arch linux package'
)

cmake --build "$build_dir" --target package
archives=("$build_dir"/howlinux-*.tar.gz)
test "${#archives[@]}" -eq 1
test -f "${archives[0]}"
mkdir "$smoke_dir/package"
tar -xzf "${archives[0]}" -C "$smoke_dir/package"
test -f "$smoke_dir/package/share/bash-completion/completions/howlinux"
test -f "$smoke_dir/package/share/zsh/site-functions/_howlinux.zsh"
test -f "$smoke_dir/package/share/fish/vendor_completions.d/howlinux.fish"
test -f "$smoke_dir/package/share/man/man1/howlinux.1"
(
  cd "$smoke_dir"
  ./package/bin/howlinux validate
  ./package/bin/howlinux show pacman
)

echo 'Install and package smoke tests passed'
