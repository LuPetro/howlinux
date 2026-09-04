# less - view text one screen at a time

`less` is an interactive pager for reading text without loading all output into
the terminal at once.

```bash
less -- FILE
COMMAND | less
```

Inside `less`, press `Space` or `Page Down` to move forward, `b` or
`Page Up` to move backward, `/PATTERN` to search forward, `n` for the next
match, and `q` to quit. `G` jumps to the end and `g` to the beginning.

Use `less -R` when piped output contains ANSI color sequences that should be
displayed. Avoid the broader `-r` option for untrusted input because it passes
through additional control characters.
