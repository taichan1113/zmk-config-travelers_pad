#!/usr/bin/env bash
set -euo pipefail
repo_root=$(CDPATH='' cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
features_root=${1:-"$repo_root/../../module/zmk-features-trackpad"}
fixture="$repo_root/tests/gesture-spacing"
test_dir=$(mktemp -d)
trap 'rm -f -- "$test_dir/test"; rmdir -- "$test_dir"' EXIT
{
  printf '#include "prelude.h"\n'
  awk '/^#define BEHAVIOR_QUEUE_SIZE/ {copy=1} /^int travelers_pad_output_routed_pointer/ {copy=0} copy {print}' \
    "$features_root/src/function/iqs9150/session/contact_output_broker.c"
  printf '#include "cases.h"\n'
} | "${CC:-cc}" -std=c11 -Wall -Wextra -x c - -I "$fixture" -o "$test_dir/test"
"$test_dir/test"
