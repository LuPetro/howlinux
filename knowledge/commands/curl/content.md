# curl - transfer data using URLs

`curl` sends or receives data using protocols such as HTTP and HTTPS. It writes
response data to standard output unless an output option is supplied.

```bash
curl --fail --location --output FILE 'https://example.com/file'
curl --include 'https://example.com/'
curl --head 'https://example.com/'
```

`--fail` makes HTTP error responses return a failure status, `--location`
follows redirects, and `--output` writes to a chosen file. `--head` requests
headers only where the protocol supports it.

Do not put passwords, access tokens, or private data directly in command-line
arguments because other local users or logs may expose them. Never pipe a
download directly into a shell; save it, verify its source and checksum, and
inspect it first.
