# Inspect and stop a process

Identify the process and verify its owner and full command before signaling
it:

```bash
ps -p PID -o pid,ppid,user,stat,etime,cmd
```

Request a normal shutdown with `TERM`:

```bash
kill -TERM PID
```

Wait for the program to clean up, then run the `ps` command again. A service
managed by systemd should usually be stopped with `systemctl stop UNIT` so the
service manager knows the intended state.

**Warning:** `kill -KILL PID` forces immediate termination without allowing
cleanup or saved state. Use it only after confirming the PID, trying `TERM`,
and understanding the consequences. PIDs can be reused after a process exits,
so recheck immediately before escalating.
