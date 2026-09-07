# nice - launch a command at lower CPU priority

`nice` starts a program with adjusted scheduling niceness. Higher niceness
generally means lower CPU priority under contention.

## Examples

```bash
nice
nice -n 10 sha256sum -- LARGE_FILE
```

## What to know

With no command, GNU nice prints the current niceness. `-n 10` adds 10 to the
inherited value rather than setting an absolute value. Ordinary users can
normally lower their own priority but cannot freely raise it. Niceness is not
a CPU usage limit and does not directly limit memory or disk I/O. Scheduling
policies and cgroups also affect results.

## Reference

[Upstream manual](https://www.gnu.org/software/coreutils/manual/coreutils.html)
