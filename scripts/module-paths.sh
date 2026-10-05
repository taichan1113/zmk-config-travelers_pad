#!/usr/bin/env bash
# Shared local checkout paths for regression scripts.
# 回帰script用のローカルcheckout共通パス。west projectのpathに合わせる。
workspace_root=$(CDPATH='' cd -- "$repo_root/../.." && pwd)
driver_module=${TRAVELERS_PAD_DRIVER_MODULE_DIR:-"$workspace_root/module/zmk-driver-iqs9150"}
function_module=${TRAVELERS_PAD_FUNCTION_MODULE_DIR:-"$workspace_root/module/zmk-input-travelers-pad"}
for module_dir in "$driver_module" "$function_module"; do
    if [[ ! -f "$module_dir/zephyr/module.yml" ]]; then
        printf 'FAIL: missing module %s; run west update first\n' "$module_dir" >&2
        exit 1
    fi
done
