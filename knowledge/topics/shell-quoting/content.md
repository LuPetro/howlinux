# Quote shell arguments and filenames safely

Bash first interprets shell syntax, expands variables, and builds the argument
list. Quoting keeps data such as filenames with spaces together.

## Examples

```bash
FILE_NAME='project notes.txt'
printf '%s\n' "$FILE_NAME"
printf '%s\n' '$FILE_NAME'
ls -l -- "$FILE_NAME"
```

## What to know

Double quotes allow variable and command substitution while preventing
ordinary word splitting and wildcard expansion. Single quotes preserve their
contents literally. The first printf prints the value; the second prints the
dollar sign and variable name. `--` ends option parsing for commands that
support it. Quote variable expansions as `"$VALUE"`; do not add quotes around
a whole multi-argument command. Never use `eval` just to handle spaces in a
path.

## Reference

[Upstream manual](https://www.gnu.org/software/bash/manual/bash.html)
