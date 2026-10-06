## English

# Complete-frame routing hardware test

## Scope

This test verifies that the router emits its pointer or two-finger contact
stream only after all primary recognizers have observed the same normalized
frame and the arbiter has selected its owner. It specifically guards against a
callback-registration-order dependency where a pointer frame could leak before
edge, drag, Tip-Tap, or multifinger ownership was known.

## Essential procedure

1. **Centre pointer then two-finger scroll
   - Move one finger in the centre. Without lifting for long, add the second
     finger and make a clear vertical and horizontal scroll.
   - Expected: cursor movement stops as soon as the two-finger classification
     begins; after classification only scroll is emitted. No click occurs.

2. **Edge ownership
   - Start a one-finger trace on each enabled edge and move far enough for at
     least one configured edge-dial step.
   - Expected: the mapped edge action is emitted and the cursor does not make
     an initial movement before the first detent.

3. **Double-tap drag reservation
   - In the centre, double tap, keep the second touch down, then move after a
     short still pause. Repeat with a stationary second touch and release.
   - Expected: the moving case starts one drag without an initial cursor jump;
     the stationary case is a double click. Neither case leaks ordinary
     pointer movement during the reserved contact.

4. **Tip-Tap and multifinger
   - Perform the configured anchored Tip-Tap, a 3F swipe, and a 5F
     Grasp/Release action.
   - Expected: only their mapped key action occurs; no pointer, wheel, or
     click report leaks into the host.

## Expected log and failure criteria

The log may show `2F route=scroll proposed; awaiting frame decision`, followed
by the final `arbiter session=... winner=scroll` line. There must be no
`arbiter output rejected`, `arbiter frame queue overflow`, `contact frame
report failed`, or unexpected `session=... cancelled`. Any pointer motion in
steps 2–4 before the mapped action is also a failure.

## 日本語

# 完全frame routing実機試験

## 対象

この試験は、routerがポインタまたは2本指接触streamを送るのは、全primary recognizerが同じ
正規化frameを観測し、arbiterがownerを選択した後だけであることを確認する。edge、drag、
Tip-Tap、または多指操作の所有権が判明する前にポインタframeが漏れる、callback登録順依存を
防ぐための回帰試験です。

## 重要手順

1. 中央ポインタから2本指scroll**
1. - 中央で1本指ポインタを動かし、長く離さずに2本目を追加して、明確な縦・横scrollを行う。
   - 期待: 2本指分類の開始でポインタ移動が止まり、分類後はscrollだけが出る。クリックはない。

2. エッジ所有権**
2. - 有効な各辺から1本指トレースを開始し、設定済みedge-dial stepが少なくとも一つ出るまで動かす。
   - 期待: 割当済みedge actionが出て、最初のdetent前にカーソルが初動しない。

3. ダブルタップdrag予約**
3. - 中央でダブルタップし、2回目を保持して短い静止後に動かす。次に2回目を静止したまま離す。
   - 期待: 動かした場合は初動ジャンプなしに一つのdragとなり、静止して離した場合はdouble click
     となる。いずれも予約中接触で通常ポインタ移動が漏れない。

4. Tip-Tapと多指操作**
4. - 設定済みのanchor付きTip-Tap、3本指swipe、5本指Grasp/Releaseを行う。
   - 期待: 割当済みキー動作だけが起こり、ポインタ、wheel、クリックreportはホストへ漏れない。

## 期待ログと不合格基準

ログには `2F route=scroll proposed; awaiting frame decision` と、それに続く最終の
`arbiter session=... winner=scroll` が現れ得ます。`arbiter output rejected`、
`arbiter frame queue overflow`、`contact frame report failed`、意図しない
`session=... cancelled` は一つでも不合格です。また、手順2〜4で割当動作より先に
ポインタが動けば不合格です。
