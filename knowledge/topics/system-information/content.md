# Collect useful Linux system information

Collect a small set of read-only facts when reporting a problem: userspace
distribution, kernel, architecture, memory, and storage layout.

## Examples

```bash
cat /etc/os-release
uname -r
uname -m
free -h
lsblk -o NAME,SIZE,TYPE,FSTYPE
```

## What to know

The distribution and kernel can have different origins in WSL or containers.
Memory and devices visible inside a container may not describe its effective
resource limits. Share only the parts relevant to the issue, and review output
for hostnames, account names, UUIDs, or private mount paths before posting
publicly. These commands do not collect passwords or execute a generated
diagnostic script.

## Reference

[Upstream manual](https://www.gnu.org/software/coreutils/manual/coreutils.html)
