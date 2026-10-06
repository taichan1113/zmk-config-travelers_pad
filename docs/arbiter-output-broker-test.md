## English

# Arbiter output broker hardware test

## Scope

This test verifies that all host-facing relative motion, wheel, and mouse-button
reports from pointer, tap/drag, edge-hold, and inertia use the shared output
broker. The broker rechecks the live primary token, auxiliary delegation, or
post-session lease at the exact emission point.

## Procedure

1. **Pointer and tap
   - Move one finger in the pad centre, then perform one-finger tap, two-finger
     tap, and a stationary one-finger double tap.
   - Expected: cursor, left click, right click, and double click remain the
     same as the previous firmware.

2. **Drag and edge continuation
   - Perform one-finger double-tap + hold + move. While dragging, hold at an
     enabled edge long enough to start continuation, then release.
   - Expected: button is held only during drag; continuation starts after drag
     starts and stops immediately on release.

3. **Scroll, edge continuation, inertia
   - Scroll in each axis with two fingers. For each axis, reach the forward
     edge with either finger and hold. Release both fingers, then touch again
     while inertia is running.
   - Expected: normal scroll, edge continuation, and inertia work; new contact
     stops inertia immediately.

## Log and failure criteria

There must be no `output report failed`, `arbiter output rejected`, `arbiter
frame queue overflow`, or unexpected `session=... cancelled` line. A denied
post-session or auxiliary token after an intentional release/new touch is
normal only when it is followed by the corresponding continuation stopping.

`output report failed`、`arbiter output rejected`、`arbiter frame queue overflow`、

## 日本語

# Arbiter output broker実機試験

## 対象

この試験は、pointer、tap/drag、edge-hold、inertiaが発行するホスト向け相対移動、ホイール、
マウスボタンreportが共有output brokerを通ることを確認する。brokerは実際の出力地点で、
live primary token、auxiliary委譲、またはpost-session leaseを再検査する。

## 手順

1. ポインタとタップ**
1. - パッド中央で1本指ポインタ移動後、1本指タップ、2本指タップ、静止した1本指
     ダブルタップを行う。
   - 期待: カーソル、左クリック、右クリック、ダブルクリックは前ファームと同じ。

2. ドラッグとエッジ継続**
2. - 1本指ダブルタップ＋ホールド＋移動を行う。ドラッグ中に有効辺で継続開始時間以上
     保持し、離す。
   - 期待: buttonはdrag中だけ保持され、継続はdrag開始後だけに始まり、解放時に直ちに止まる。

3. スクロール、エッジ継続、慣性**
3. - 2本指で各軸をスクロールする。各軸でいずれかの指を進行方向の辺へ到達させて
     保持する。両指を離し、慣性中に再接触する。
   - 期待: 通常scroll、edge継続、慣性が動作し、新規接触で慣性が直ちに止まる。

## ログと不合格基準

意図しない`session=... cancelled`が一つでもあれば不合格です。意図的な解放／新規接触後に
post-sessionまたはauxiliary tokenが拒否されることは、その継続が停止する場合に限り正常です。
