# Contributing to howlinux

Contributions are welcome. Please open an issue for a larger change and use a
pull request for implementation or Knowledge updates.

By submitting a contribution, you confirm that you have the right to submit it
and agree that it is provided under this repository's MIT License. Do not copy
code, documentation, or examples from sources with incompatible terms.

## Before opening a pull request

Run the same checks as CI:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/howlinux validate knowledge
```

Maintainers preparing a tag should also run `./scripts/release-audit.sh`.

For Knowledge changes, also test representative queries with `--explain`.
Add durable expected queries to `tests/data/ranking-v2.tsv` and run
`./scripts/evaluate-ranking.sh ./build/howlinux knowledge` when search behavior
or metadata changes.
Do not add secrets, private data, unverified commands, or automatically
generated authoritative answers.

## Review policy

The default branch should be protected. Pull requests require approval from the
repository owner, @LuPetro, and passing CI before merging. Contributors should
not push directly to the default branch.

Repository administrators must enable these settings under GitHub:

1. Settings → Branches → Add branch protection rule for `master`.
2. Require a pull request before merging.
3. Require approvals (at least 1).
4. Require all four `Ubuntu ... / ...` checks from the `CI` workflow.
5. Require branches to be up to date before merging.
6. Restrict who can push to the protected branch, if desired.

CODEOWNERS marks @LuPetro as the required reviewer for all files. GitHub
branch protection is a repository setting and cannot be enabled by files in the
repository alone.

## Commit guidelines

Use a short, descriptive subject, for example:

```text
Add knowledge entry for find
Improve fuzzy query ranking
```

Keep pull requests focused, update tests and public documentation when
behavior changes, and mention any compatibility or security impact.
