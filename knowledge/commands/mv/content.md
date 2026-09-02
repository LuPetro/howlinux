```bash
mv SOURCE DESTINATION
```

`mv` stands for **move**.

Linux uses the same command to move and rename files and directories.

By default, `mv` can replace an existing destination without asking. Use `-i`
when you want a confirmation prompt before overwriting, and use `--` before
file names so a leading dash is not interpreted as an option.

## Examples

```bash
mv -i -- "file.txt" "directory/"
mv -i -- "old name" "new name"
```
