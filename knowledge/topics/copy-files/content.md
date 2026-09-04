# Copy files and directories safely

Use `cp` for a local copy. Choose the destination carefully because an
existing file may otherwise be replaced.

```bash
cp -i -- "SOURCE_FILE" "DESTINATION"
cp -a -- "SOURCE_DIRECTORY" "DESTINATION"
```

`-i` asks before overwriting a file. For a directory tree, `-a` copies
recursively and preserves common metadata and links. If `DESTINATION` is an
existing directory, the source is copied inside it; otherwise that path
becomes the name of the new copy.

Compare important copies before deleting the source. For a quick byte-for-byte
file comparison, use `cmp -- SOURCE_FILE DESTINATION_FILE`. A successful local
copy is not a backup until it is stored and tested independently from the
original system.
