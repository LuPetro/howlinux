# Diagnose exhausted filesystem inodes

A filesystem can run out of inodes even while bytes remain free. Each file or
directory needs filesystem metadata, and many small files can exhaust that
capacity.

## Examples

```bash
df -h -- .
df -i -- .
du --inodes --max-depth=1 -- .
```

## What to know

`df -i` shows inode totals where supported. GNU `du --inodes` estimates inode
use below the current directory; permissions can hide parts of the tree. Some
filesystems allocate metadata dynamically and do not report traditional inode
limits. Inspect application caches, queues, and temporary-file producers
before deleting anything. Quotas and reserved space are other reasons writes
can fail while a general disk summary looks healthy.

## Reference

[Upstream manual](https://www.gnu.org/software/coreutils/manual/coreutils.html)
