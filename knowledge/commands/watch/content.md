# watch - repeat a command on screen

`watch` reruns a command and displays the latest screen of output. Choose
read-only commands because every refresh repeats their effects.

## Examples

```bash
watch -n 2 free -h
watch -d -n 2 -- ls -l
```

## What to know

`-n` sets the interval in seconds and `-d` highlights differences. Ctrl+C
exits. Quote a pipeline when you want the shell invoked by watch to run the
whole pipeline. In procps, `-x` executes arguments directly without shell
parsing. Output is a display snapshot, not a durable log; use explicit logging
when you need history.

## Reference

[Upstream manual](https://man7.org/linux/man-pages/man1/watch.1.html)
