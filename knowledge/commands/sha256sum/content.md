# sha256sum - compute and verify file checksums

`sha256sum` calculates a file digest or checks files against a checksum
manifest.

## Examples

```bash
sha256sum -- FILE
sha256sum --check SHA256SUMS
```

## What to know

Run the check from the directory expected by the manifest's relative paths. A
successful comparison detects a difference from the listed digest; it does not
establish who published the file. Obtain the expected checksum from a trusted
source, and verify a publisher signature when available. If both a file and
its checksum are replaced by an attacker, they can still agree.

## Reference

[Upstream manual](https://www.gnu.org/software/coreutils/manual/coreutils.html)
