# Multi-finger tap discrimination BLE test

[English](#english) | [日本語](#日本語)

## English

This candidate includes the pointer release guard and terminal click pacing.
Three-/four-finger swipe recognition now uses only the full contact set; partial
release freezes the last full-set centroid. Completing/reacquiring the full set
rebases uncommitted swipe motion. A committed swipe retains its original direction
through staggered release.

Tip-Tap requires `anchor-min-hold-ms = <250>` of stationary single-finger anchor
before the second finger is added. Movement exceeding `contact-slop` restarts
that interval. A failed two-finger candidate does not count as single-finger hold
time. Three-or-more contacts invalidate the anchor candidate. Already-owned
Tip-Tap repeats do not wait again while the same anchor remains down. The value
is keyboard-configurable; zero explicitly restores immediate acquisition.

Terminal click button-down is 50 ms (`CONFIG_TRAVELERS_PAD_CLICK_PRESS_MS=50`);
the button-up interval remains 5 ms (`CONFIG_TRAVELERS_PAD_CLICK_GAP_MS=5`). These
are input enqueue intervals, not guarantees of BLE arrival times. Live dragging
still follows contact duration, not a 50 ms pulse. The tap contact-duration limit
remains 180 ms, settling allowance 120 ms, and double-tap wait 250 ms. This stage
changes gesture discrimination first; duration tuning requires fresh rejection logs.

Build with `bash scripts/build-click-spacing.sh [features-checkout]`. Run
`test-tip-tap-anchor.sh`, `test-ble-management.sh`, `test-click-spacing.sh`, and
`test-pointer-release.sh` under `scripts/` with the same checkout. They exercise
production code, including staggered release, 249/250 ms anchor boundary,
repeated Tip-Taps, competing ownership, reset, and 5/50 ms click timing.

### Hardware procedure — BLE required

Flash `travelers-pad-multifinger-tap-click50-20261008.uf2` from Windows Downloads.
All host input remains BLE; USB can supply power and serial logging but not HID.
Keep the current bonding unless connection itself fails.

1. Perform 20 stationary two-finger taps and 20 three-finger taps. Deliberately
   vary which finger arrives/lifts first. Expect right/middle clicks, not Tip-Tap
   tab switching or swipe keys. Complete normal taps within the existing 180 ms
   contact limit; repeat naturally and submit failures rather than guessing timing.
2. Hold one finger still for about half a second, then tap a second finger on its
   left/right. Expect Tip-Tap. Keep the anchor down and repeat three times without
   waiting again. Add both fingers nearly together: expect an ordinary two-finger
   tap instead.
3. Confirm genuine three-/four-finger swipes, four-finger management hold, one- and
   two-finger tap dragging, pointer, scroll/inertia, and no release jump.
4. If possible, check Ubuntu word selection and titlebar double-click maximization
   using the 50 ms click candidate.

For failures submit logs around the operation: `tap rejected`, `tap suppressed`,
`Tip-Tap ignored`, `swipe`, `terminal click queued`, and `paced click`. Distinguish
recognition rejection from accepted clicks that the host fails to handle. Hardware
acceptance is pending; the driver and released west revisions are unchanged.

## 日本語

この試験版はポインタ解放時ジャンプ対策とclick送信間隔設定を含みます。
3・4本指スワイプは全接触が揃った間だけ移動を判定し、部分解放中は最後の完全接触時の
重心を保持します。未確定の状態で全指が揃う・揃い直す場合は移動基準を取り直します。
確定済みスワイプは、順番に指を離しても方向を維持します。

Tip-Tapは、先に1本指を `anchor-min-hold-ms = <250>` 静止してから2本目を追加した
場合だけ候補にします。`contact-slop` を超える移動では静止時間を数え直します。
不成立の2本指期間は1本指の静止時間に加算せず、3本以上になれば候補を無効にします。
Tip-Tap確定後に同じanchorを維持して繰り返す操作は待ち直しません。
キーボード側overlayで調整でき、0は従来の即時anchor認定を明示的に選ぶ設定です。

タップclickの押下時間は50ms (`CONFIG_TRAVELERS_PAD_CLICK_PRESS_MS=50`)、
解放間隔は5ms (`CONFIG_TRAVELERS_PAD_CLICK_GAP_MS=5`) です。
BLE到達間隔の保証ではなくinputへの投入間隔です。ドラッグは50msで自動解除せず、
従来どおり接触を離すまで保持します。タップ接触時間上限180ms、複数指の安定化猶予
120ms、ダブルタップ待ち250msは維持しています。まず誤ジェスチャー判定を修正し、
接触時間のチューニングは新しい棄却ログを確認後に行います。

`bash scripts/build-click-spacing.sh [features-checkout]` でビルドします。
`scripts/` 内の `test-tip-tap-anchor.sh`、`test-ble-management.sh`、
`test-click-spacing.sh`、`test-pointer-release.sh` を同じcheckout指定で実行します。
実コードで部分解放、anchorの249/250ms境界、反復Tip-Tap、所有権競合、reset、
5/50msのclick送信を確認します。

### 実機手順 — 必ずBLE

Windows Downloadsの `travelers-pad-multifinger-tap-click50-20261008.uf2` を書き込みます。
操作入力はすべてBLEです。USBを接続しても給電・シリアルログ用途で、HIDには使いません。
接続自体が失敗しない限りボンディングは維持してください。

1. 静止した2本指タップ・3本指タップを各20回。置く・離す指の順番も変えて確認。
   右・中クリックになり、Tip-Tapのタブ切替やスワイプキーに化けないこと。
   通常タップは従来の接触時間上限180ms以内が対象です。体感で自然に操作し、
   失敗分は無理にタイミングを推測せずログを提出してください。
2. 1本指を約0.5秒静止し、左右に2本目をタップするとTip-Tapになること。
   anchorを維持し、待ち直さず3回反復できること。ほぼ同時に2本置いた場合は
   通常の2本指タップになること。
3. 本来の3・4本指スワイプ、4本指管理ホールド、1・2本指タップドラッグ、
   ポインタ、スクロール・慣性、離した際のジャンプがないことを確認。
4. 可能ならUbuntuの単語選択・タイトルバー最大化も50ms押下版で確認。

失敗時は `tap rejected`、`tap suppressed`、`Tip-Tap ignored`、`swipe`、
`terminal click queued`、`paced click` を含む前後ログを提出してください。
認識の棄却と、click受付後にホスト側で動作しない問題を分けて確認します。
実機合格は未判定で、ドライバ・westのリリースrevisionは変更していません。
