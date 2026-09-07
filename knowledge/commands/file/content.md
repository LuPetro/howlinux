# file - identify file contents

`file` examines content and filesystem metadata to estimate the file type.
Extensions are only names and may be misleading.

## Examples

```bash
file -- FILE
file --mime-type -- FILE
file -L -- LINK
```

## What to know

`--mime-type` prints a MIME-style classification; `-L` follows a symbolic
link. Identification is heuristic: it does not prove that an archive,
document, or executable is trustworthy or well formed. Keep the file utility
updated when examining untrusted samples. Use `stat` for ownership and
timestamps instead.

## Reference

[Upstream manual](https://man7.org/linux/man-pages/man1/file.1.html)
