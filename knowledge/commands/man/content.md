# man - read local reference manuals

`man` opens locally installed reference documentation. Package installations
may omit manuals on minimal systems.

## Examples

```bash
man ls
man 5 passwd
man -k checksum
```

## What to know

Section 1 is commonly user commands, 5 is file formats, and 8 is
administrative commands. `man 5 passwd` describes the account file rather than
the password-changing command. In a less-based pager, `/PATTERN` searches, `n`
moves to the next match, and `q` exits. `man -k` searches descriptions using
the manual index, which may need to be built by the distribution.

## Reference

[Upstream manual](https://man7.org/linux/man-pages/man1/man.1.html)
