# Inspect and manage a systemd service

Begin with read-only status and recent logs:

```bash
systemctl status UNIT
journalctl -u UNIT -n 100 --no-pager
```

If a configuration change has been validated and a restart is appropriate,
changing a system service normally requires administrator authorization:

```bash
sudo systemctl restart UNIT
systemctl is-active UNIT
```

Restarting interrupts the service, so assess users and dependent systems
first. Prefer `reload` only when the unit documents that it supports reloading.

`enable` controls whether a unit is linked into boot targets; it is different
from `start`, which changes the current runtime state. For per-user services,
use `systemctl --user ...` without `sudo`.
