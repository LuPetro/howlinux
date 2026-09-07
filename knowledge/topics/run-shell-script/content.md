# Inspect and run a Bash script

Read a script before executing it. A script can change files, run other
programs, or access the network with your account's permissions.

## Examples

```bash
less -- SCRIPT.sh
bash -n -- SCRIPT.sh
```

## What to know

`bash -n` checks Bash syntax without executing the script; it does not prove
the logic is correct or safe. **Warning:** the following runs all commands in
the script. Only continue after reviewing and trusting it.

```bash
bash -- SCRIPT.sh
```

For direct execution, use a suitable shebang such as `#!/usr/bin/env bash`,
add the owner executable bit with `chmod u+x SCRIPT.sh`, and run
`./SCRIPT.sh`. Using `sh SCRIPT.sh` can fail on Bash-specific syntax. Sourcing
a script with `.` also changes your current shell environment.

## Reference

[Upstream manual](https://www.gnu.org/software/bash/manual/bash.html)
