#!/usr/bin/env bash
set -euo pipefail
repo_root=$(CDPATH='' cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
features_root=${1:-"$repo_root/../../module/zmk-features-trackpad"}
fixture="$repo_root/tests/pointer-release"
test_dir=$(mktemp -d)
trap 'rm -f -- "$test_dir/test"; rmdir -- "$test_dir"' EXIT
source_file="$features_root/src/function/iqs9150/core/iqs9150_pointer.c"
{
  printf '#include "prelude.h"\n'
  awk '/^struct pointer_config/ {copy=1} /^static int32_t absolute_int32/ {copy=0} copy {print}' "$source_file"
  printf '#include "transport.h"\n'
  awk '/^static void process_frame/ {copy=1} /^static int pointer_init/ {copy=0} copy {print}' "$source_file"
  awk '/^static void pointer_handle_input/ {copy=1} /^#define POINTER_INST/ {copy=0} copy {print}' "$source_file"
  printf '#include "cases.h"\n'
} | "${CC:-cc}" -std=c11 -Wall -Wextra -x c - -I "$fixture" -o "$test_dir/test"
"$test_dir/test"
