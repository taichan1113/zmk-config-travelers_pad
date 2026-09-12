#!/usr/bin/env bash
# Structural regression check for the Travelers Pad contact-arbiter path.
# Travelers Padのcontact-arbiter経路に対する構造回帰検査。
#
# This deliberately checks source-level invariants that a normal firmware build
# cannot prove: one raw subscriber, no retired shared-claim API, complete-frame
# routing, primary token gates, observer-only delayed paths, and a single
# host-facing REL/KEY transport point. It complements, rather than replaces, the hardware procedure in
# docs/all-input-integration-test.md.
# 通常のファームビルドだけでは証明できない構造不変条件を検査する。すなわち、唯一の
# 生入力購読者、廃止済みshared-claim APIが無いこと、完全frame後のrouting、primary token
# gate、observerだけの遅延経路、ホスト向けREL/KEY transportの一元化である。docs/all-input-integration-test.mdの
# 実機試験を置き換えない。

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
require_source 'function/iqs9150/core/iqs9150_multifinger_router.c' \
    'router_deliver_resolved_frame'
require_source 'function/iqs9150/core/iqs9150_multifinger_router.c' \
    'travelers_pad_arbiter_bind_frame_handler'
require_source 'function/iqs9150/session/contact_arbiter.c' \
    'dispatch_frame_handlers'
require_source 'function/iqs9150/session/contact_arbiter.c' \
    'travelers_pad_arbiter_frame_handler_t'
require_source 'function/iqs9150/session/contact_arbiter.c' \
    'primary_registered_mask'
require_source 'function/iqs9150/session/contact_arbiter.c' \
    'is_primary_recognizer'
require_source 'function/iqs9150/session/contact_arbiter.c' \
    'frame->resolved'
require_source 'function/iqs9150/session/contact_arbiter.c' \
    'frame_handler_mask'
require_source 'function/iqs9150/session/contact_arbiter.c' \
    'dispatch_frame_handler'

if rg --fixed-strings --quiet -- 'state.provisional' "$source_root/function/iqs9150/session/contact_arbiter.c"; then
    fail 'retired provisional-owner state remains in arbiter'
fi
if rg --glob '*.[ch]' --quiet -- 'POINTER_PROVISIONAL|TOKEN_PROVISIONAL' \
    "$source_root" "$include_root"; then
    fail 'retired provisional ownership vocabulary remains in live source'
fi
require_source 'function/iqs9150/tap_drag/iqs9150_tap_drag.c' \
    'travelers_pad_arbiter_submit'
require_source 'function/iqs9150/tap_drag/iqs9150_tap_drag.c' \
    'travelers_pad_output_primary'
require_source 'function/iqs9150/tip_tap/iqs9150_tip_tap_kscan.c' \
    'travelers_pad_arbiter_submit'
require_source 'function/iqs9150/tip_tap/iqs9150_tip_tap_kscan.c' \
    'travelers_pad_output_behavior_primary'
require_source 'function/iqs9150/multifinger_gesture/iqs9150_multifinger_gesture_kscan.c' \
    'travelers_pad_arbiter_submit'
require_source 'function/iqs9150/multifinger_gesture/iqs9150_multifinger_gesture_kscan.c' \
    'travelers_pad_output_behavior_primary'
require_source 'function/iqs9150/edge_dial/iqs9150_edge_dial_kscan.c' \
    'travelers_pad_arbiter_submit'
require_source 'function/iqs9150/edge_dial/iqs9150_edge_dial_kscan.c' \
    'travelers_pad_output_behavior_primary'

require_source 'function/iqs9150/edge_hold_continue/iqs9150_edge_hold_continue.c' \
    'travelers_pad_output_auxiliary'
require_source 'function/iqs9150/scroll_inertia/iqs9150_scroll_inertia.c' \
    'travelers_pad_arbiter_acquire_post_session_lease'
require_source 'function/iqs9150/scroll_inertia/iqs9150_scroll_inertia.c' \
    'travelers_pad_output_post_session'
require_source 'function/iqs9150/scroll_inertia/iqs9150_scroll_inertia.c' \
    'inertia_deliver_resolved_frame'
require_source 'function/iqs9150/scroll_inertia/iqs9150_scroll_inertia.c' \
    'travelers_pad_arbiter_bind_frame_handler'
require_source 'function/iqs9150/edge_hold_continue/iqs9150_edge_hold_continue.c' \
    'edge_hold_deliver_resolved_frame'
require_source 'function/iqs9150/edge_hold_continue/iqs9150_edge_hold_continue.c' \
    'travelers_pad_arbiter_bind_frame_handler'
require_source 'function/iqs9150/tap_drag/iqs9150_tap_drag.c' \
    'travelers_pad_arbiter_activate_owner'
require_source 'function/iqs9150/session/contact_arbiter.c' \
    'TRAVELERS_PAD_OWNER_PHASE_ACTIVE'
require_source 'function/iqs9150/session/contact_arbiter.c' \
    'arbiter post-session lease'
require_source 'function/iqs9150/session/contact_arbiter.c' \
    'snapshot->cancelled'
require_source 'function/iqs9150/core/iqs9150_multifinger_router.c' \
    'retaining scroll owner'
require_source 'function/iqs9150/session/contact_output_broker.c' \
    'travelers_pad_arbiter_authorize_output'
require_source 'function/iqs9150/session/contact_output_broker.c' \
    'travelers_pad_arbiter_authorize_auxiliary_output'
require_source 'function/iqs9150/session/contact_output_broker.c' \
    'travelers_pad_arbiter_authorize_post_session_output'
require_source 'function/iqs9150/session/contact_output_broker.c' \
    'travelers_pad_output_behavior_primary'
require_source 'function/iqs9150/session/contact_output_broker.c' \
    'held_button_record'
require_source 'function/iqs9150/session/contact_output_broker.c' \
    'remember_held_button'
require_source 'function/iqs9150/session/contact_output_broker.c' \
    'take_held_button'
require_source 'function/iqs9150/session/contact_arbiter.c' \
    'travelers_pad_arbiter_issue_terminal_action'
require_source 'function/iqs9150/session/contact_arbiter.c' \
    'travelers_pad_arbiter_consume_terminal_action'
require_source 'function/iqs9150/session/contact_output_broker.c' \
    'travelers_pad_arbiter_consume_terminal_action'
require_source 'function/iqs9150/tap_drag/iqs9150_tap_drag.c' \
    'travelers_pad_output_terminal_click'
require_source 'function/iqs9150/tap_drag/iqs9150_tap_drag.c' \
    'resolved terminal owner'

if rg --fixed-strings --quiet -- 'travelers_pad_output_terminal_tap' "$source_root" "$include_root"; then
    fail 'retired ungranted terminal tap transport remains'
fi

# Initial ATI can be interrupted by an early touch. Retain the bounded automatic
# recovery path so this condition cannot regress into a permanent ERROR state.
# 初回ATI中の早すぎる接触を、恒久ERRORに戻さないための上限付き自動復旧経路を確認する。
require_source 'driver/iqs9150/iqs9150_ev02_driver.c' \
    'IQS9150_ATI_RECOVERY_RETRY_MAX'
require_source 'driver/iqs9150/iqs9150_ev02_driver.c' \
    'startup ATI error'
require_source 'driver/iqs9150/iqs9150_ev02_driver.c' \
    'IQS9150_RECOVER_RESET'
require_source 'driver/iqs9150/iqs9150_ev02_driver.c' \
    'complete command is 0x0002'

# Semantic REL/KEY output must not bypass the broker. Router/session/grid ABS
# forwarding remains intentionally outside this assertion.
# semanticなREL/KEY出力はbrokerを迂回してはならない。router/session/gridのABS転送は
# 意図的にこの検査の対象外である。
for producer in \
    'function/iqs9150/core/iqs9150_pointer.c' \
    'function/iqs9150/tap_drag/iqs9150_tap_drag.c' \
    'function/iqs9150/edge_hold_continue/iqs9150_edge_hold_continue.c' \
    'function/iqs9150/scroll_inertia/iqs9150_scroll_inertia.c'; do
    if rg --quiet 'input_report_(rel|key)' "$source_root/$producer"; then
        fail "semantic REL/KEY output bypasses contact output broker: $producer"
    fi
done

# Primary virtual-KSCAN actions must not split token authorization from the
# callback pair. The optional grid adapter is intentionally outside this check.
# primary仮想KSCAN actionはtoken認可とcallback対を分離してはならない。任意grid
# adapterは意図的にこの検査対象外とする。
for producer in \
    'function/iqs9150/tip_tap/iqs9150_tip_tap_kscan.c' \
    'function/iqs9150/multifinger_gesture/iqs9150_multifinger_gesture_kscan.c' \
    'function/iqs9150/edge_dial/iqs9150_edge_dial_kscan.c'; do
    if rg --quiet 'callback\([^\n]*(true|false)' "$source_root/$producer"; then
        fail "primary virtual-KSCAN output bypasses contact output broker: $producer"
    fi
done

printf 'PASS: contact-arbiter structural regression checks\n'
