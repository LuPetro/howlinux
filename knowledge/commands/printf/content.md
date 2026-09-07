# printf - print formatted text reliably

`printf` prints a format string followed by formatted arguments. A fixed
format keeps data containing percent signs from being interpreted as
instructions.

## Examples

```bash
printf '%s\n' 'hello world'
printf '%s: %d\n' 'count' 3
printf '%s' 'no final newline'
```

## What to know

Use `%s` for text, `%d` for integers, and `\n` in the format for a newline.
Prefer `printf '%s\n' "$VALUE"` over using a variable as the format. Unlike
many `echo` variants, this makes newline and backslash behavior explicit. Bash
includes a builtin `printf`.

## Reference

[Upstream manual](https://www.gnu.org/software/bash/manual/bash.html)
