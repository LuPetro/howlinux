# Process filenames safely in shell loops

Filenames can contain spaces, tabs, newlines, wildcard characters, and leading
dashes. Line-based parsing and unquoted shell expansions can split one
filename into several arguments.

## Examples

```bash
while IFS= read -r -d '' path; do
    printf '%s\n' "$path"
done < <(find . -type f -print0)
```

## What to know

This example requires Bash. `find -print0` separates paths with NUL, and `read
-d ''` uses that delimiter. `IFS=` preserves whitespace and `-r` prevents
backslash interpretation. Always quote `"$path"` when passing it to a command,
and use `--` where supported. The printed display can still span lines when a
filename contains a newline. For batch actions, `find ... -exec COMMAND {} +`
may be simpler than a shell loop.

## Reference

[Upstream manual](https://www.gnu.org/software/bash/manual/bash.html)
