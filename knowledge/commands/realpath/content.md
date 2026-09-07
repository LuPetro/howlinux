# realpath - resolve absolute canonical paths

`realpath` resolves relative components and symbolic links and prints an
absolute path.

## Examples

```bash
realpath -- FILE
realpath -e -- EXISTING_PATH
realpath --relative-to=. -- FILE
```

## What to know

`-e` requires every component to exist. GNU `-m` can normalize missing paths
but does not prove they exist or are safe to use. `--relative-to` prints a
path relative to a chosen directory. Quote path variables and keep command
output as data. A resolved path can become stale if the filesystem changes.

## Reference

[Upstream manual](https://www.gnu.org/software/coreutils/manual/coreutils.html)
