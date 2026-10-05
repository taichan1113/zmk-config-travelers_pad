#!/usr/bin/env bash
# Structural regression check for Travelers Pad feature-level isolation.
# Travelers Padの機能単位分離に対する構造回帰検査。

set -euo pipefail

repo_root=$(CDPATH='' cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
kconfig="$repo_root/src/function/iqs9150/Kconfig"
cmake="$repo_root/cmake/functions.cmake"

fail() {
    printf 'FAIL: %s\n' "$*" >&2
    exit 1
}

require() {
    local file=$1
    local text=$2

    rg --fixed-strings --quiet -- "$text" "$file" ||
        fail "missing required dependency: $text ($file)"
}

# Session manager is the always-present contact/arbiter foundation. Pointer
# requires it; router requires pointer; observer continuations require the
# three-stage session -> router -> pointer data path.
require "$kconfig" 'IQS9150_EV02_DRIVER && TRAVELERS_PAD_IQS9150_CONTACT_SESSION && DT_HAS_ZMK_TRAVELERS_PAD_IQS9150_POINTER_ENABLED'
require "$kconfig" 'IQS9150_EV02_DRIVER && TRAVELERS_PAD_IQS9150_CONTACT_SESSION && TRAVELERS_PAD_IQS9150_POINTER && DT_HAS_ZMK_TRAVELERS_PAD_IQS9150_MULTIFINGER_ROUTER_ENABLED'
require "$kconfig" 'IQS9150_EV02_DRIVER && TRAVELERS_PAD_IQS9150_CONTACT_SESSION && TRAVELERS_PAD_IQS9150_MULTIFINGER_ROUTER && TRAVELERS_PAD_IQS9150_POINTER && DT_HAS_ZMK_TRAVELERS_PAD_IQS9150_EDGE_HOLD_CONTINUE_ENABLED'
require "$kconfig" 'IQS9150_EV02_DRIVER && TRAVELERS_PAD_IQS9150_CONTACT_SESSION && TRAVELERS_PAD_IQS9150_MULTIFINGER_ROUTER && TRAVELERS_PAD_IQS9150_POINTER && DT_HAS_ZMK_TRAVELERS_PAD_SCROLL_INERTIA_ENABLED'

# All recognizers are session consumers; Tip-Tap must bring in the KSCAN API
# itself instead of relying on another recognizer to select it incidentally.
require "$kconfig" 'IQS9150_EV02_DRIVER && TRAVELERS_PAD_IQS9150_CONTACT_SESSION && DT_HAS_ZMK_TRAVELERS_PAD_IQS9150_TIP_TAP_KSCAN_ENABLED'
require "$kconfig" 'select KSCAN'
require "$kconfig" 'IQS9150_EV02_DRIVER && TRAVELERS_PAD_IQS9150_CONTACT_SESSION && DT_HAS_ZMK_TRAVELERS_PAD_IQS9150_MULTIFINGER_GESTURE_KSCAN_ENABLED'
require "$kconfig" 'IQS9150_EV02_DRIVER && TRAVELERS_PAD_IQS9150_CONTACT_SESSION && DT_HAS_ZMK_TRAVELERS_PAD_IQS9150_TRANSFORM_GESTURE_ENABLED'

# Session-owned sources must remain compiled as a unit. A partial session
# build would leave producers with an arbiter/broker declaration but no owner.
require "$cmake" 'CONFIG_TRAVELERS_PAD_IQS9150_CONTACT_SESSION src/function/iqs9150/session/contact_session_manager.c'
require "$cmake" 'CONFIG_TRAVELERS_PAD_IQS9150_CONTACT_SESSION src/function/iqs9150/session/contact_arbiter.c'
require "$cmake" 'CONFIG_TRAVELERS_PAD_IQS9150_CONTACT_SESSION src/function/iqs9150/session/contact_output_broker.c'

# Every existing diagnostic profile disables transform. The composite driver
# still constructs its child, so the phandle must target the inert mock.
# 診断profileでtransformを外す場合もcomposite childは構築されるためmockへ差し替える。
for overlay in "$repo_root"/tests/feature-matrix/*.overlay; do
    if rg --quiet 'iqs9150_transform_gesture.*status = "disabled"' "$overlay"; then
        require "$overlay" 'transform_gesture { kscan = <&iqs9150_kscan>; };'
    fi
done

printf 'PASS: Travelers Pad feature dependency checks\n'
