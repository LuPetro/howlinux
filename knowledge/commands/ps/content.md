# ps - display process information

`ps` prints a snapshot of processes. It does not continuously refresh.

```bash
ps -ef
ps aux
ps -p PID -o pid,ppid,user,stat,etime,cmd
```

`ps -ef` uses the POSIX-style option set; `ps aux` uses the BSD-style set.
Both commonly show processes from all users, but their columns differ. The
explicit `-o` form is preferable in scripts because it selects known fields.

A PID identifies one process only while that process exists and may later be
reused. Recheck the command and owner before sending a signal. For a live
interactive view, use `top` or another installed process monitor.
