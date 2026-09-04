# rm - remove files and directories

`rm` removes directory entries. It does not normally move items to a desktop
trash folder, so recovery may be difficult or impossible.

## Safer usage

**Warning:** Check the resolved path before running `rm`; recursive removal can
permanently delete an entire directory tree.

```bash
rm -i -- FILE
rm -I -r -- DIRECTORY
```

`-i` asks before every removal. For recursive removal, `-I` asks once when the
operation is potentially broad. `-r` is required for non-empty directories.
Use `--` so a file name beginning with a dash is not parsed as an option.

Avoid combining recursive removal with `-f` unless you have a specific,
reviewed automation requirement. Quote variables in scripts and reject empty
or unexpected paths before passing them to `rm`.
