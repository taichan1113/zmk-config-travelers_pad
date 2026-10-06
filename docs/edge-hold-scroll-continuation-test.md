## English

# Edge-hold two-finger scroll continuation test

This stage extends a router-committed, axis-locked two-finger scroll. Its
direction is taken from the existing pointer function's canonical wheel output,
then passes through the same listener scaler as ordinary two-finger scrolling.
Holding at an edge for 120 ms repeats a wheel delta every 24 ms.

## Procedure

1. Flash `travelers-pad-edge-hold-scroll-continuation.uf2`. Confirm the startup
   line includes both `drag delay=120ms ...` and `scroll delay=120ms period=24ms delta=4`.

2. From the centre, perform a clear vertical two-finger scroll until the log
   contains `2F route=scroll committed` and `scroll axis locked: vertical`.
   Without lifting, bring either finger to the forward edge and hold for one
   second. After 120 ms, scroll must continue in the same direction. For a
   vertical axis only top/bottom are eligible; for a horizontal axis only
   left/right are eligible.

3. Move the representative finger out of the edge zone without lifting either
   finger. Repeated scrolling must stop immediately; ordinary physical
   two-finger scrolling must continue. Lift either finger and verify that no
   delayed scrolling remains.

4. Repeat with horizontal scrolling. Confirm `scroll axis locked: horizontal`;
   the continued motion must be horizontal and retain the direction established
   immediately before the edge was reached.

5. While an edge is held, reverse physical finger motion without leaving the
   edge. The continuation direction must not reverse. To reverse it, leave the
   edge, re-enter with the new direction, then hold again.

6. Re-run a one-finger pointer, a drag edge hold, stationary 2F tap,
   pinch/rotation, and 3F swipe. None may acquire scroll continuation except a
   router-committed two-finger scroll.

## Pass criteria

- Repetition starts only after either contact of a committed, axis-locked 2F
  scroll reaches its forward, axis-matching edge.
- Direction remains stable until the edge is left or contact is released.
- Normal and continuation wheel reports receive the same scaler policy.

## 日本語

# エッジホールド・2本指スクロール継続試験

この段階では、router確定済みかつ軸ロック済みの2本指スクロールを拡張する。方向は既存の
ポインタ機能が出力する標準ホイール値から取得し、通常の2本指スクロールと同じlistener
scalerを通す。辺で120 ms保持すると、24 msごとにホイール移動を繰り返す。

## 手順

1. `travelers-pad-edge-hold-scroll-continuation.uf2`を書き込む。起動ログに
   `drag delay=120ms ...` と `scroll delay=120ms period=24ms delta=4` の両方があることを確認する。

2. 中央で明確な縦2本指スクロールを開始し、`2F route=scroll committed` と
   `scroll axis locked: vertical` を確認する。指を離さず、いずれかの指を進行方向側の
   辺へ移動し1秒保持する。120 ms後、同じ方向にスクロールが継続しなければならない。
   縦軸では上／下だけ、横軸では左／右だけが有効である。

3. どちらの指も離さず、代表指を辺のゾーンから戻す。繰返しスクロールは直ちに止まり、
   通常の物理的な2本指スクロールは継続する。いずれかの指を離し、遅延スクロールが
   残らないことを確認する。

4. 横スクロールでも繰り返す。`scroll axis locked: horizontal`を確認し、継続移動が
   横方向で、辺に達する直前に確立した方向を維持することを確認する。

5. 辺を保持中に、辺から出ず物理的な指の移動方向だけを反転する。継続方向は反転しては
   ならない。反転するには辺から出て、新しい方向で再進入してから再び保持する。

6. 1本指ポインタ、ドラッグのエッジ継続、静止2本指タップ、ピンチ／回転、3本指スワイプを
   再試験する。router確定済み2本指スクロール以外はスクロール継続を開始してはならない。

## 合格基準

- 繰返しは、確定済み・軸ロック済みの2本指スクロールのいずれかの指が、軸と進行方向に
  対応する辺へ達した場合だけ開始する。
- 方向は辺から出るか接触を離すまで安定する。
- 通常と継続のホイールレポートは同じscaler方針を受ける。
