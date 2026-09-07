# tee - display output and save a copy

`tee` copies standard input to standard output and one or more files.
**Warning:** normal mode replaces an existing destination file; `-a` appends
to it.

## Examples

```bash
printf '%s\n' 'build started' | tee -a build.log
```

## What to know

Only standard output reaches the pipe by default. `COMMAND 2>&1 | tee -a
run.log` also captures standard error. In Bash, enable `set -o pipefail` if
the pipeline should report a failure in an earlier command. Avoid storing
secrets in logs. With `sudo tee`, the file writer has elevated privileges; use
it only when the destination actually requires them.

## Reference

[Upstream manual](https://www.gnu.org/software/coreutils/manual/coreutils.html)
