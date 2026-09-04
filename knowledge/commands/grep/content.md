# grep - search text for patterns

`grep` prints lines that match a pattern.

```bash
grep -n -- 'PATTERN' FILE
grep -i -n -- 'pattern' FILE
grep -RIn -- 'PATTERN' DIRECTORY
```

`-n` includes line numbers, `-i` ignores case, and `-R` searches recursively
while following symbolic links named on the command line. Use `-r` instead for
a recursive search that does not follow those links.

Basic regular expressions are enabled by default. Use `-F` when the pattern
must be treated as literal text, or `-E` for extended regular expressions.
Quote the pattern to protect characters such as `*`, `$`, and spaces from the
shell. A normal no-match result uses exit status 1; it is not the same as an
execution error.
