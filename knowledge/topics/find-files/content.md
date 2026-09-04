# Find files by name, type, or age

Start `find` at the narrowest useful directory so results are faster and
permission errors are limited.

```bash
find "DIRECTORY" -type f -name '*.txt'
find "DIRECTORY" -type d -iname '*cache*'
find "DIRECTORY" -type f -mtime -7 -print
```

`-type f` selects regular files and `-type d` selects directories. Quote the
name pattern so the shell does not expand it early. `-iname` is the GNU
case-insensitive form. `-mtime -7` means fewer than seven 24-hour periods ago,
not necessarily the current calendar week.

Print results before attaching a modifying action. If names must be passed to
another program, prefer `-exec PROGRAM {} +` over parsing newline-separated
output because Linux file names may contain spaces and newlines.
