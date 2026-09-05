# howlinux v1 requirements

This document defines the v1 behavior contract. Future ideas belong in
[`docs/future-features.md`](docs/future-features.md) and must not silently
change this contract.

## 1. Purpose

`howlinux` is a fast, local CLI that answers Linux questions from a
human-maintained knowledge base. It is a retrieval tool, not a chatbot: it
selects the best reviewed entry and renders that entry without rewriting it.

The application must be offline, deterministic, economical enough for modest
Linux systems, and useful without a database or network service.

## 2. Non-goals

Version 1 does not:

- generate or automatically translate answers;
- execute commands from queries, YAML, or Markdown;
- use cloud APIs, telemetry, embeddings, or a vector database;
- crawl arbitrary documents or treat unreviewed text as authoritative;
- maintain a persistent index or background update service;
- provide a GUI, TUI, daemon, package manager, or automatic updater.

## 3. Architecture

The executable is a thin process boundary around `howlinux_core`. The core
contains the CLI parser, knowledge loader, concept dictionary, query processor,
in-memory index, search engine, result policy, and renderers. Tests link the
same core library as the executable.

Runtime flow:

```text
CLI arguments
    -> resolve knowledge path
    -> load and validate YAML/Markdown
    -> load concepts
    -> run authoring lint when validating
    -> build in-memory index
    -> normalize and classify query
    -> generate and rank candidates
    -> apply result policy
    -> render text or JSON
```

## 4. Knowledge loading

The loader recursively discovers entry directories below the selected root.
An entry directory contains exactly the entry's `meta.yaml` and `content.md`;
categories such as `commands/` and `topics/` are organizational and must not be
hard-coded into search behavior.

The loader must:

- produce deterministic entry order independent of filesystem iteration;
- require a readable directory root;
- treat an existing empty root as a valid zero-entry knowledge base;
- accept only regular `meta.yaml` and `content.md` files and reject symlinks;
- require valid UTF-8-compatible YAML values of the documented types;
- require non-empty `id`, `title`, and `type` strings;
- require readable, non-empty Markdown content;
- reject duplicate IDs across every category;
- validate `related` references after all otherwise valid entries are loaded;
- report file, entry ID when known, severity, and cause for every diagnostic;
- skip an invalid entry without discarding unrelated valid entries;
- distinguish entry problems from global configuration failures.

Unknown metadata fields produce a warning and have no search effect. Missing
optional fields behave as empty values.

After entries and concepts load successfully, `validate` must run an offline,
deterministic authoring lint. It reports:

- aliases duplicated after search normalization, including collisions between
  entries;
- aliases within one entry that differ by only a restrained edit;
- duplicate or overly broad keywords;
- duplicate, self-referential, or non-reciprocal `related` values;
- unclosed Markdown code fences and missing local Markdown link targets;
- local Markdown links that escape the knowledge root; and
- concepts unused by all searchable entry metadata.

External links are not fetched. A lint finding is a validation issue, does not
prevent normal runtime loading, and uses exit code `1` from `validate`.

## 5. Entry format

Example:

```yaml
id: rename-folder
title: Rename a folder
type: howto
command: mv

aliases:
  - rename a folder
  - change the name of a directory
keywords:
  - rename
  - folder
  - directory
related:
  - mv
intent:
  - how_to
difficulty: beginner
platforms:
  - linux
tags:
  - filesystem
examples:
  - mv -- old-name new-name
```

Required fields:

- `id`: globally unique stable identifier
- `title`: short display title
- `type`: entry kind, normally `command` or `howto`

Optional fields:

- `command`: primary command
- `aliases`, `keywords`, `related`, `intent`, `platforms`, `tags`, `examples`:
  lists of strings
- `difficulty`: descriptive string

`content.md` is the reviewed answer. Rendering may add terminal formatting,
but must not semantically rewrite headings, prose, lists, inline code, or code
blocks.

## 6. Concepts

`knowledge/concepts.yaml` contains global equivalents:

```yaml
concepts:
  folder:
    - folder
    - directory
    - dir
```

The mapping key is the canonical term. Values may be single words or phrases.
Canonical terms and expressions must be non-empty and unique. One expression
must not belong to multiple groups. Missing `concepts.yaml` is valid and
disables expansion; an invalid or contradictory file is a global
configuration error.

Concept expansion applies to queries and indexed entry fields while preserving
the original tokens. It must remain inspectable through `--explain`.

## 7. Query processing

The processor must:

- keep the original query for display and JSON;
- normalize case and punctuation predictably;
- retain Linux-relevant tokens such as commands, flags, paths, archive
  extensions, permission modes, and redirection operators;
- preserve token sequence for phrase detection;
- remove only a small documented set of low-value stopwords;
- deduplicate scoring tokens so repetition cannot inflate a score;
- derive phrases needed by aliases and multi-word concepts;
- classify the query as `explain`, `how_to`, `why`, `command`, or `general`;
- treat query type as a ranking signal, never as a hard filter.

Leading-dash query text must be accepted after the CLI's `--` delimiter.

## 8. Search and ranking

Search uses a deterministic in-memory inverted index over metadata. Full
Markdown bodies are not scanned for every query and are not ranking fields.
Candidate generation uses exact tokens, phrases, concepts, commands, and a
bounded fuzzy fallback.

Default score components are:

| Signal | Maximum/base weight |
| --- | ---: |
| Exact alias | 100 |
| Phrase | 40 |
| Command | 30 |
| Keyword | 20 |
| Intent | 20 |
| Concept | 15 |
| Title | 10 |
| Fuzzy | 8 |
| General token | 6, adjusted by IDF |

Rare tokens receive more weight than common tokens based on document
frequency. Fuzzy matching is limited to query tokens of at least four
characters, has a bounded candidate count, and remains weaker than an exact
match. A result records whether fuzzy matching was used.

Results sort by total score, then exact/phrase contribution, then intent
contribution, then entry ID. The same inputs and knowledge base must always
produce the same ordering. `--explain` exposes component scores and concise
match reasons.

## 9. Result policy

The default policy uses these thresholds:

- confident score: `90`
- required lead over the second result: `15`
- minimum meaningful score: `8`
- default result limit: `5`
- maximum result limit: `100`

A top result is `confident` only when it reaches the confident threshold and,
when another candidate exists, clears the margin. A meaningful but weak or
ambiguous result is `uncertain`. No meaningful result is `no_match`.

Only a confident result renders the complete answer. Uncertain results render
suggestions; no-match responses explain that no suitable curated answer was
found. The tool must never fill a gap with generated text.

## 10. CLI and output

Supported forms:

```text
howlinux [options] <query...>
howlinux [options] search <query...>
howlinux [options] list
howlinux [options] show <entry-id>
howlinux [options] validate [path]
```

The CLI supports `--help`, `--version`, `--knowledge`, `--limit`, `--explain`,
`--json`, and `--`. Both `--option value` and documented `--option=value`
forms must behave consistently. `show` accepts an exact loaded ID and never a
filesystem path.

Knowledge path precedence is:

1. `--knowledge`
2. `HOWLINUX_KNOWLEDGE`
3. `knowledge/` beside the executable
4. the installed share directory relative to the executable
5. `knowledge/` in the current directory

Text output may use terminal presentation. JSON output must be valid UTF-8,
contain no ANSI escapes, remain structurally stable, and keep diagnostics out
of a normal search payload. `validate --json` intentionally embeds structured
diagnostics and a `lint` object containing whether lint ran and how many
entries, aliases, keywords, and concepts were checked.

Exit codes:

| Code | Contract |
| --- | --- |
| `0` | Confident match or successful management command |
| `1` | Uncertain/no match, unknown ID, or validation issue |
| `2` | Invalid arguments or options |
| `3` | Unavailable knowledge root or invalid global configuration |

## 11. Security and robustness

- Query text, metadata, and Markdown are data and must never be executed.
- `show` must not permit path traversal.
- The loader must not follow entry-file symlinks.
- Malformed entries must fail with controlled diagnostics rather than crashes.
- A single invalid entry must not poison unrelated valid entries.
- Search must remain bounded for long or typo-heavy queries.
- Normal operation performs no network access and emits no telemetry.
- Curated command examples must be reviewed, quote paths correctly, avoid
  private data, and warn immediately before destructive behavior.

## 12. Build, installation, and distribution

- Use standard CMake with C++20 and warnings enabled.
- Depend only on the standard library and `yaml-cpp` at runtime/build time.
- Build both Debug and Release configurations in CI on supported Ubuntu
  versions and rolling Arch Linux.
- Source builds prefer static `yaml-cpp` when available and otherwise use the
  installed package target. Explicit static linkage must fail if unavailable.
- Run unit, integration, CLI, JSON, validation, installation, and smoke tests.
- Install the executable, knowledge base, documentation, shell completions,
  and man page using GNU directory conventions.
- Produce a versioned TGZ release archive whose root contains `bin/` and
  `share/`, plus a SHA-256 checksum file.
- Official release archives must not require a specific shared `yaml-cpp` ABI;
  the release workflow must explicitly require static linkage.
- A release tag must exactly match the compiled version.

## 13. Acceptance criteria

Version 1 is acceptable when:

- the full test suite passes in clean Debug and Release builds;
- `howlinux validate knowledge` succeeds;
- representative exact, phrase, command, concept, typo, ambiguous, and
  unrelated queries follow the result policy;
- installed execution works outside the source tree;
- packaged execution works after extraction into a clean prefix;
- public documentation, CLI text, metadata, and knowledge content are English;
- licensing and third-party notices ship with source and binary packages;
- no build outputs, credentials, private paths, or personal sample data are
  tracked.
