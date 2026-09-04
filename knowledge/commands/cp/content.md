# cp - copy files and directories

`cp` creates a copy of a file or directory. The source remains in place.

## Common forms

```bash
cp -i -- SOURCE_FILE DESTINATION
cp -a -- SOURCE_DIRECTORY DESTINATION
```

`-i` asks before replacing an existing destination. `-a` recursively copies a
directory while preserving common metadata such as permissions, timestamps,
and symbolic links. Use `--` before operands whose names could begin with a
dash.

The meaning of `DESTINATION` depends on whether it already exists and is a
directory. Inspect it first when an accidental overwrite would matter. For
large or resumable transfers, `rsync` may be a better fit if it is installed.
