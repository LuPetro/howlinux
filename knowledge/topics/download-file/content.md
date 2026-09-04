# Download a file with curl

Choose the output name explicitly and make HTTP failures visible:

```bash
curl --fail --location --output "FILE" 'https://example.com/file'
```

`--location` follows redirects, `--fail` returns a failure status for HTTP
error responses, and `--output` prevents binary data from being printed to the
terminal. Add `--continue-at -` to attempt resuming a partially downloaded
file when the server supports it.

Treat downloads as untrusted input. Obtain an expected checksum or signature
through a trusted channel and verify it before opening or executing the file.
Never pipe downloaded content directly to a shell. URLs and command-line
arguments can be logged, so do not embed credentials or private tokens in
them.
