# readlink - inspect symbolic link targets

`readlink` prints the target stored in a symbolic link. That target can be
relative to the directory containing the link.

## Examples

```bash
readlink -- LINK
readlink -f -- LINK
readlink -e -- LINK
```

## What to know

`-f` canonicalizes the path and requires all but its last component to exist.
`-e` requires every component to exist. An ordinary file is not a symlink, so
plain `readlink FILE` normally fails. Canonicalizing a path does not lock it
or prevent another process changing the link afterward.

## Reference

[Upstream manual](https://www.gnu.org/software/coreutils/manual/coreutils.html)
