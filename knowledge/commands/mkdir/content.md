# mkdir - create directories

`mkdir` creates one or more directories.

```bash
mkdir -- NEW_DIRECTORY
mkdir -p -- PARENT/CHILD
```

Without `-p`, the parent directory must already exist and `mkdir` reports an
error if the requested directory exists. `-p` creates missing parent
directories and accepts an existing target.

The new directory's permissions are derived from the requested mode and the
process `umask`. To request a particular starting mode, use `-m`, for example
`mkdir -m 750 -- PRIVATE_DIRECTORY`; the effective result can still be subject
to platform rules and access-control lists.
