# Understand space held by deleted open files

Removing a filename does not free the underlying data while a process still
holds the file open. This can explain some differences between filesystem and
directory usage.

## Examples

```bash
df -h -- .
du -sh -- .
lsof +L1
```

## What to know

`lsof +L1` lists open files with no remaining directory links. Other users'
processes may need administrator privileges to inspect. Identify the owning
application and its documented log-reopen procedure. A controlled restart can
close old descriptors, but it may interrupt users or discard in-memory work.
Do not truncate arbitrary `/proc/PID/fd` paths. Filesystem snapshots, reserved
blocks, sparse files, and mount boundaries can also explain differences
between df and du.

## Reference

[Upstream manual](https://man7.org/linux/man-pages/man1/lsof.1.html)
