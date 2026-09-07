# Investigate high memory usage

Linux uses spare RAM for caches, so a high used value alone does not mean a
problem. Start with available memory and the processes consuming resident
memory.

## Examples

```bash
free -h
ps -eo pid,comm,rss --sort=-rss | head -n 11
top
```

## What to know

`ps` RSS is normally reported in KiB. Shared pages can be counted in more than
one process, so adding RSS values is not a precise total. Swap usage can
remain after past pressure and does not alone establish current thrashing.
Look for workload growth, application limits, and cgroup limits in containers.
Kernel journal messages may record out-of-memory kills. Do not clear caches or
disable swap as a first response; those actions can reduce performance or make
failures worse.

## Reference

[Upstream manual](https://man7.org/linux/man-pages/man1/free.1.html)
