# lsof - inspect open files and sockets

`lsof` lists files opened by processes, including regular files, directories,
and network sockets. It may need to be installed separately.

## Examples

```bash
lsof -- FILE
lsof -p PID
lsof -nP -iTCP:8080 -sTCP:LISTEN
lsof +L1
```

## What to know

`-nP` avoids hostname and port-name lookups. `+L1` finds open files whose link
count is below one, often files deleted while still open. Results may be
incomplete without privileges to inspect other users' processes. Do not close
arbitrary descriptors or kill a process just to reclaim space; identify its
service and use an appropriate restart procedure.

## Reference

[Upstream manual](https://man7.org/linux/man-pages/man1/lsof.1.html)
