```bash
tar -tzf "archive.tar.gz"
```

This lists the archive first. Inspect paths and links before extracting an
archive from an untrusted source.

```text
t = list      (inspect without extracting)
x = extract   (used in the extraction command below)
z = gzip      (compressed with gzip)
f = file      (the filename follows)
```

## Extract into a specific directory

```bash
mkdir -- "target-directory"
tar -xzf "archive.tar.gz" -C "target-directory"
```

Use a new, empty target directory when practical. An archive can contain paths
or symbolic links that write somewhere you did not expect; do not extract an
untrusted archive with elevated privileges.
