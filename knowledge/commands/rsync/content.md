# rsync - synchronize files with a preview

`rsync` transfers differences between source and destination. Begin with a dry
run; the trailing slash on the source controls whether you copy the directory
itself or its contents.

## Examples

```bash
rsync -avhn -- SOURCE/ DESTINATION/
```

## What to know

`-a` preserves common attributes and copies recursively; it does not include
hard links, ACLs, or extended attributes. `-n` previews, `-v` explains, and
`-h` formats sizes. **Warning:** the real run may replace destination files.
After reviewing the preview and keeping a separate backup, run:

```bash
rsync -avh -- SOURCE/ DESTINATION/
```

These examples do not delete extra destination files. Never add `--delete`
without reviewing exactly which files would be removed.

## Reference

[Upstream manual](https://rsync.samba.org/ftp/rsync/rsync.1)
