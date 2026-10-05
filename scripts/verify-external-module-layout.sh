#!/usr/bin/env bash
# External ownership checks / 外部moduleの所有境界検査。
set -euo pipefail
repo_root=$(CDPATH='' cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
source "$repo_root/scripts/module-paths.sh"

fail() { printf 'FAIL: %s\n' "$*" >&2; exit 1; }
[[ -f "$driver_module/include/travelers_pad/multitouch.h" ]] || fail 'missing driver contact ABI'
[[ ! -f "$function_module/include/travelers_pad/multitouch.h" ]] || fail 'duplicated contact ABI'
[[ ! -f "$function_module/dts/bindings/input/azoteq,iqs9150.yaml" ]] || fail 'duplicated sensor binding'
[[ -f "$driver_module/dts/bindings/input/azoteq,iqs9150.yaml" ]] || fail 'missing sensor binding'
for local_dir in src/driver src/function include/travelers_pad dts/bindings dts/travelers-pad; do
    if [[ -d "$repo_root/$local_dir" ]] && rg --files "$repo_root/$local_dir" | rg --quiet .; then
        fail "keyboard repository still owns moved implementation: $local_dir"
    fi
done
if rg --quiet 'IQS9150_EV02_DRIVER|TRAVELERS_PAD_IQS9150_' "$repo_root/CMakeLists.txt" "$repo_root/Kconfig"; then
    fail 'keyboard module still builds or declares driver/function symbols'
fi

for module_dir in "$driver_module" "$function_module"; do
    git -C "$module_dir" diff --exit-code --quiet
    git -C "$module_dir" diff --cached --exit-code --quiet
done
printf 'PASS: driver/functions are external, uniquely owned and tracked-clean\n'
