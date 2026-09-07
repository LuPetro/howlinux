# sed - transform text with a stream editor

`sed` reads lines, applies editing commands, and prints the result. The
examples preview changes without modifying the input.

## Examples

```bash
sed 's/old/new/g' FILE
sed -n '10,20p' FILE
sed '/^[[:space:]]*$/d' FILE
```

## What to know

`s/old/new/g` replaces every match on each line; without `g`, only the first
match changes. The search expression is a regular expression. In replacement
text `&` means the entire match, so escape it for a literal ampersand. GNU
`sed -i.bak` edits in place and creates a backup, but review stdout first and
avoid repeated edits that overwrite your backup.

## Reference

[Upstream manual](https://www.gnu.org/software/sed/manual/sed.html)
