# Understand exit status and conditional commands

A shell command returns a numeric status: zero conventionally means success
and nonzero describes another result. The exact meaning belongs to the
command.

## Examples

```bash
test -d . && printf '%s\n' 'directory exists'
test -f MISSING_FILE || printf '%s\n' 'file not found'
false
result=$?
printf 'saved status: %s\n' "$result"
```

## What to know

`$?` holds the status of the immediately previous command; save it before
running anything else. `&&` runs the next command after success, while `||`
runs it after a nonzero status. `grep` uses 1 for no match and `diff` uses 1
for a difference, so these results are not necessarily operational errors.
Avoid treating `set -e` as complete error handling; its behavior depends on
shell context.

## Reference

[Upstream manual](https://www.gnu.org/software/bash/manual/bash.html)
