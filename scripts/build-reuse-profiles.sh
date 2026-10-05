#!/usr/bin/env bash
# No Travelers Pad shield or external grid dependency.
# Travelers Pad shield・外部格子module無しで再利用profileを検査する。
set -euo pipefail
repo_root=$(CDPATH='' cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
source "$repo_root/scripts/module-paths.sh"
example_dir="$repo_root/examples/reuse"
profiles=("$@")
if (( $# == 0 )); then profiles=(driver-only pointer); fi
for profile in "${profiles[@]}"; do
    overlays="$example_dir/driver-only.overlay"
    case "$profile" in
        driver-only) ;;
        pointer) overlays="$overlays;$example_dir/pointer.overlay" ;;
        *) printf 'Unknown reuse profile: %s\n' "$profile" >&2; exit 1 ;;
    esac
    (
        cd "$workspace_root"
        west build -p always -s zmk/app -d "$repo_root/build/reuse/$profile" \
            -b seeeduino_xiao_ble -- -DZMK_CONFIG="$example_dir" \
            -DZMK_EXTRA_MODULES="$repo_root" \
            -DKEYMAP_FILE="$example_dir/minimal.keymap" \
            -DEXTRA_DTC_OVERLAY_FILE="$overlays" \
            -DEXTRA_CONF_FILE="$example_dir/prj.conf"
    )
    config="$repo_root/build/reuse/$profile/zephyr/.config"
    if [[ "$profile" == driver-only ]]; then
        if rg --quiet '^CONFIG_TRAVELERS_PAD_IQS9150_.*=y$' "$config"; then
            printf 'FAIL: function enabled in driver-only profile\n' >&2; exit 1
        fi
    else
        for feature in CONTACT_SESSION POINTER MULTIFINGER_ROUTER TAP_DRAG SCROLL_INERTIA; do
            rg --quiet "^CONFIG_TRAVELERS_PAD_IQS9150_${feature}=y$" "$config"
        done
        if rg --quiet '^CONFIG_TRAVELERS_PAD_IQS9150_TRANSFORM_GESTURE=y$' "$config"; then
            printf 'FAIL: unmapped transform enabled in pointer profile\n' >&2; exit 1
        fi
    fi
done
printf 'PASS: driver-only and/or reusable pointer build profiles\n'
