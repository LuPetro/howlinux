# umask - control default creation permissions

`umask` is a shell builtin that masks permission bits when programs create
files or directories. It does not change existing objects.

## Examples

```bash
umask
umask -S
(umask 027; mkdir -- private-example)
```

## What to know

Typical programs request mode 666 for files and 777 for directories. A mask of
027 commonly produces 640 files and 750 directories; bits are removed, not
arithmetically subtracted. The parentheses limit the mask change to a
subshell. Programs may request stricter modes and default ACLs affect results.
Inspect with `stat` when access matters.

## Reference

[Upstream manual](https://www.gnu.org/software/bash/manual/bash.html)
