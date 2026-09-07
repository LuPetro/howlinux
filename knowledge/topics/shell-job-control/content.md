# Pause resume and manage shell jobs

Interactive Bash tracks jobs started by that shell. Ctrl+Z usually suspends
the foreground job; it does not finish or save that program.

## Examples

```bash
sleep 60 &
jobs -l
fg %1
```

## What to know

`&` starts a background job and `$!` holds its most recent process ID. `jobs
-l` shows this shell's jobs and IDs. Replace `%1` with the actual job shown
before using `fg`. `bg %1` resumes a suspended job in the background, but
programs reading from the terminal may suspend again. Ctrl+C normally
interrupts the foreground job. Background jobs can receive a hangup when a
session ends; use a deliberate service or session manager for durable work.

## Reference

[Upstream manual](https://www.gnu.org/software/bash/manual/bash.html)
