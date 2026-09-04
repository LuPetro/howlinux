# Investigate disk space usage

First identify which mounted filesystem is full:

```bash
df -h -- "PATH"
df -i -- "PATH"
```

The first command checks byte capacity; the second checks inode capacity. Then
measure a relevant directory tree:

```bash
du -h --max-depth=1 -- "DIRECTORY"
du -sh -- "DIRECTORY"
```

Start with a narrow directory and widen only as needed. Permission errors make
the result incomplete. `df` and `du` can disagree because deleted files may
still be open, and because of reserved space, snapshots, sparse files, hard
links, or filesystem metadata.

Do not delete unfamiliar system files merely because they are large. Identify
their owner and purpose, check application-specific cleanup procedures, and
back up important data before removal.
