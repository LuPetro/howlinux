# du - estimate file and directory usage

`du` walks files and directories and estimates their storage usage.

```bash
du -sh -- DIRECTORY
du -h --max-depth=1 -- DIRECTORY
du -ah -- DIRECTORY
```

`-s` prints only a total and `-h` formats sizes for people. GNU
`--max-depth=1` reports the immediate children of a directory. `-a` includes
individual files and can produce a great deal of output.

By default, `du` reports allocated filesystem space, which may differ from
apparent file sizes because of sparse files, compression, hard links, and
filesystem behavior. Permission errors mean the total may be incomplete. Use
`df` when the question is total capacity of a mounted filesystem.
