# export - pass shell variables to child processes

`export` marks a Bash variable for inclusion in the environment of
subsequently launched programs.

## Examples

```bash
export APP_MODE=development
printenv APP_MODE
unset APP_MODE
```

## What to know

There must be no spaces around `=`. Quote values containing spaces: `export
APP_LABEL="local test"`. A plain shell assignment is not necessarily inherited
by child processes. This change lasts only for the current shell and its
descendants; it does not change other terminals or persist across logins.
Never store real secrets in shared shell history or examples.

## Reference

[Upstream manual](https://www.gnu.org/software/bash/manual/bash.html)
