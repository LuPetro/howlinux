# ss - inspect network sockets and listeners

`ss` from iproute2 inspects local sockets. Listening sockets show which local
addresses and ports applications have bound.

## Examples

```bash
ss -lnt
ss -lnu
ss -tn
ss -lntp
```

## What to know

`-l` selects listeners, `-n` prints numeric addresses and ports, `-t` selects
TCP, and `-u` selects UDP. `-p` adds process details when permissions allow.
`127.0.0.1` is loopback; `0.0.0.0` means all IPv4 interfaces. A local listener
does not prove remote reachability because firewalls, routing, and address
families also matter.

## Reference

[Upstream manual](https://man7.org/linux/man-pages/man8/ss.8.html)
