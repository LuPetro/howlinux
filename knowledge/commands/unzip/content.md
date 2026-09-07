# unzip - inspect and extract ZIP archives

`unzip` lists, tests, or extracts ZIP archives. Inspect an archive before
extraction and use a separate directory for unfamiliar content.

## Examples

```bash
unzip -l ARCHIVE.zip
unzip -t ARCHIVE.zip
unzip -n ARCHIVE.zip -d EXTRACT_DIRECTORY
```

## What to know

`-l` lists members, `-t` checks compressed data, and `-n` refuses to overwrite
existing files. `-d` selects the destination, which can be created when
needed. Extraction can still create many files and consume disk space. Avoid
`-o` unless overwriting has been deliberately reviewed, and do not run
untrusted extracted programs simply because the integrity test passed.

## Reference

[Upstream manual](https://man7.org/linux/man-pages/man1/unzip.1.html)
