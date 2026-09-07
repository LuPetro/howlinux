# uname - inspect kernel and machine information

`uname` reports kernel and machine information. It does not directly identify
the installed distribution release.

## Examples

```bash
uname -r
uname -m
uname -a
cat /etc/os-release
```

## What to know

`-r` prints the kernel release, `-m` the machine architecture, and `-a` the
available system fields. On containers and WSL, the kernel can belong to the
host environment. `/etc/os-release` describes the userspace distribution. A
64-bit kernel does not by itself prove that every installed program uses a
64-bit ABI.

## Reference

[Upstream manual](https://www.gnu.org/software/coreutils/manual/coreutils.html)
