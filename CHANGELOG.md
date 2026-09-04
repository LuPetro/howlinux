# Changelog

All notable changes to this project are documented here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and releases use
[Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.1] - 2026-09-04

### Added

- Expanded the curated knowledge base from 5 to 38 entries covering common
  filesystem, text, storage, process, networking, systemd, log, package, and
  archive workflows.
- Added safety-focused examples, metadata, cross-references, and broader query
  vocabulary for the new and existing entries.
- Added a versioned ranking evaluation dataset with positive and negative
  queries plus automated top-1, recall-at-five, and false-confidence checks.

### Changed

- Extended smoke and release audits to exercise the expanded knowledge base
  and ranking evaluation.

## [1.0.0] - 2026-09-02

### Added

- Offline, deterministic search over curated YAML and Markdown knowledge.
- Recursive loading with schema, duplicate-ID, concept, and reference checks.
- Explainable ranking with aliases, phrases, commands, keywords, concepts,
  intent, IDF-weighted tokens, and bounded typo matching.
- Conservative confident, uncertain, and no-match result policy.
- Search, list, show, and validate commands with stable JSON output.
- CMake installation, shell completions, man page, test suite, and TGZ release
  packaging with SHA-256 checksums.

[1.0.1]: https://github.com/LuPetro/howlinux/compare/v1.0.0...v1.0.1
[1.0.0]: https://github.com/LuPetro/howlinux/releases/tag/v1.0.0
