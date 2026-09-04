# Search for text inside files

Use `grep` with fixed-string mode when the search text should be interpreted
literally:

```bash
grep -nF -- 'TEXT' FILE
grep -RInF -- 'TEXT' DIRECTORY
```

`-n` prints line numbers, `-F` disables regular-expression syntax, `-i`
ignores letter case, and `-R` searches directories recursively. Remove `-i`
when case matters.

For a regular expression, omit `-F` or use `-E` for extended syntax:

```bash
grep -REn -- 'error|failed' DIRECTORY
```

Quote the pattern. Exit status 0 means a match, 1 means no match, and values
greater than 1 indicate an error; scripts should distinguish those outcomes.
