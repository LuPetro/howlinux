# howlinux 1.0.1

`howlinux` is a fast, local command-line search tool for curated Linux
knowledge. It selects the best entry from a YAML and Markdown knowledge base
and prints the reviewed content verbatim.

The program works entirely offline. It does not generate answers, call cloud
services, send telemetry, execute searches, or run any shell command shown in
an answer.

## Features

- Recursive knowledge loader with schema, reference, and duplicate validation
- Extensible categories below `knowledge/` without C++ changes
- Global single-word and multi-word synonyms in `concepts.yaml`
- Linux-aware query normalization that preserves tokens such as `tar.gz`,
  `755`, `-r`, `--recursive`, and `2>`
- Deterministic in-memory inverted index with IDF weighting
- Explainable ranking across aliases, phrases, commands, keywords, concepts,
  intent, titles, tokens, and limited typo correction
- Conservative confident, uncertain, and no-match result policy
- `search`, `list`, `show`, `validate`, `--explain`, and ANSI-free JSON output
- 38 reviewed command references and task-oriented guides covering files,
  text search, storage, processes, networking, services, logs, and packages
- Debug and release builds, automated tests, installation rules, shell
  completions, and a man page

The detailed behavior contract is in [requirements.md](requirements.md).
See [docs/knowledge-authoring.md](docs/knowledge-authoring.md) to add content
and [docs/future-features.md](docs/future-features.md) for post-v1 ideas.

## Requirements

On Ubuntu or Debian:

```bash
sudo apt update
sudo apt install -y build-essential cmake libyaml-cpp-dev git
```

Building requires a C++20 compiler, CMake 3.16 or newer, and the `yaml-cpp`
development package. Release archives link `yaml-cpp` statically, so they do
not depend on a particular `libyaml-cpp` ABI at runtime. The application has no
network or database dependency.

## Install

Clone, build, test, and install to `~/.local`:

```bash
git clone https://github.com/LuPetro/howlinux.git
cd howlinux
./scripts/install.sh
```

Choose another absolute installation prefix with `HOWLINUX_PREFIX`:

```bash
HOWLINUX_PREFIX="$HOME/opt/howlinux" ./scripts/install.sh
```

If the command is not yet on `PATH`:

```bash
export PATH="$HOME/.local/bin:$PATH"
howlinux --version
howlinux validate
```

Uninstall a CMake installation made by the script:

```bash
cmake --build build-install --target uninstall
```

Alternatively, download the v1.0.1 archive and `SHA256SUMS` from the GitHub
release, verify it, and extract it into a prefix:

```bash
sha256sum --check SHA256SUMS
mkdir -p "$HOME/.local"
tar -xzf howlinux-1.0.1-Linux-x86_64.tar.gz -C "$HOME/.local"
"$HOME/.local/bin/howlinux" validate
```

The archive contains `bin/` and `share/` at its root, including shell
completions, the man page, the knowledge base, and project documentation.

### WSL and PowerShell

Run `howlinux` normally inside WSL. From PowerShell, use a WSL login shell so
the Linux profile and `PATH` are loaded:

```powershell
wsl.exe -d Ubuntu -- bash -lc "howlinux --version"
wsl.exe -d Ubuntu -- bash -lc "howlinux 'what does chmod 755 mean'"
```

## Build and test

Debug build:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Release build:

```bash
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release --parallel
ctest --test-dir build-release --output-on-failure
```

Install the release build manually:

```bash
cmake --install build-release --prefix "$HOME/.local"
```

Before creating a release tag, run the clean Debug, Release, install, and
package audit:

```bash
./scripts/release-audit.sh
```

## Quick start

```bash
./build/howlinux rename folder
./build/howlinux "how can I change the name of a directory"
./build/howlinux "what does chmod 755 mean"
./build/howlinux "renmae fodler"
./build/howlinux --explain "change the directory name"
./build/howlinux --json "extract tar.gz"
```

A confident match prints the complete, unchanged `content.md` entry. If the
score or the lead over the second result is too small, howlinux prints
suggestions instead. An unrelated query never produces invented text.

## CLI reference

```text
howlinux [options] <query...>
howlinux [options] search <query...>
howlinux [options] list
howlinux [options] show <entry-id>
howlinux [options] validate [path]
```

| Option | Meaning |
| --- | --- |
| `-h`, `--help` | Show help without loading the knowledge base |
| `-V`, `--version` | Show the program version |
| `--knowledge <path>` | Select a knowledge directory |
| `--limit <n>` | Return 1 to 100 search results; default: 5 |
| `--explain` | Include query type, concepts, score components, and match reasons |
| `--json` | Emit stable, machine-readable, ANSI-free JSON |
| `--` | Stop option parsing and treat the rest as query text |

`list` prints all valid entries in deterministic ID order. `show <entry-id>`
only accepts an already loaded ID and never interprets it as a path.
`validate [path]` uses the runtime loader to check entries, field types, IDs,
references, and concepts.

Queries beginning with a dash must follow `--`:

```bash
./build/howlinux -- "--recursive"
./build/howlinux --knowledge knowledge search -- "tar -xzf"
```

### Exit codes

| Code | Meaning |
| --- | --- |
| `0` | Confident match or successful management command |
| `1` | Uncertain/no match, unknown ID, or validation failure |
| `2` | Invalid CLI arguments or options |
| `3` | Unreadable knowledge directory or invalid global configuration |

Loader and configuration diagnostics go to `stderr` during normal search so
`stdout` remains a single JSON value. `validate --json` includes diagnostics
in its `diagnostics` array. Help, version, and parser errors remain readable
plain text.

## Knowledge path resolution

The first available source wins:

1. `--knowledge <path>`
2. `HOWLINUX_KNOWLEDGE`
3. `knowledge/` beside the executable
4. `../share/howlinux/knowledge/` relative to the executable
5. `knowledge/` in the current working directory

Explicit relative paths and the environment variable are resolved from the
current working directory. Prefer absolute paths in scripts and services:

```bash
HOWLINUX_KNOWLEDGE=/srv/howlinux/knowledge howlinux mv
howlinux --knowledge /srv/howlinux/knowledge validate
```

A missing directory is a configuration error. An existing empty directory is
a valid knowledge base containing zero entries.

## Add knowledge

Each entry is a directory containing `meta.yaml` and `content.md`:

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

A minimal metadata file looks like this:

```yaml
id: rename-folder
title: Rename a folder
type: howto
command: mv
aliases:
  - rename folder
  - rename directory
  - change folder name
keywords:
  - rename
  - folder
  - directory
related:
  - mv
intent:
  - how_to
```

`id`, `title`, `type`, and a readable, non-empty regular `content.md` file are
required. Symlinks are rejected. The Markdown content may contain headings,
lists, inline code, and fenced code blocks; howlinux does not rewrite it.

No rebuild is needed after adding an entry:

```bash
./build/howlinux validate knowledge
./build/howlinux --explain "a realistic search phrase"
```

Global equivalents belong in `knowledge/concepts.yaml`. An expression may
belong to only one concept group. A missing concepts file disables expansion;
contradictory concepts are a configuration error. For the full schema and
review checklist, read [docs/knowledge-authoring.md](docs/knowledge-authoring.md).

## Search and JSON behavior

At startup, howlinux builds an in-memory index from metadata. Rare tokens carry
more weight than common tokens. Ranking combines exact aliases and phrases,
commands, weighted keywords, concepts, intent, titles, token overlap, and a
limited fuzzy fallback. `--explain` exposes each component. Ties are resolved
deterministically.

Search JSON always contains `status` (`confident`, `uncertain`, or `no_match`),
`query`, `query_type`, `concepts`, `results`, and `entry`. Results contain
`id`, `title`, `score`, `fuzzy_used`, and `match_reasons`. With `--explain`,
they also contain a score `breakdown`. The complete entry is present only for
a confident match.

```bash
./build/howlinux --json --explain "chmod 755" | jq .
```

## Troubleshooting

- If the knowledge directory is unreadable, pass an explicit absolute path and
  run `validate`.
- If an entry is skipped, `validate knowledge` reports the file, entry ID, and
  cause. One invalid entry does not prevent valid entries from loading.
- If `-r` or `--recursive` is parsed as an option, place `--` before the query.
- If a result remains uncertain, compare candidates with `--explain` and add a
  precise alias, keyword, or globally valid concept.
- If CMake cannot find `yaml-cpp`, install `libyaml-cpp-dev` or set
  `CMAKE_PREFIX_PATH` to a custom dependency prefix.

## Security and trust model

The curated knowledge base is the only answer source. howlinux reads metadata
and Markdown, but never executes their contents or interpolates query text into
shell commands. Contributors must still review every example, quote paths,
mark destructive commands clearly, and keep secrets and private data out of
the repository. See [SECURITY.md](SECURITY.md) for vulnerability reports.

## Contributing and license

Contributions are welcome through issues and pull requests. Read
[CONTRIBUTING.md](CONTRIBUTING.md) first. howlinux is released under the
[MIT License](LICENSE); bundled dependency notices are in
[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
