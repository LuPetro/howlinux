```bash
chmod MODE -- FILE
```

`chmod` changes the permissions of a file or directory.

## Example

```bash
chmod 755 -- "script.sh"
```

Choose the mode deliberately: overly broad permissions may expose data or let
other users run a file. `chmod` does not verify that a script is trustworthy.
