# timeout - limit how long a command runs

`timeout` starts a command and sends a signal when the duration expires.
**Warning:** termination can interrupt writes or other state changes; use this
only where interruption is acceptable.

## Examples

```bash
timeout 5s sleep 20
timeout --kill-after=2s 5s sleep 20
```

## What to know

The default timeout signal is TERM. `--kill-after=2s` escalates to KILL if the
process remains after the initial signal. Exit status 124 usually indicates
the duration expired; 137 can indicate KILL. Other command failures retain
their own status in normal cases. A timeout is not a rollback and cannot
guarantee that partial work is undone.

## Reference

[Upstream manual](https://www.gnu.org/software/coreutils/manual/coreutils.html)
