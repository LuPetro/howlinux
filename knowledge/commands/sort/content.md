# sort - order lines of text

`sort` writes sorted input to standard output without editing the input file.

## Examples

```bash
sort -- FILE
sort -n -- NUMBERS_FILE
sort -r -- FILE
LC_ALL=C sort -u -- FILE
```

## What to know

`-n` compares numbers, `-r` reverses ordering, and `-u` keeps one
representative for each equal sort key. The locale affects text ordering;
`LC_ALL=C` makes byte ordering predictable. For whitespace-separated columns
use `sort -k2,2n FILE`. Do not redirect output onto the input file: shell
redirection truncates it before sort reads it.

## Reference

[Upstream manual](https://www.gnu.org/software/coreutils/manual/coreutils.html)
