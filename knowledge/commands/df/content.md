# df - report filesystem space

`df` reports used and available space for mounted filesystems.

```bash
df -h
df -h -- PATH
df -i -- PATH
```

`-h` uses human-readable size units. When a path is supplied, `df` reports the
filesystem containing that path. `-i` reports inode usage instead of data
blocks; a filesystem can run out of inodes even when byte capacity remains.

Values can differ from a simple sum of visible files because of reserved
blocks, filesystem metadata, snapshots, sparse files, and deleted files still
held open by processes. Use `du` to estimate usage within particular directory
trees.
