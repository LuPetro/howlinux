# id - inspect user and group identities

`id` displays user and group IDs. With no username it reports the current
process identity and group membership.

## Examples

```bash
id
id -u
id -un
id -Gn
id USER
```

## What to know

`-u` selects the user ID, `-g` the primary group, and `-G` all groups; `-n`
prints names. Asking about a named account consults the account databases and
may differ from an already running session. After group membership changes, a
fresh login is normally required for the session to inherit them.

## Reference

[Upstream manual](https://www.gnu.org/software/coreutils/manual/coreutils.html)
