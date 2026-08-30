# Contributing to howlinux

Contributions are welcome. Please open an issue for a larger change and use a
pull request for implementation or Knowledge updates.

## Before opening a pull request

Run the same checks as CI:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/howlinux validate knowledge
```

For Knowledge changes, also test representative queries with `--explain`.
Do not add secrets, private data, unverified commands, or automatically
generated authoritative answers.

## Review policy

The default branch is protected. Pull requests require approval from the
repository owner, @LuPetro, and passing CI before merging. Contributors should
not push directly to the default branch.

Repository administrators must enable these settings under GitHub:

1. Settings → Branches → Add branch protection rule for `main`.
2. Require a pull request before merging.
3. Require approvals (at least 1).
4. Require status checks: `build-and-test`.
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
