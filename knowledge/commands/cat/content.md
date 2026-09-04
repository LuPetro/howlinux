# cat - concatenate and display files

`cat` copies the contents of files to standard output in the given order.

```bash
cat -- FILE
cat -- PART_ONE PART_TWO
cat -n -- FILE
```

`-n` numbers output lines. With no file operand, `cat` reads standard input,
which can look as though it is waiting indefinitely until input ends.

For long files, `less` is easier to navigate and does not flood the terminal.
For only the beginning or end of a file, use `head` or `tail`. Redirecting
`cat` output with `>` replaces the destination file, so inspect the redirection
target before running such a command.
