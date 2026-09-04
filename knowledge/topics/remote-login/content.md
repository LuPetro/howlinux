# Connect to a remote host with SSH

You need an SSH server address, a user account, network access, and either a
password or an authorized key.

```bash
ssh USER@HOST
ssh -p PORT USER@HOST
```

At the first connection, compare the presented host-key fingerprint with one
obtained from the administrator through a trusted channel. Accepting an
unverified key can establish trust in an impostor.

For repeated connections, put non-secret settings in `~/.ssh/config`:

```text
Host example
    HostName server.example.com
    User USER
    Port 22
```

Then connect with `ssh example`. Keep private keys private and protected with a
passphrase. If a known-host warning appears later, investigate why the server
identity changed instead of suppressing the check.
