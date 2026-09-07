# cd - change the shell directory

`cd` is a shell builtin that changes the current shell's working directory.
Quote directory names containing spaces.

## Examples

```bash
cd -- "project notes"
cd ..
cd ~
cd -
```

## What to know

`..` means the parent directory, `~` expands to your home, and `cd -` returns
to the previous directory. A bare `cd` also goes home. `cd /` goes to the
filesystem root. In a script, use `cd -- "$DIRECTORY" || exit 1` before
operations that depend on being in the right place.

## Reference

[Upstream manual](https://www.gnu.org/software/bash/manual/bash.html)
