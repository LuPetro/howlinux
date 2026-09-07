# pwd - print the current directory

`pwd` shows the directory used to resolve relative paths. It does not list the
files inside that directory.

## Examples

```bash
pwd
pwd -P
```

## What to know

`pwd -P` resolves symbolic links in the directory path. Bash normally uses the
logical path, which may retain a symlink name. Use `ls` to inspect the
contents and `cd` to move elsewhere. A path starting with `/` is absolute;
`./notes.txt` is relative to the current directory.

## Reference

[Upstream manual](https://www.gnu.org/software/coreutils/manual/coreutils.html)
