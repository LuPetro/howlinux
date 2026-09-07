# Redirect standard output and errors

Shell file descriptor 1 is standard output and 2 is standard error.
Redirection is set up by the shell before the program starts.

## Examples

```bash
printf '%s\n' 'another line' >> OUTPUT_FILE
ls -- MISSING_PATH 2>> ERROR_LOG
```

## What to know

`>>` appends and creates the destination if absent. **Warning:** `>` and `2>`
truncate existing output files before the command runs. Use unused output
paths or backups when choosing replacement redirection.

`COMMAND > OUTPUT_FILE 2>&1` combines both streams in one file. Order matters:
`COMMAND 2>&1 > OUTPUT_FILE` leaves stderr connected to the previous stdout.
Never read and redirect onto the same file, such as `sort FILE > FILE`. `sudo
COMMAND > FILE` does not elevate the shell that opens FILE.

## Reference

[Upstream manual](https://www.gnu.org/software/bash/manual/bash.html)
