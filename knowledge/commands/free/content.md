# free - inspect memory and swap usage

`free` summarizes RAM and swap statistics from the kernel. Use available
memory to estimate headroom for additional work.

## Examples

```bash
free -h
free -m
free -h -s 2
```

## What to know

`-h` uses readable units, `-m` uses mebibytes, and `-s 2` repeats every two
seconds until Ctrl+C. Cache is useful memory that may be reclaimable; low free
memory alone does not prove a shortage. Available memory is an estimate.
Containers may have separate cgroup limits, so host totals can exceed the
memory actually permitted to a workload.

## Reference

[Upstream manual](https://man7.org/linux/man-pages/man1/free.1.html)
