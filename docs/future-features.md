# Post-v1 roadmap

This document records possible work after v1.0.0. It is not a promise or a
release requirement. Any proposal must preserve the core contract: curated
answers, deterministic behavior, offline operation, no command execution, and
no hidden telemetry.

Priority meanings:

- **P1:** practical next steps with clear value and limited architectural risk
- **P2:** medium-term work that needs a design proposal and measurements
- **P3:** experiments that require evidence before becoming product features

## P1 - practical extensions

### Persistent versioned index

Large knowledge bases may benefit from an optional compiled index. Its format
would need a magic header, schema version, engine compatibility, source
fingerprint, checksum, deterministic serialization, atomic replacement, and a
safe fallback to rebuilding in memory. The YAML and Markdown sources remain
authoritative and must never be deleted after compilation.

Add this only after benchmarks show that startup time or memory is a real
problem.

### Distribution and installation

Potential additions include signed checksums, distribution packages, more
architectures, and documented uninstall/upgrade behavior. Each artifact must
come from tested tagged source and retain the license and third-party notices.

### Authoring and lint tools

Useful checks include duplicate or overly similar aliases, overly broad
keywords, unused concepts, Markdown link/code-fence validation, placeholder
quality, and risky command patterns. Lint must be deterministic and must not
silently rewrite reviewed content.

### Ranking evaluation

Create a versioned dataset of expected top results for positive, ambiguous,
typo, and negative queries. Measure top-1 accuracy, recall at five, false
confident matches, and latency before changing weights or confidence
thresholds.

## P2 - medium-term extensions

### Versioned knowledge packages

Separate content packages could have a manifest, package ID, semantic version,
language, engine compatibility, checksums, deterministic archives, signatures,
and explicit selection. Download or update operations must be separate and
opt-in; the runtime remains offline.

### More precise lexical search

Candidates include configurable field weights, explicit keyword weights,
field-aware BM25, better compound-word and Unicode handling, language-specific
stopwords, cautious stemming, selected Markdown section indexing, and bounded
snippets. Every signal must remain deterministic and visible in `--explain`.

### Curated multilingual knowledge

Language-specific packages could provide human-reviewed content,
normalization, concepts, and an explicit fallback language. Validation must
detect missing or stale translations. Machine translation may create a draft,
but unreviewed translated text must never be presented as authoritative.

### Reusable library and interfaces

A stable core API could support an interactive terminal selector, read-only
TUI, local editor integrations, and distribution tooling. No interface may run
shell commands without a separate, explicit security design and user consent.

### Safe local reload

Very large or frequently edited knowledge bases may need change detection,
incremental indexing, consistent snapshots, rollback to the last valid state,
and measurable resource limits. A partially loaded state must never be visible.

## P3 - experiments

### Optional embedding ranking

Embeddings may only be an optional stage behind a stable interface. Prefer a
local model, keep lexical and fuzzy fallbacks, version the model and vector
format, expose semantic scores, and never allow a vague semantic match to
override an exact command or alias. Evaluate false confident matches and
resource use before adoption.

Embeddings may select reviewed entries; they must not generate or modify answer
text.

### Learned hybrid ranking

A learned ranker requires a sufficiently large, curated, versioned evaluation
set; reproducible training without user telemetry; an exported versioned
model; inspectable signals; and a deterministic fallback. Keep the rule-based
ranker unless measurements demonstrate a meaningful improvement.

### Knowledge graph

`related` and concepts might become a typed, validated graph with orphan and
cycle checks, nearby-topic suggestions, and a bounded ranking bonus. Graph
proximity must never make an irrelevant result confident.

### Local privacy-preserving diagnostics

An optional local-only mode might summarize no-match queries. It must be off by
default, make no network requests, document storage and deletion, redact
potentially sensitive input, and require explicit consent.

## Outside the roadmap

The following would violate the project contract:

- unreviewed generative answers as default behavior;
- automatic execution of displayed commands;
- execution of query, YAML, or Markdown content;
- required cloud services or runtime network access;
- hidden telemetry;
- a required external vector database;
- replacing editable YAML/Markdown sources with an opaque binary format.

Every future design must retain the ability to add a reviewed knowledge entry
without changing C++ code.
