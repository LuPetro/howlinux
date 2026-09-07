# Set up SSH public key login carefully

Create a user key pair and keep the private key on your client. Confirm the
server fingerprint through a trusted channel before the first connection.

## Examples

```bash
ssh-keygen -t ed25519 -C "workstation"
ssh-copy-id -i ~/.ssh/id_ed25519.pub USER@HOST
ssh -i ~/.ssh/id_ed25519 USER@HOST
```

## What to know

Choose an unused key filename and adapt the examples to it. `ssh-copy-id`
requires existing access, is packaged separately on some systems, and adds the
public key to the remote account's authorized_keys. The `.pub` file is safe to
install; never copy the private key to the server. Keep an existing working
session open while testing a second login. Do not disable password
authentication until key access and a recovery route are verified.

## Reference

[Upstream manual](https://man.openbsd.org/ssh.1)
