# history - inspect Bash command history

`history` is a Bash builtin for inspecting the commands recorded by the
current interactive shell.

## Examples

```bash
history 20
history | grep -F -- 'PATTERN'
```

## What to know

Ctrl+R starts interactive reverse search; Ctrl+C cancels it. Edit and review a
recalled command before pressing Enter, especially commands that change files.
History storage depends on shell settings, session flushing, and concurrent
terminals. It is not an audit log. Avoid typing tokens or passwords into
command arguments because they can be recorded in history and visible to other
local inspection tools.

## Reference

[Upstream manual](https://www.gnu.org/software/bash/manual/bash.html)
