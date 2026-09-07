# scp - copy files over SSH

`scp` copies between local and remote paths through SSH authentication. Verify
a new server's host-key fingerprint through a trusted channel. **Warning:** an
existing destination file can be overwritten without a confirmation prompt.

## Examples

```bash
scp ./FILE USER@HOST:./REMOTE_FILE
scp USER@HOST:./REMOTE_FILE ./LOCAL_FILE
scp -P 2222 ./FILE USER@HOST:./REMOTE_FILE
```

## What to know

`-P` uses an uppercase P for the SSH port; lowercase `-p` preserves times and
modes. Modern OpenSSH normally uses SFTP for transfers, so the server must
support it. Use `./` for local names containing a colon so they are not
confused with host syntax. For interactive browsing and explicit destination
selection, use `sftp`.

## Reference

[Upstream manual](https://man.openbsd.org/scp.1)
