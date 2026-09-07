# env - inspect or change a child environment

`env` can run a program with modified environment variables without changing
the parent shell.

## Examples

```bash
env LANG=C date
env -u HTTP_PROXY printenv HTTP_PROXY
env -i PATH=/usr/bin:/bin /usr/bin/env
```

## What to know

`NAME=value` sets a variable for the child; `-u NAME` removes one, and `-i`
starts with an empty environment. The second example normally exits nonzero
because the removed variable is absent. Running bare `env` displays the entire
environment, which can contain secrets. An empty environment is useful for
diagnostics but is not a security sandbox.

## Reference

[Upstream manual](https://www.gnu.org/software/coreutils/manual/coreutils.html)
