# pgrep - find process IDs by name

`pgrep` searches running processes and prints matching IDs. Add display
options to inspect what matched before taking action.

## Examples

```bash
pgrep -a -x sshd
pgrep -af -- PATTERN
pgrep -u USER -a
```

## What to know

`-x` requires an exact name match, `-a` displays the full command, and `-f`
searches the full command line. Patterns are regular expressions. The default
Linux process-name match is limited by the kernel name field, commonly 15
characters. Exit status 1 means no processes matched. Verify current details
with `ps` before sending a signal; process IDs can be reused.

## Reference

[Upstream manual](https://man7.org/linux/man-pages/man1/pgrep.1.html)
