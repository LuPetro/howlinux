# Create and inspect a project archive

Create the archive outside the source directory and choose an unused output
filename. Inspect source files for secrets and unwanted build artifacts before
sharing.

## Examples

**Warning:** archive creation replaces NEW_ARCHIVE.tar.gz if it already
exists. Select a new name before running it.

```bash
tar -czf NEW_ARCHIVE.tar.gz -C PARENT_DIRECTORY PROJECT_DIRECTORY
tar -tzf NEW_ARCHIVE.tar.gz
sha256sum -- NEW_ARCHIVE.tar.gz
```

## What to know

`-C` changes the directory used
by tar, allowing relative member names. The listing shows what was stored; the
checksum is useful for checking transfer integrity. Ordinary archives of live
databases are not necessarily consistent backups. To restore, inspect names
first and extract into a separate empty directory with the extract-tar guide.

## Reference

[Upstream manual](https://www.gnu.org/software/tar/manual/tar.html)
