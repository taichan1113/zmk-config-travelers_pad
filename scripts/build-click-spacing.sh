#!/usr/bin/env bash
# Build the local test module without changing the released west revisions.
# 公開済みwest revisionを変えず、ローカル試験moduleでビルドする。
set -euo pipefail
repo_root=$(CDPATH='' cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
workspace_root=$(CDPATH='' cd -- "$repo_root/../.." && pwd)
features_root=${1:-"$workspace_root/module/zmk-features-trackpad"}
features_root=$(CDPATH='' cd -- "$features_root" && pwd)
rg -q 'config TRAVELERS_PAD_CLICK_PRESS_MS' "$features_root/src/function/iqs9150/Kconfig" || {
  printf 'Select the test/terminal-click-spacing features checkout first.\n' >&2
  exit 1
}
cd "$workspace_root"
nix develop --command west build -s zmk/app \
  -d "$repo_root/build/click-spacing-5ms" -b seeeduino_xiao_ble -S zmk-usb-logging -- \
  '-DSHIELD=iqs9150_xiao rgbled_adapter' "-DZMK_CONFIG=$repo_root/config" \
  "-DZMK_EXTRA_MODULES=$repo_root;$features_root"
