# Diagnose a permission denied error

Start by checking the exact path, your effective user, and the containing
directories. File permissions alone do not describe every access requirement.

## Examples

```bash
id
ls -ld -- . PARENT_DIRECTORY FILE
stat -- FILE
findmnt --target .
```

## What to know

Reading a file needs read permission plus execute/search permission on every
containing directory. Listing names needs directory read permission; creating
or removing names typically needs directory write and execute permission. The
sticky bit can add deletion restrictions. Read-only mounts, ACLs, SELinux, or
AppArmor can deny access despite ordinary mode bits. Do not apply `chmod -R
777` as a workaround. Change only the specific ownership or permission that
the intended access model requires.

## Reference

[Upstream manual](https://www.gnu.org/software/coreutils/manual/coreutils.html)
