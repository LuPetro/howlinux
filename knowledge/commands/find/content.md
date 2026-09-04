# find - search directory trees

`find` walks one or more directory trees and evaluates tests and actions for
each path.

```bash
find DIRECTORY -type f -name '*.log'
find DIRECTORY -type d -iname '*cache*'
find DIRECTORY -type f -mtime -7 -print
```

Quote wildcard patterns so the shell does not expand them before `find` sees
them. `-name` is case-sensitive; GNU `find` also provides `-iname` for a
case-insensitive match. `-mtime -7` selects files modified less than seven
24-hour periods ago.

**Warning:** Actions such as `-delete` and commands invoked with `-exec` can
change or remove every matching path. Run the same expression with `-print`
first and review the complete result set.
