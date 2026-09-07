# Find what is listening on a port

A bind failure can mean another process already owns the requested address and
port. Inspect the listener before stopping any service.

## Examples

```bash
ss -lntp 'sport = :8080'
ss -lnup 'sport = :8080'
```

## What to know

TCP and UDP are separate protocols, so check the one your application uses.
Process details may be hidden for other users; an administrator can repeat the
read-only query with sudo when necessary. Loopback bindings and wildcard
bindings have different reach. Identify whether the owner is an expected
service, duplicate development server, or socket-activated unit. Reconfigure
or stop the intended application gracefully instead of killing an unidentified
PID.

## Reference

[Upstream manual](https://man7.org/linux/man-pages/man8/ss.8.html)
