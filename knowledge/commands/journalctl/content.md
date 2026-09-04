# journalctl - query the systemd journal

`journalctl` reads structured logs collected by systemd-journald.

```bash
journalctl -b
journalctl -u UNIT --since today
journalctl -p warning..alert
journalctl -f
```

`-b` limits output to the current boot, `-u` selects a unit, `--since` limits
the time range, and `-p` selects priorities. `-f` follows new messages until
`Ctrl-C` is pressed. Output normally opens in a pager; add `--no-pager` for a
non-interactive script.

The available history depends on journal configuration and retention. Reading
all system messages may require membership in groups such as `systemd-journal`
or administrator authorization. Logs can contain hostnames, user names, and
application data, so review them before sharing.
