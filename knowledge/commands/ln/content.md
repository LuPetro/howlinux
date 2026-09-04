# ln - create hard or symbolic links

`ln` creates another directory entry referring to a file. By default it makes
a hard link; `-s` makes a symbolic link that stores a target path.

```bash
ln -s -- TARGET LINK_NAME
ln -- EXISTING_FILE HARD_LINK
```

A relative symbolic-link target is interpreted relative to the directory that
contains the link, not necessarily the shell's current directory. Check it
with `readlink -- LINK_NAME` or `readlink -f -- LINK_NAME` where supported.

Hard links normally cannot cross filesystems or refer to directories. Removing
one hard-link name does not remove the file data while another hard link still
exists. Replacing an existing link with `-f` is destructive, so inspect the
destination first.
