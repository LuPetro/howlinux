# sftp - transfer files interactively over SSH

`sftp` opens an interactive file-transfer session over SSH. Verify the host
fingerprint before trusting a new server.

## Examples

```bash
sftp USER@HOST
sftp -P 2222 USER@HOST
```

## What to know

Inside the prompt, `pwd` and `ls` inspect the remote side, while `lpwd` and
`lls` inspect the local side. `cd` and `lcd` change their respective
directories. **Warning:** `get REMOTE LOCAL` and `put LOCAL REMOTE` can
overwrite destination files. Choose unused destinations or back up existing
ones first. Type `help` for available commands and `bye` to leave. The server
must enable its SFTP subsystem.

## Reference

[Upstream manual](https://man.openbsd.org/sftp.1)
