# uptime - inspect runtime and system load

`uptime` shows elapsed time since boot and recent load averages.

## Examples

```bash
uptime
uptime -p
uptime -s
```

## What to know

The three load averages cover roughly 1, 5, and 15 minutes. On Linux they
include runnable tasks and tasks in uninterruptible sleep, so load is not a
CPU utilization percentage. Interpret the values alongside CPU count, `top`,
and storage behavior. `-p` prints a readable duration; `-s` prints the boot
timestamp.

## Reference

[Upstream manual](https://man7.org/linux/man-pages/man1/uptime.1.html)
