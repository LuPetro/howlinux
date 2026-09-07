# top - monitor processes interactively

`top` refreshes system and process statistics. Start with observation before
changing or terminating a process.

## Examples

```bash
top
top -p PID
top -b -n 1
```

## What to know

Press `q` to quit, `P` to sort by CPU, and `M` to sort by memory in common
procps configurations. `-b -n 1` prints a single batch sample. RES is resident
memory, while VIRT includes virtual address space and is not actual RAM use.
CPU percentages depend on display mode and available CPUs; a multi-threaded
process can exceed 100%.

## Reference

[Upstream manual](https://man7.org/linux/man-pages/man1/top.1.html)
