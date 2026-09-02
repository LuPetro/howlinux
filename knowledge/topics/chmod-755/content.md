```bash
chmod 755 -- "script.sh"
```

`755` means:

```text
Owner: read, write, execute (7)
Group: read, execute         (5)
Other: read, execute         (5)
```

This is commonly used for scripts and programs that should be executable.
It also makes the file readable and executable by every local user. Do not use
`755` for files that contain secrets, and inspect a script before executing it.
