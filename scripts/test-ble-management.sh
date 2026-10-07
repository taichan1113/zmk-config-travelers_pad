#!/usr/bin/env bash
# Extract production recognizer/gate; platform stubs only.
# 実recognizer/gateを抽出し、プラットフォームだけスタブ化する。
set -euo pipefail
repo_root=$(CDPATH='' cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
features_root=${1:-"$repo_root/../../module/zmk-features-trackpad"}
fixture="$repo_root/tests/ble-management"
test_dir=$(mktemp -d)
trap 'rm -f -- "$test_dir/hold" "$test_dir/gate"; rmdir -- "$test_dir"' EXIT
{
  printf '#include "hold-prelude.h"\n'
  awk '/^enum gesture_column/ {copy=1} /^static void gesture_handle_input/ {copy=0} copy {print}' \
    "$features_root/src/function/iqs9150/multifinger_gesture/iqs9150_multifinger_gesture_kscan.c"
  printf '#include "hold-cases.h"\n'
} | "${CC:-cc}" -std=c11 -O1 -x c - -I "$fixture" -o "$test_dir/hold"
"$test_dir/hold"
{
  printf '#include "gate-prelude.h"\n'
  awk '/^struct gate_config/ {copy=1} /^static struct zmk_input_processor_driver_api/ {copy=0} copy {print}' \
    "$repo_root/src/platform/xiao/management_gate.c"
  printf '#include "gate-cases.h"\n'
} | "${CC:-cc}" -std=c11 -O1 -x c - -I "$fixture" -o "$test_dir/gate"
"$test_dir/gate"
