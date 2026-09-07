# dig - inspect DNS records

`dig` from BIND DNS tools asks DNS servers for records. It may be packaged as
`dnsutils`, `bind9-dnsutils`, or `bind` depending on the distribution.

## Examples

```bash
dig example.com A
dig example.com AAAA
dig example.com MX
dig +short example.com
```

## What to know

Inspect the response status and answer section: NXDOMAIN means the queried
name does not exist in that DNS response, while NOERROR can still have no
records of the requested type. `+short` hides useful diagnostic context. `dig
@SERVER NAME TYPE` selects a resolver explicitly. DNS results may differ from
application lookups using `/etc/hosts` or other NSS sources.

## Reference

[Upstream manual](https://bind9.readthedocs.io/en/latest/manpages.html#dig-dns-lookup-utility)
