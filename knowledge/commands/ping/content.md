# ping - test ICMP reachability

`ping` sends ICMP echo requests and reports replies, timing, and loss. Use a
finite count for a quick diagnostic.

## Examples

```bash
ping -c 4 example.com
ping -4 -c 4 example.com
ping -6 -c 4 example.com
```

## What to know

`-c 4` stops after four requests and `-4` or `-6` selects the address family.
A failed hostname lookup is different from unanswered packets. Many networks
block or rate-limit ICMP, so failure does not prove a web service is down. A
reply proves neither HTTP health nor that every port is reachable. Use `curl`
to test an HTTP application.

## Reference

[Upstream manual](https://man7.org/linux/man-pages/man8/ping.8.html)
