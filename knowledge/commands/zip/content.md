# zip - create ZIP archives

`zip` stores named files in a ZIP archive. Choose an unused archive filename;
if the archive already exists, zip normally updates its entries.

## Examples

```bash
zip NEW_ARCHIVE.zip FILE1 FILE2
zip -r NEW_ARCHIVE.zip DIRECTORY
```

## What to know

`-r` includes a directory recursively. Keep the destination outside the source
directory to avoid confusing archive-in-archive workflows. On Unix, symbolic
links are normally followed; `-y` stores the links themselves when that is
intended. Check the contents with `unzip -l`. ZIP is convenient for
interchange but may not preserve all Linux ownership, ACLs, and extended
attributes needed for a system backup.

## Reference

[Upstream manual](https://man7.org/linux/man-pages/man1/zip.1.html)
