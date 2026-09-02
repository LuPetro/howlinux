#!/usr/bin/env bash

_howlinux_complete() {
    local current previous
    current="${COMP_WORDS[COMP_CWORD]}"
    previous="${COMP_WORDS[COMP_CWORD-1]}"

    if [[ "$previous" == "show" ]]; then
        local entries
        entries="$(howlinux list --json 2>/dev/null | sed -n 's/.*\"id\":\"\([^\"]*\)\".*/\1/p')"
        COMPREPLY=( $(compgen -W "$entries" -- "$current") )
        return
    fi

    if [[ "$previous" == "--knowledge" ]]; then
        COMPREPLY=( $(compgen -d -- "$current") )
        return
    fi

    COMPREPLY=( $(compgen -W '--help --version --knowledge --limit --explain --json search list show validate' -- "$current") )
}

complete -F _howlinux_complete howlinux
