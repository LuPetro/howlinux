# groups - show group memberships

`groups` lists the groups of the current process, or the configured groups of
a named account.

## Examples

```bash
groups
groups USER
```

## What to know

Group membership can grant access to files, devices, or administrative
facilities. Existing sessions do not automatically inherit account database
changes; compare `groups` with `groups USER` and log in again when
appropriate. Use `id` for numeric group IDs. This command reports membership
and does not modify it.

## Reference

[Upstream manual](https://www.gnu.org/software/coreutils/manual/coreutils.html)
