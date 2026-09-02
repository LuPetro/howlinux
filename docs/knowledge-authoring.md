# Authoring knowledge for howlinux

This guide covers the everyday workflow for adding and reviewing knowledge
entries. An entry consists only of YAML metadata and Markdown content; adding
one does not require a C++ change or rebuild.

howlinux never generates answer text. Accuracy, realistic search phrases, and
careful validation therefore matter more than content volume.

> **Safety rule:** Entries may explain and show shell commands, but howlinux
> never runs them. Put a clear warning immediately before a destructive or
> irreversible command.

## 1. Layout

```text
knowledge/
|-- concepts.yaml
|-- commands/
|   `-- mv/
|       |-- meta.yaml
|       `-- content.md
`-- topics/
    `-- rename-folder/
        |-- meta.yaml
        `-- content.md
```

- `commands/` contains individual command references.
- `topics/` contains task-oriented guides and explanations.
- Other categories are supported, but introduce them only with a deliberate
  taxonomy.
- Entry IDs are unique across the entire knowledge base.
- The directory name and `id` should match.
- `concepts.yaml` contains global equivalents and is not an entry.

Select another knowledge directory with `--knowledge`:

```bash
./build/howlinux --knowledge /absolute/path/to/knowledge "rename a directory"
./build/howlinux validate /absolute/path/to/knowledge
```

## 2. Create an entry

### Choose a category and ID

Use a short, stable, descriptive ID containing lowercase ASCII letters,
numbers, and hyphens:

```text
rename-folder
chmod-755
extract-tar
```

Avoid version numbers, marketing terms, and unnecessary abbreviations. Other
entries may refer to the ID through `related`, so renaming an ID is an
incompatible content change.

```bash
mkdir -p knowledge/topics/rename-folder
```

### Write `meta.yaml`

```yaml
id: rename-folder
title: Rename a folder
type: howto
command: mv

aliases:
  - rename a folder
  - change the name of a directory
  - give a directory a different name

keywords:
  - rename
  - folder
  - directory
  - mv

related:
  - mv

intent:
  - how_to

difficulty: beginner
platforms:
  - linux
  - ubuntu
tags:
  - filesystem
examples:
  - mv -- old-name new-name
```

| Field | Status | Format | Purpose |
| --- | --- | --- | --- |
| `id` | required | string | Globally unique, stable identifier |
| `title` | required | string | Short, clear display title |
| `type` | required | string | Normally `command` or `howto` |
| `command` | optional | string | Primary command, when applicable |
| `aliases` | recommended | string list | Realistic search phrases |
| `keywords` | recommended | string list | Distinctive terms, flags, and commands |
| `related` | optional | ID list | Existing related entries |
| `intent` | optional | string list | Query types such as `how_to` or `explain` |
| `difficulty` | optional | string | For example `beginner` or `advanced` |
| `platforms` | optional | string list | Tested or relevant platforms |
| `tags` | optional | string list | Editorial grouping |
| `examples` | optional | string list | Short examples for metadata/tools |

Missing optional lists behave as empty lists. Unknown fields produce a
validation warning and do not affect search.

YAML rules:

- Save YAML and Markdown as UTF-8.
- Use spaces, never tabs.
- Quote values with colons, leading special characters, or YAML-like boolean
  words.
- Every list field must actually be a list of strings.
- `related` contains IDs, not titles or paths.
- Do not repeat aliases or keywords to manipulate ranking.
- `meta.yaml` and `content.md` must be regular files; symlinks are rejected.

### Write `content.md`

The Markdown file is the reviewed answer and may contain headings, lists,
inline code, and fenced code blocks. A useful order is:

1. Short explanation
2. General syntax
3. Simple verified example
4. Important variants or flags
5. Common mistakes, prerequisites, and safety warnings
6. Related entries

Example:

````markdown
# Rename a folder

The `mv` command can rename a folder by moving it to a new path.

## Syntax

```bash
mv -- OLD_NAME NEW_NAME
```

## Example

```bash
mv -- "old project" "new project"
```

An existing destination can change `mv` behavior. Inspect the destination
before running the command.

## Related

- `mv`
````

For every entry:

- verify commands in a safe test environment;
- use obvious placeholders such as `OLD_NAME` and `NEW_NAME`;
- quote file names containing spaces;
- state required privileges, packages, versions, and platform limitations;
- warn immediately before destructive or irreversible effects;
- do not recommend `sudo` by default;
- do not copy unverified one-liners from external sources;
- never include secrets, real tokens, private hostnames, personal paths, or
  personal data;
- provide understandable examples in the content, not only in metadata.

## 3. Aliases, keywords, type, and intent

Aliases should be phrases a person would genuinely type. Aim for at least
three meaningfully different phrases. Avoid generic phrases such as `linux
help`, stopword-only variants, and many nearly identical sentences. Exact
aliases are a strong ranking signal, so an overly broad alias can suppress the
correct entry.

Keywords should distinguish the entry: its primary action, object, command,
important flags, file formats, permission modes, or established technical
terms. Avoid complete questions and generic words such as `help`, `linux`, or
`command` unless they truly distinguish the entry.

`type` describes the entry: normally `command` for a command reference and
`howto` for a task or explanation. `intent` describes suitable query types:
`explain`, `how_to`, `why`, `command`, or `general`. Intent is a ranking signal,
not a filter.

## 4. Concepts and synonyms

Put globally equivalent language in `knowledge/concepts.yaml`:

```yaml
concepts:
  folder:
    - folder
    - directory
    - dir
  permissions:
    - permission
    - permissions
    - rights
    - access rights
```

Use the canonical term as the key and include it in its list for clarity.
Only group expressions that are genuinely interchangeable in the Linux
context. An expression may appear in one group only; canonical names must be
unique; groups must not define each other recursively. Entry-specific wording
belongs in local aliases instead.

## 5. Validate search behavior

Run the complete validator after every knowledge change:

```bash
./build/howlinux validate knowledge
./build/howlinux --json validate knowledge
```

Then test several query forms:

```bash
./build/howlinux --explain "rename a directory"
./build/howlinux --explain "how can I change a folder name"
./build/howlinux --explain "mv"
./build/howlinux --explain "renmae fodler"
./build/howlinux --explain "an unrelated question"
```

Include an exact alias, natural phrase, command-oriented query, restrained typo,
ambiguous query, and unrelated query. Inspect component scores and ensure the
right result is confident only when justified. Use `--` for leading flags:

```bash
./build/howlinux -- "--recursive"
```

Exit code `0` means a confident match or successful management command; `1`
means uncertain/no match, unknown ID, or validation issue; `2` means invalid
CLI input; `3` means the knowledge root or global concepts configuration could
not be used.

## 6. Review checklist

- [ ] ID is stable, lowercase, hyphenated, and globally unique.
- [ ] Required fields have the correct types.
- [ ] Related IDs exist and do not merely point back accidentally.
- [ ] Aliases are realistic, distinct, and specific.
- [ ] Keywords distinguish this entry from its neighbors.
- [ ] Type and intent match the content.
- [ ] Every command was tested safely.
- [ ] Placeholders and quoting are clear.
- [ ] Prerequisites and platform limits are explicit.
- [ ] Destructive behavior has an immediate warning.
- [ ] No secret, private hostname, personal path, or personal data is present.
- [ ] `validate knowledge` passes.
- [ ] Exact, natural, typo, ambiguous, and unrelated queries behave sensibly.
- [ ] `--explain` shows understandable reasons for the result.

## 7. Common problems

If an entry does not load, run `validate knowledge` and check file types,
required values, YAML list types, duplicate IDs, and `related` references.

If an entry cannot be found, add realistic aliases and distinctive keywords;
use a concept only when the equivalence is global.

If the wrong entry wins, compare candidates with `--explain`. Remove broad
aliases or keywords before changing confidence thresholds.

If a flag is parsed as a howlinux option, place `--` before the query.
