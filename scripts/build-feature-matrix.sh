#!/usr/bin/env bash
# Build every supported Travelers Pad diagnostic profile from a ZMK workspace.
# ZMK workspace内で対応するTravelers Pad診断profileをすべてbuildする。

set -euo pipefail

repo_root=$(CDPATH='' cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
workspace_root=$(CDPATH='' cd -- "$repo_root/../.." && pwd)
config_dir="$repo_root/config"
matrix_dir="$repo_root/tests/feature-matrix"

[[ -d "$workspace_root/zmk/app" ]] || {
    printf 'FAIL: run this repository from a ZMK workspace (missing %s/zmk/app)\n' "$workspace_root" >&2
    exit 1
}
[[ -d "$workspace_root/module/zmk-input-matrix" ]] || {
    printf 'FAIL: missing required module %s/module/zmk-input-matrix\n' "$workspace_root" >&2
    exit 1
}

extra_modules="$repo_root;$workspace_root/module/zmk-input-matrix"
keymap="$config_dir/iqs9150_xiao.keymap"

build_profile() {
    local profile=$1
    local overlay=$matrix_dir/$profile.overlay
    local dts_overlays="$keymap;$overlay"

    printf '==> Building %s\n' "$profile"
    (
        cd "$workspace_root"
        west build -p always -s zmk/app -d "$repo_root/build/feature-matrix/$profile" \
            -b seeeduino_xiao_ble -- -DSHIELD=iqs9150_xiao -DZMK_CONFIG="$config_dir" \
            -DZMK_EXTRA_MODULES="$extra_modules" -DDTC_OVERLAY_FILE="$dts_overlays"
    )
}

if (( $# )); then
    profiles=("$@")
else
    profiles=(router-pointer router-pointer-observers router-tap-pointer tap-drag multifinger-gesture tip-tap edge-dial transform-pinch-only transform-rotation-only transform-both-disabled transform-both-enabled inertia-legacy)
fi

for profile in "${profiles[@]}"; do
    [[ -f "$matrix_dir/$profile.overlay" ]] || {
        printf 'FAIL: unknown feature profile: %s\n' "$profile" >&2
        exit 1
    }
    build_profile "$profile"
done

printf 'PASS: Travelers Pad feature build matrix\n'
