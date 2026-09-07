# Connect commands with shell pipelines

A pipe sends one command's standard output to the next command's standard
input. It carries a stream of data, not a list of shell arguments.

## Examples

```bash
printf '%s\n' apple pear apple | sort | uniq -c
(set -o pipefail; false | cat)
printf 'pipeline status: %s\n' "$?"
```

## What to know

The first pipeline counts fruit names. By default, Bash uses the exit status
of the final command as the pipeline status. With `pipefail`, the pipeline
reports the rightmost nonzero status, or zero if all commands succeed.
Parentheses limit the setting to the example subshell. Stderr is separate
unless explicitly redirected. Commands that stop reading early can cause an
upstream SIGPIPE, so interpret that result in context.

## Reference

[Upstream manual](https://www.gnu.org/software/bash/manual/bash.html)
