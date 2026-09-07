# awk - process text fields and records

`awk` runs a small program for each input record. The default record is a line
and default fields are separated by whitespace.

## Examples

```bash
awk '{print $1, $3}' FILE
awk -F ':' '{print $1}' /etc/passwd
awk '{total += $2} END {print total}' FILE
awk '$2 > 100 {print $0}' FILE
```

## What to know

`$0` is the whole line, `$1` the first field, `NF` the field count, and `NR`
the record number. Single-quote the program so the shell does not expand `$1`.
Use `awk -v limit=100 '$2 > limit' FILE` for a parameter. Simple `-F ,`
splitting cannot correctly parse CSV with quoted commas or multiline fields.

## Reference

[Upstream manual](https://www.gnu.org/software/gawk/manual/gawk.html)
