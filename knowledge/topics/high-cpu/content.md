# Investigate high CPU usage and load

Observe the system over several samples. High load can include tasks waiting
for I/O, while high CPU utilization indicates time spent executing work.

## Examples

```bash
uptime
top
ps -eo pid,ppid,comm,pcpu --sort=-pcpu | head -n 11
```

## What to know

`top` provides interval-based samples; procps `ps` CPU percentage is generally
a lifetime average and can tell a different story. Check the command and its
parent before changing it. Multi-threaded processes can use more than one CPU,
and a container can have CPU quotas smaller than the host count. An expected
batch job may simply need time; lower its scheduling priority on a future run
with nice if appropriate. Terminate only identified work with an understood
recovery path.

## Reference

[Upstream manual](https://man7.org/linux/man-pages/man1/top.1.html)
