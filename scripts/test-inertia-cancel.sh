#!/usr/bin/env bash
# Exercise production inertia handlers with deterministic broker cancellation.
# 実慣性handlerを使い、brokerの出力権限取消と次セッションの復帰を検証する。
set -euo pipefail
repo_root=$(CDPATH='' cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
module_root=${1:-"$repo_root/../../module/zmk-features-trackpad"}
source_file="$module_root/src/function/iqs9150/scroll_inertia/iqs9150_scroll_inertia.c"
fixture_root="$repo_root/tests/inertia-cancel"
test_directory=$(mktemp -d)
test_binary="$test_directory/inertia-cancel"
trap 'rm -f -- "$test_binary"; rmdir -- "$test_directory"' EXIT
[[ -f "$source_file" ]] || { printf 'Missing source: %s\n' "$source_file" >&2; exit 1; }
{
    printf '#include "prelude.h"\n'
    awk '/^struct scroll_inertia_config/ {copy=1} /^static int inertia_init/ {copy=0} copy {print}' "$source_file"
    printf '#include "cases.h"\n'
} | "${CC:-cc}" -std=c11 -O1 -Werror=implicit-function-declaration -x c - \
    -I "$fixture_root" -I "$module_root/include" -o "$test_binary"
"$test_binary"
