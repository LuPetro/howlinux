# lsblk - list block devices and filesystems

`lsblk` inspects block devices and their relationships. It does not partition
or format them.

## Examples

```bash
lsblk
lsblk -f
lsblk -o NAME,SIZE,TYPE,FSTYPE,MOUNTPOINTS
```

## What to know

`-f` adds filesystem metadata such as labels and UUIDs. A disk can contain
partitions that in turn back encryption or logical volumes, so read the tree
structure before identifying a device. Mount points can be multiple. Older
util-linux releases may only provide the `MOUNTPOINT` column. Never infer a
safe formatting target from a device name alone.

## Reference

[Upstream manual](https://man7.org/linux/man-pages/man8/lsblk.8.html)
