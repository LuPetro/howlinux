# touch - update timestamps or create an empty file

`touch` updates a file's access and modification timestamps. If the named file
does not exist, it creates an empty regular file by default.

```bash
touch -- FILE
touch -c -- EXISTING_FILE
touch -r REFERENCE_FILE -- FILE
```

`-c` prevents creation when the target does not exist. `-r` copies timestamps
from a reference file. Use `-a` to change only the access time or `-m` to
change only the modification time.

`touch` does not create missing parent directories and does not add content to
a file. Use `mkdir -p` for parents and a text editor or redirection when
content is required.
