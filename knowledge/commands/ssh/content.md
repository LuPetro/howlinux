# ssh - open a secure remote shell

`ssh` creates an encrypted connection to an SSH server and can open an
interactive shell or run one remote command.

```bash
ssh USER@HOST
ssh -p PORT USER@HOST
ssh -i IDENTITY_FILE USER@HOST
```

On the first connection, verify the displayed host-key fingerprint through a
trusted channel before accepting it. A changed host key can be legitimate
after a rebuild, but it can also indicate interception; investigate instead of
deleting the warning automatically.

Private key files must remain private and should normally be protected with a
passphrase. Prefer configuration in `~/.ssh/config` for repeated connections
instead of long command lines. Avoid enabling agent forwarding unless the
specific trust relationship requires it.
