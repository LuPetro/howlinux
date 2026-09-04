# ls - list directory contents

`ls` displays files and directories. With no path it lists the current
directory; one or more paths can be supplied explicitly.

## Common forms

```bash
ls -- DIRECTORY
ls -l -- DIRECTORY
ls -la -- DIRECTORY
ls -lh -- DIRECTORY
```

`-l` uses the long format, `-a` includes names beginning with `.`, and `-h`
makes sizes easier to read when combined with `-l`. Use `--` before a path
that could begin with a dash.

`ls` output is intended for people and can vary with options, aliases, locale,
and terminal settings. Scripts should normally use tools designed for machine
processing, such as `find`, instead of parsing `ls` output.
