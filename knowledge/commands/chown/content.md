# chown - change file ownership

`chown` changes a file's owner or group. Inspect the existing values first.
Changing owners generally needs administrator privileges.

## Examples

```bash
stat -- FILE
```

## What to know

**Warning:** changing ownership can prevent users or services from accessing a
file. After checking the exact path and intended account, an administrator can
run:

```bash
sudo chown USER:GROUP -- FILE
```

A colon separates owner and group. To change only the group, use `chown :GROUP
FILE` with appropriate privileges. Avoid recursive ownership changes across
home directories, system trees, or mounted filesystems; a wrong target can
break applications.

## Reference

[Upstream manual](https://www.gnu.org/software/coreutils/manual/coreutils.html)
