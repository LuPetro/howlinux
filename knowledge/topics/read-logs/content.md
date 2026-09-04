# Read and follow Linux logs

On a systemd system, query the journal by unit and time range:

```bash
journalctl -u UNIT --since today
journalctl -u UNIT -n 100 --no-pager
journalctl -u UNIT -f
```

The last form follows new messages until `Ctrl-C`. For a traditional text log,
read a bounded tail before following it:

```bash
tail -n 100 -- "LOG_FILE"
tail -F -- "LOG_FILE"
```

Log access may require an appropriate group or administrator authorization.
Use narrow unit and time filters before exporting data. Logs often contain
user names, addresses, internal paths, query data, and tokens accidentally
written by applications; redact sensitive information before sharing them.
