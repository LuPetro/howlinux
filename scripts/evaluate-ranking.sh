#!/usr/bin/env bash
set -euo pipefail

if [[ "$#" -lt 2 || "$#" -gt 3 ]]; then
  echo "Usage: $0 BINARY KNOWLEDGE_DIR [DATASET]" >&2
  exit 2
fi

binary="$1"
knowledge="$2"
dataset="${3:-$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)/tests/data/ranking-v2.tsv}"

if [[ ! -x "$binary" ]]; then
  echo "Evaluation binary is not executable: $binary" >&2
  exit 2
fi
if [[ ! -r "$dataset" ]]; then
  echo "Evaluation dataset is not readable: $dataset" >&2
  exit 2
fi

positive_total=0
top_one_correct=0
recall_five_correct=0
negative_total=0
false_confident=0
failures=0

while IFS=$'\t' read -r expected_id query; do
  if [[ -z "${expected_id:-}" || "$expected_id" == \#* ]]; then
    continue
  fi
  if [[ -z "${query:-}" ]]; then
    echo "Dataset row has an empty query for expected ID '$expected_id'." >&2
    failures=$((failures + 1))
    continue
  fi

  set +e
  output="$("$binary" --knowledge "$knowledge" --json -- "$query")"
  command_status=$?
  set -e

  status="$(printf '%s\n' "$output" | sed -n 's/.*"status":"\([^"]*\)".*/\1/p')"
  results="$(printf '%s\n' "$output" | sed -n 's/.*"results":\[\(.*\)\],"entry":.*/\1/p')"
  mapfile -t result_ids < <(printf '%s\n' "$results" | grep -o '"id":"[^"]*"' | sed 's/"id":"\([^"]*\)"/\1/')

  if [[ "$expected_id" == "-" ]]; then
    negative_total=$((negative_total + 1))
    if [[ "$status" == "confident" ]]; then
      false_confident=$((false_confident + 1))
      failures=$((failures + 1))
      echo "False confident match: '$query' -> '${result_ids[0]:-none}'" >&2
    fi
    continue
  fi

  positive_total=$((positive_total + 1))
  if [[ "$command_status" -gt 1 || -z "$status" ]]; then
    failures=$((failures + 1))
    echo "Evaluation error for '$query' (exit $command_status)." >&2
    continue
  fi
  if [[ "${result_ids[0]:-}" == "$expected_id" ]]; then
    top_one_correct=$((top_one_correct + 1))
  else
    failures=$((failures + 1))
    echo "Top-1 mismatch: '$query' expected '$expected_id', got '${result_ids[0]:-none}'." >&2
  fi
  for result_id in "${result_ids[@]:0:5}"; do
    if [[ "$result_id" == "$expected_id" ]]; then
      recall_five_correct=$((recall_five_correct + 1))
      break
    fi
  done
done <"$dataset"

echo "Ranking evaluation:"
echo "  Positive queries: $positive_total"
echo "  Top-1 accuracy: $top_one_correct/$positive_total"
echo "  Recall at five: $recall_five_correct/$positive_total"
echo "  Negative queries: $negative_total"
echo "  False confident matches: $false_confident/$negative_total"

if [[ "$failures" -ne 0 ]]; then
  exit 1
fi
