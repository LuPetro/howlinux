# tail - show or follow the end of files

`tail` prints the end of each input file. The default is ten lines.

```bash
tail -n 50 -- FILE
tail -f -- LOG_FILE
tail -F -- LOG_FILE
```

`-n` selects the number of lines. `-f` keeps watching an open file for appended
data. GNU `tail -F` retries when the path is replaced or temporarily missing,
which is useful for rotated logs. Press `Ctrl-C` to stop following.

Access to system logs may require membership in an appropriate group or
administrator privileges. For services managed by systemd, `journalctl -f`
often provides a more complete view than following one text file.
