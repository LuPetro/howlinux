# tar - create and extract archives

`tar` stores multiple files and their metadata in one archive. Compression is
optional and is selected separately.

```bash
tar -tf ARCHIVE.tar
tar -czf ARCHIVE.tar.gz -- DIRECTORY
tar -xzf ARCHIVE.tar.gz -C DESTINATION
```

`-t` lists, `-c` creates, and `-x` extracts. `-f` identifies the archive file;
`-z` adds gzip compression. The order and portability of some option styles
vary, so the examples use conventional GNU tar forms.

**Warning:** An untrusted archive may contain absolute paths, `..` components,
links, or files that overwrite existing destinations. List it first, extract
into a new empty directory, and inspect the result before moving files into a
sensitive location.
