# getent - query configured name service databases

`getent` reads databases through the system Name Service Switch configuration.
This can include local files, DNS, and other configured providers.

## Examples

```bash
getent ahosts example.com
getent hosts localhost
getent passwd USER
getent group GROUP
```

## What to know

`ahosts` uses address resolution similar to many applications and may print
several socket-type rows per address. `hosts` uses the hosts database. This
differs from `dig`, which queries DNS directly and does not consult all NSS
sources. A passwd record is account metadata, not the password hash. Minimal
non-glibc systems can support different database names or options.

## Reference

[Upstream manual](https://man7.org/linux/man-pages/man1/getent.1.html)
