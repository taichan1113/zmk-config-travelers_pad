#!/usr/bin/env bash
# Compile the production state machine against a fake GPIO/I2C/work platform.
# 実driver状態機械をGPIO/I2C/workのスタブと組み合わせて検証する。
set -euo pipefail
repo_root=$(CDPATH='' cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
driver_root=${1:-"$repo_root/../../module/zmk-driver-iqs9150"}
driver_source="$driver_root/src/driver/iqs9150/iqs9150_ev02_driver.c"
fixture_root="$repo_root/tests/iqs9150-recovery"
test_directory=$(mktemp -d)
test_binary="$test_directory/recovery-test"
trap 'rm -f -- "$test_binary"; rmdir -- "$test_directory"' EXIT
[[ -f "$driver_source" ]] || { printf 'Missing driver: %s\n' "$driver_source" >&2; exit 1; }
{
    printf '#include "prelude.h"\n'
    awk '/^#define IQS9150_PRODUCT_NUMBER_REG/ {copy=1} /^static const uint8_t ev02_115c/ {copy=0} copy {print}' "$driver_source"
    printf '#include "stubs.h"\n'
    awk '/^static void iqs9150_fail\(/ {copy=1} /^static void iqs9150_rdy_callback/ {copy=0} copy {print}' "$driver_source"
    printf '#include "cases.h"\n'
} | "${CC:-cc}" -std=c11 -O1 -x c - -I "$fixture_root" -o "$test_binary"
"$test_binary"
