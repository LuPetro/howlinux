# Diagnose hostname and DNS resolution problems

Separate the system's application lookup from a direct DNS query. Different
results can point to local host mappings or resolver configuration.

## Examples

```bash
getent ahosts example.com
dig example.com A
dig example.com AAAA
cat /etc/resolv.conf
```

## What to know

If getent succeeds while dig differs, inspect `/etc/hosts` and the hosts line
in `/etc/nsswitch.conf`. A loopback resolver in resolv.conf may be managed by
a local service; do not overwrite that file blindly. DNS timeout and NXDOMAIN
are different failures. `dig` requires BIND tools; systemd-resolved
installations may also offer `resolvectl query example.com`. Once lookup
works, use curl to test the application separately.

## Reference

[Upstream manual](https://bind9.readthedocs.io/en/latest/manpages.html#dig-dns-lookup-utility)
