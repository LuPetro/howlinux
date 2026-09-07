# Back up a directory and verify restoration

Use a destination on separate storage when protecting against disk failure. A
copy on the same disk protects against some mistakes but not loss of that
disk.

## Examples

```bash
rsync -avhn -- SOURCE/ BACKUP_DIRECTORY/
```

## What to know

Check the preview and the mounted destination. **Warning:** a real sync may
overwrite existing backup files, so keep older versions or snapshots before
updating the backup.

```bash
rsync -avh -- SOURCE/ BACKUP_DIRECTORY/
rsync -avhnc -- SOURCE/ BACKUP_DIRECTORY/
```

The final checksum dry run can reveal content differences but reads both sides
and can be slow. Stop applications or use an application-consistent snapshot
for actively changing data. Archive mode omits ACLs, extended attributes, and
hard-link preservation. Test a restoration into a separate empty directory; a
successful transfer is not a full recovery test.

## Reference

[Upstream manual](https://rsync.samba.org/ftp/rsync/rsync.1)
