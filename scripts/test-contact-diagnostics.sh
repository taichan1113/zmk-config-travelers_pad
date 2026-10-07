#!/usr/bin/env bash
set -euo pipefail
repo_root=$(CDPATH='' cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
driver_root=${1:-"$repo_root/../../module/zmk-driver-iqs9150"}
test_dir=$(mktemp -d)
trap 'rm -f -- "$test_dir/test"; rmdir -- "$test_dir"' EXIT
{
  printf '#include "prelude.h"\n'
  awk '/^static void observe_contact_report/ {copy=1} /^#endif/ {copy=0} copy {print}' \
    "$driver_root/src/driver/iqs9150/iqs9150_ev02_driver.c"
  printf '#include "cases.h"\n'
} | "${CC:-cc}" -std=c11 -Wall -Wextra -Werror -x c - \
  -I "$repo_root/tests/contact-diagnostics" -o "$test_dir/test"
"$test_dir/test"
