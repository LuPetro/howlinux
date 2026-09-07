# xargs - build argument lists from input

`xargs` turns input into arguments for another command. For filenames, pair
null-delimited input with `-0` so spaces, quotes, and newlines are preserved.

## Examples

```bash
find . -type f -name '*.txt' -print0 | xargs -0 -r wc -l
```

## What to know

`-r` is a GNU option that prevents an invocation when input is empty. `-n 20`
limits arguments per invocation, which can change the meaning of commands with
totals. Default xargs parsing treats whitespace and quotes specially and is
unsuitable for arbitrary filenames. `find ... -exec wc -l {} +` is another
safe way to batch paths. Review matches before substituting a command that
modifies files.

## Reference

[Upstream manual](https://www.gnu.org/software/findutils/manual/html_mono/find.html)
