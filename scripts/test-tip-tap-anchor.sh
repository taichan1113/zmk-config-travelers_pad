#!/usr/bin/env bash
set -euo pipefail
repo_root=$(CDPATH='' cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
features_root=${1:-"$repo_root/../../module/zmk-features-trackpad"}
fixture="$repo_root/tests/tip-tap-anchor"
test_dir=$(mktemp -d)
trap 'rm -f -- "$test_dir/test"; rmdir -- "$test_dir"' EXIT
{
  printf '#include "prelude.h"\n'
  awk '/^struct cfg/ {copy=1} /^static int configure/ {copy=0} copy {print}' \
    "$features_root/src/function/iqs9150/tip_tap/iqs9150_tip_tap_kscan.c"
  printf '#include "cases.h"\n'
} | "${CC:-cc}" -std=c11 -Wall -Wextra -x c - -I "$fixture" -o "$test_dir/test"
"$test_dir/test"
