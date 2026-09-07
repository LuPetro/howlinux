# gzip - compress and inspect gzip files

`gzip` compresses a single byte stream. Use `tar` first when you need to keep
a directory tree and its filenames.

## Examples

```bash
gzip -k -- FILE
gzip -l -- FILE.gz
gzip -t -- FILE.gz
gzip -dc -- FILE.gz
```

## What to know

`-k` keeps the input file; without it, normal compression replaces the input
with a `.gz` file. `-t` checks compressed-data integrity, `-l` prints size
information, and `-dc` decompresses to stdout without removing the archive.
Choose unused output paths and avoid redirecting binary output to an
interactive terminal. A valid gzip stream is not proof that its contents are
safe.

## Reference

[Upstream manual](https://www.gnu.org/software/gzip/manual/gzip.html)
