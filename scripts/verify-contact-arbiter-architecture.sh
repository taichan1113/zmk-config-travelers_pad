#!/usr/bin/env bash
# Structural regression check for the Travelers Pad contact-arbiter path.
# Travelers Padのcontact-arbiter経路に対する構造回帰検査。
#
# This deliberately checks source-level invariants that a normal firmware build
# cannot prove: one raw subscriber, no retired shared-claim API, primary token
# gates, and observer-only delayed paths. It complements, rather than replaces,
# the hardware procedure in docs/all-input-integration-test.md.
# 通常のファームビルドだけでは証明できない構造不変条件を検査する。すなわち、唯一の
# 生入力購読者、廃止済みshared-claim APIが無いこと、primary token gate、observerだけの
# 遅延経路である。docs/all-input-integration-test.mdの実機試験を置き換えない。

set -euo pipefail

repo_root=$(CDPATH='' cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
overlay="$repo_root/boards/shields/iqs9150_xiao/iqs9150_xiao.overlay"
source_root="$repo_root/src"
include_root="$repo_root/include"

fail() {
    printf 'FAIL: %s\n' "$*" >&2
    exit 1
}

require() {
    local file=$1
    local text=$2

    rg --fixed-strings --quiet -- "$text" "$file" ||
        fail "missing required invariant: $text ($file)"
}

require_source() {
    local relative_path=$1
    local text=$2

    require "$source_root/$relative_path" "$text"
}

raw_subscribers=$(rg --fixed-strings --count 'input-device = <&iqs9150>;' "$overlay" || true)
[[ "$raw_subscribers" == '1' ]] ||
    fail "expected exactly one direct IQS9150 input subscriber, found $raw_subscribers"
require "$overlay" 'iqs9150_session: iqs9150_session'
require "$overlay" 'input-device = <&iqs9150_session>;'

if rg --glob '*.[ch]' --quiet \
    'travelers_pad_session_(claim|tap_allowed|owner)|travelers_pad_tap_drag_is_(active|tracking)|shadow_' \
    "$source_root" "$include_root"; then
    fail 'retired shared-claim/private-recognizer ownership API remains'
fi

require_source 'function/iqs9150/core/iqs9150_multifinger_router.c' \
    'travelers_pad_arbiter_submit'
require_source 'function/iqs9150/core/iqs9150_multifinger_router.c' \
    'travelers_pad_arbiter_authorize_output'
require_source 'function/iqs9150/tap_drag/iqs9150_tap_drag.c' \
    'travelers_pad_arbiter_submit'
require_source 'function/iqs9150/tap_drag/iqs9150_tap_drag.c' \
    'travelers_pad_arbiter_authorize_output'
require_source 'function/iqs9150/tip_tap/iqs9150_tip_tap_kscan.c' \
    'travelers_pad_arbiter_submit'
require_source 'function/iqs9150/tip_tap/iqs9150_tip_tap_kscan.c' \
    'travelers_pad_arbiter_authorize_output'
require_source 'function/iqs9150/multifinger_gesture/iqs9150_multifinger_gesture_kscan.c' \
    'travelers_pad_arbiter_submit'
require_source 'function/iqs9150/multifinger_gesture/iqs9150_multifinger_gesture_kscan.c' \
    'travelers_pad_arbiter_authorize_output'
require_source 'function/iqs9150/edge_dial/iqs9150_edge_dial_kscan.c' \
    'travelers_pad_arbiter_submit'
require_source 'function/iqs9150/edge_dial/iqs9150_edge_dial_kscan.c' \
    'travelers_pad_arbiter_authorize_output'

require_source 'function/iqs9150/edge_hold_continue/iqs9150_edge_hold_continue.c' \
    'travelers_pad_arbiter_authorize_auxiliary_output'
require_source 'function/iqs9150/scroll_inertia/iqs9150_scroll_inertia.c' \
    'travelers_pad_arbiter_owner_for_frame'

printf 'PASS: contact-arbiter structural regression checks\n'
