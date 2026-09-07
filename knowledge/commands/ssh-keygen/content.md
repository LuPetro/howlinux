# ssh-keygen - create and inspect SSH keys

`ssh-keygen` creates an SSH authentication key pair. Choose an unused output
filename and a passphrase when prompted; do not overwrite an existing key you
still need.

## Examples

```bash
ssh-keygen -t ed25519 -C "workstation"
ssh-keygen -lf ~/.ssh/id_ed25519.pub
```

## What to know

The `.pub` file is the public key that can be installed on a server. The file
without `.pub` is private and must stay secret. The second command prints the
public key fingerprint; adapt its path if you selected another filename. This
is your user key, not the server's host key. Protect the private key and keep
a recovery path before changing authentication settings.

## Reference

[Upstream manual](https://man.openbsd.org/ssh-keygen.1)
