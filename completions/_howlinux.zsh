#compdef howlinux

_howlinux() {
  _arguments \
    '(-h --help)'{-h,--help}'[show help]' \
    '(-V --version)'{-V,--version}'[show version]' \
    '--knowledge=[knowledge directory]:directory:_directories' \
    '--limit=[maximum results]:number:' \
    '--explain[show ranking explanation]' \
    '--json[emit JSON]' \
    '1:command:(search list show validate)' \
    '*:query:_message "query"'
}

_howlinux "$@"
