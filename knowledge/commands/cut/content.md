# cut - select fields or characters

`cut` selects parts of each input line. It works well with simple delimiters
and does not understand quoted CSV fields.

## Examples

```bash
cut -d ':' -f 1 /etc/passwd
cut -f 1,3 -- FILE
cut -c 1-10 -- FILE
```

## What to know

The default field delimiter is a tab. `-d` changes it and `-f` selects
one-based fields or ranges. `-s` suppresses lines with no delimiter.
Consecutive spaces count as separate delimiters, so use `awk` for
variable-width whitespace. Character selection on multibyte text depends on
the implementation; avoid assuming display-column behavior.

## Reference

[Upstream manual](https://www.gnu.org/software/coreutils/manual/coreutils.html)
