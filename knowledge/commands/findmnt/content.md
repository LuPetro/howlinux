# findmnt - inspect mounted filesystems

`findmnt` queries the mount table and can identify the filesystem covering a
particular path.

## Examples

```bash
findmnt
findmnt --target /
findmnt -o TARGET,SOURCE,FSTYPE,OPTIONS
```

## What to know

`--target PATH` finds the filesystem containing that path, even when it is not
itself a mount point. `SOURCE` can be a device, virtual filesystem, or network
location. The mount namespace of a container can differ from the host. These
examples inspect active mounts and do not change `/etc/fstab` or mount
anything.

## Reference

[Upstream manual](https://man7.org/linux/man-pages/man8/findmnt.8.html)
