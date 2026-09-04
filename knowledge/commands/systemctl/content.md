# systemctl - inspect and control systemd units

`systemctl` communicates with the systemd service manager. A unit name often
ends in `.service`, although the suffix may be omitted for service operations.

```bash
systemctl status UNIT
systemctl is-active UNIT
systemctl list-units --type=service --state=running
systemctl --user status UNIT
```

The read-only forms show status without changing the system. `--user` targets
the current user's service manager rather than the system manager.

Starting, stopping, restarting, enabling, or disabling system units changes
system state and commonly requires administrator authorization. `enable`
changes boot-time links but does not necessarily start a service now;
`start` starts it now but does not necessarily enable it for boot. Review logs
with `journalctl -u UNIT` when a unit fails.
