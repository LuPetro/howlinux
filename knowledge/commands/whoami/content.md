# whoami - print the effective username

`whoami` shows the account name associated with the effective user ID of the
process.

## Examples

```bash
whoami
id -un
```

## What to know

The two examples answer the same question. The effective user can differ from
the original login user after privilege changes. Do not rely on the editable
`$USER` environment variable to make authorization decisions. Use `id` to
inspect numeric IDs and group memberships when diagnosing permissions.

## Reference

[Upstream manual](https://www.gnu.org/software/coreutils/manual/coreutils.html)
