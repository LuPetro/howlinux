# Understand PATH and persistent environment settings

`PATH` is an ordered, colon-separated list of directories searched for
executable names that do not include a slash.

## Examples

```bash
printf '%s\n' "$PATH"
command -v howlinux
export PATH="$HOME/.local/bin:$PATH"
```

## What to know

The export affects the current shell and new child processes. To persist it,
edit the startup file appropriate to your shell and session: Bash interactive
non-login shells read `~/.bashrc`, while login shells read the first available
user login profile. Avoid repeatedly appending duplicate PATH edits. Empty
PATH components and `.` allow current-directory command lookup, which can run
an unexpected program. A slash in `./program` bypasses PATH lookup.

## Reference

[Upstream manual](https://www.gnu.org/software/bash/manual/bash.html)
