# kill - send a signal to a process

`kill` sends a signal to a process identified by PID. Despite its name, the
default `TERM` signal requests orderly termination and can be handled by the
program.

```bash
kill -TERM PID
kill -0 PID
kill -l
```

`kill -0` sends no signal but checks whether the process exists and whether the
caller may signal it. `kill -l` lists signal names on the current system.

**Warning:** `kill -KILL PID` forces immediate termination. The process cannot
save state or clean up locks and temporary files. Verify the PID with `ps`, try
`TERM`, and allow time for shutdown before considering `KILL`. Signaling other
users' processes requires appropriate privileges.
