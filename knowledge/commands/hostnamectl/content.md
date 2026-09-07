# hostnamectl - inspect system identity

`hostnamectl` reports hostname and system information on machines using
systemd and its hostname service.

## Examples

```bash
hostnamectl status
hostnamectl --static
```

## What to know

The static hostname is stored configuration; the transient hostname may be
supplied dynamically. A pretty hostname is a human-readable label. These
examples only inspect state. Changing the hostname needs appropriate
authorization and can affect service configuration. Minimal containers and WSL
setups without systemd may not provide this command's service.

## Reference

[Upstream manual](https://www.freedesktop.org/software/systemd/man/latest/hostnamectl.html)
