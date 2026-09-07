# uniq - report adjacent duplicate lines

`uniq` compares adjacent lines. Sort first when equal lines can appear
anywhere in the input.

## Examples

```bash
uniq -- FILE
sort -- FILE | uniq -c
sort -- FILE | uniq -d
```

## What to know

`-c` prefixes each group with its count, `-d` shows repeated groups, and `-u`
shows groups occurring once. Sorting changes the original order. For
case-insensitive comparison use `sort -f FILE | uniq -i`. These are line
operations, not a parser for quoted CSV records.

## Reference

[Upstream manual](https://www.gnu.org/software/coreutils/manual/coreutils.html)
