# Remove files and directories carefully

First print and inspect the exact path:

```bash
ls -ld -- "TARGET"
```

**Warning:** `rm` normally bypasses the desktop trash and can make data
unrecoverable. Confirm that `TARGET` is correct before continuing.

```bash
rm -i -- "FILE"
rm -I -r -- "DIRECTORY"
```

Use `-i` for confirmation on individual files. A non-empty directory needs
recursive mode; `-I` adds one confirmation for a potentially broad operation.
The `--` marker protects names beginning with a dash.

Do not use broad globs until you have printed their expansion. In scripts,
quote variables and validate that a computed path is non-empty, absolute when
appropriate, and contained below the intended directory.
