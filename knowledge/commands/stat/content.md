# stat - inspect file metadata

`stat` reports size, permissions, owner, inode, and timestamps without
displaying file contents.

## Examples

```bash
stat -- FILE
stat -c '%n %s bytes %a %U:%G' -- FILE
stat -L -- LINK
```

## What to know

On GNU systems, `-c` selects an output format and `-L` inspects the symlink
target. Modification time concerns contents; change time concerns inode
metadata and is not creation time. Birth time may be unavailable. Apparent
byte size can differ from allocated storage, especially for sparse files;
compare `du` for actual allocation.

## Reference

[Upstream manual](https://www.gnu.org/software/coreutils/manual/coreutils.html)
