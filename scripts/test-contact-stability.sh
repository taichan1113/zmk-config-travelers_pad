#!/usr/bin/env bash
# Compile actual sensor helpers and session manager with a host input/arbiter stub.
# 実センサーhelper・session managerをhost input/arbiter stubで検証する。
set -euo pipefail
repo_root=$(CDPATH='' cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
driver_root=${1:-"$repo_root/../../module/zmk-driver-iqs9150"}
features_root=${2:-"$repo_root/../../module/zmk-features-trackpad"}
driver_source="$driver_root/src/driver/iqs9150/iqs9150_ev02_driver.c"
session_source="$features_root/src/function/iqs9150/session/contact_session_manager.c"
fixture_root="$repo_root/tests/contact-stability"
test_directory=$(mktemp -d)
test_binary="$test_directory/contact-test"
trap 'rm -f -- "$test_binary"; rmdir -- "$test_directory"' EXIT
{
    printf '#include "prelude.h"\n'
    printf '#define SESSION_CONFIRM_WAKE 0x7f01\n'
    awk '/^#define IQS9150_REPORT_FINGER1_OFFSET/ {copy=1} /^#define IQS9150_1F_TAP_MASK/ {copy=0} copy {print}' "$driver_source"
    awk '/^static uint8_t decode_contact_slots/ {copy=1} /^static void (report_contact_frame|iqs9150_observe_power)/ {copy=0} copy {print}' "$driver_source"
    awk '/^struct session_data/ {copy=1} /^#define SESSION_INST/ {copy=0} copy {print}' "$session_source"
    printf '#include "cases.h"\n'
} | "${CC:-cc}" -std=c11 -Wall -Wextra -Werror -x c - -I "$fixture_root" \
    -I "$features_root/include" -I "$driver_root/include" -o "$test_binary"
"$test_binary"
