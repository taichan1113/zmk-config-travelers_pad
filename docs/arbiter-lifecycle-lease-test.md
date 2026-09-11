# Arbiter lifecycle and lease hardware test / Arbiter lifecycle・lease実機試験

## Scope / 対象

This focused hardware test verifies the first transaction-arbiter migration
slice: cancellation-aware token revocation, `DRAG_RESERVED` versus
`DRAG_ACTIVE`, normal-end-only scroll inertia, and a committed scroll that
keeps its logical two-contact route if a third finger arrives.

この重点実機試験は、transaction arbiter移行の最初のsliceを確認する。対象はcancellationを
考慮したtoken失効、`DRAG_RESERVED` と `DRAG_ACTIVE`、通常終了だけのscroll inertia、
scroll確定後に3本目が来ても論理2点routeを維持することです。

## Procedure / 手順

1. **Stationary double tap / 静止ダブルタップ**
   - In the centre of the pad, make a one-finger double tap and keep the second
     touch still for at least 150 ms, then release.
   - パッド中央で1本指ダブルタップし、2回目を150 ms以上静止保持して離す。
   - Expected: a double click only. No `arbiter owner active ... owner=drag`
     line may appear because button-down never succeeded.
   - 期待: double clickだけ。button-downが成功しないため、`arbiter owner active ...
     owner=drag` は出てはならない。

   The standard full profile cannot make a valid stationary drag candidate in
   the edge-hold band: the wider enabled edge-dial region correctly owns that
   first contact. The negative edge-hold-before-activation condition is
   therefore structurally checked in this profile; a dedicated edge-dial-off
   fixture will cover it in the later feature build matrix.

   標準の全機能profileでは、edge-hold帯域で静止した有効drag候補を作れない。より広い
   有効edge-dial領域が最初の接触を正しく所有するためです。そのため、このprofileでは
   drag開始前edge-hold禁止を構造検査で確認し、後続のfeature build matrixでedge-dial無効の
   専用fixtureを用意して負の条件を実機確認します。

2. **Active drag edge continuation / 有効dragの辺継続**
   - Repeat the double tap, move past the drag threshold, then hold at the
     matching edge for more than 150 ms.
   - 同じダブルタップ後、drag閾値を越えて移動し、対応する辺で150 ms以上保持する。
   - Expected: `arbiter owner active ... owner=drag` precedes
     `drag continuation started`; the held item moves continuously and stops
     immediately on release.
   - 期待: `arbiter owner active ... owner=drag` が `drag continuation started`
     より先に出る。掴んだ対象は連続移動し、離すと直ちに止まる。

3. **Normal scroll inertia / 通常scrollの慣性**
   - Scroll with two fingers, lift both fingers, and touch again while inertia
     is running.
   - 2本指scroll後に両指を離し、慣性中に再接触する。
   - Expected log order: `arbiter post-session lease`, `scroll inertia pending
     normal-end lease`, `scroll inertia lease acquired`. A new touch stops the
     glide immediately.
   - 期待ログ順: `arbiter post-session lease`、`scroll inertia pending normal-end
     lease`、`scroll inertia lease acquired`。新規接触で慣性が直ちに止まる。

4. **Third finger during committed scroll / 確定scroll中の3本目**
   - Start axis-locked two-finger scrolling, add a third finger without lifting
     the original two, keep moving, then remove the third finger and finish.
   - 軸ロック済み2本指scroll中に、元の2本を離さず3本目を加え、移動を継続してから
     3本目を離し、最後に終了する。
   - Expected: scrolling continues with no 3F gesture action. The log contains
     `added during committed scroll; retaining scroll owner` and has no
     `cancelled scroll` line.
   - 期待: 3本指gestureを発火せずscrollが継続する。ログに
     `added during committed scroll; retaining scroll owner` があり、
     `cancelled scroll` がない。

## Failure criteria / 不合格基準

Fail this slice on `arbiter output rejected`, `arbiter frame queue overflow`,
unexpected `session=... cancelled`, a continuation before drag activation,
inertia after a reset/cancellation, or loss of scroll after adding the third
finger.

`arbiter output rejected`、`arbiter frame queue overflow`、意図しない
`session=... cancelled`、drag開始前の継続、reset/cancellation後の慣性、3本目追加後の
scroll停止が一つでもあれば、このsliceは不合格です。
