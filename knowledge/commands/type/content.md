# type - explain how the shell resolves a command

`type` is a shell builtin that explains whether a name resolves to an alias,
function, builtin, or executable.

## Examples

```bash
type cd
type -a printf
command -v ls
```

## What to know

In Bash, `type -a NAME` lists all known definitions and matching executable
paths. `command -v NAME` is useful for a simple availability check, but its
output can describe a builtin or alias rather than a filesystem path. The
current shell's PATH and definitions determine results. Use `hash -r` after
moving executables if Bash still remembers an old path.

## Reference

[Upstream manual](https://www.gnu.org/software/bash/manual/bash.html)
