# wc - count lines words and bytes

`wc` counts newlines, words, and bytes in text. Pass file paths or send text
through standard input.

## Examples

```bash
wc -- FILE
wc -l -- FILE
wc -w -- FILE
wc -m -- FILE
```

## What to know

`-l` counts newline characters, so an unterminated final line is not included.
`-w` counts whitespace-delimited words, `-c` counts bytes, and `-m` counts
characters according to the locale. Unicode characters can occupy multiple
bytes. Use `wc -l < FILE` to omit the filename in output.

## Reference

[Upstream manual](https://www.gnu.org/software/coreutils/manual/coreutils.html)
