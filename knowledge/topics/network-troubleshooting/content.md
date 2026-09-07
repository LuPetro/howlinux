# Diagnose a network connection step by step

Check the local interface, routing, name resolution, and application in that
order. This makes failures easier to locate without changing network settings.

## Examples

```bash
ip -br address
ip route
getent ahosts example.com
curl --head --connect-timeout 5 --max-time 15 https://example.com
```

## What to know

An address alone does not guarantee a default route. If resolution fails, use
the DNS troubleshooting guide. A timeout suggests different causes from
connection refused or an HTTP error. Some servers reject HEAD requests despite
serving GET successfully. VPN routes, proxies, firewalls, captive portals, and
IPv4/IPv6 differences can all matter. Avoid flushing firewall or routing rules
during a remote session; first preserve access and inspect the specific
failing layer.

## Reference

[Upstream manual](https://curl.se/docs/manpage.html)
