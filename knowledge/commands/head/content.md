# head - show the beginning of files

`head` prints the beginning of each input file. The default is ten lines.

```bash
head -- FILE
head -n 20 -- FILE
head -c 100 -- FILE
```

`-n` chooses a line count and `-c` chooses a byte count. When several files
are supplied, headers identify their output. Use `head -q` with GNU coreutils
to suppress those headers.

`head` also accepts piped input, for example `COMMAND | head -n 20`. Some
producers may report a broken-pipe status after `head` deliberately stops
reading; account for that when a script enables strict pipeline status checks.
