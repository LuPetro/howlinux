# diff - compare text files

`diff` compares text and describes changes. It does not modify either input.

## Examples

```bash
diff -u -- OLD_FILE NEW_FILE
diff -rq -- OLD_DIRECTORY NEW_DIRECTORY
```

## What to know

`-u` includes surrounding context, `-r` descends into directories, and `-q`
only reports which files differ. Exit status 0 means equal, 1 means different,
and values above 1 mean trouble. A difference is expected data, so do not
treat every nonzero result as a broken comparison. Use a binary-aware tool
when byte identity is the question.

## Reference

[Upstream manual](https://www.gnu.org/software/diffutils/manual/diffutils.html)
