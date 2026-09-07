# tr - translate or remove characters

`tr` transforms standard input and writes standard output. It accepts
character sets, not input filenames.

## Examples

```bash
printf '%s\n' 'hello' | tr '[:lower:]' '[:upper:]'
printf '%s\n' 'a    b' | tr -s ' '
tr -d '\r' < DOS_FILE
```

## What to know

`-s` squeezes repeated selected characters and `-d` deletes them. Removing
`\r` deletes every carriage return, not only CRLF endings. Character classes
depend on locale and implementation; this is not a general Unicode
case-folding tool. Always write transformations to a different file before
replacing an original.

## Reference

[Upstream manual](https://www.gnu.org/software/coreutils/manual/coreutils.html)
