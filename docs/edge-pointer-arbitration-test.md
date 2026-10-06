## English

# Edge Dial and Pointer Arbitration Test

## Intent

The edge dial starts as an ordinary one-finger contact. Once it emits its first
virtual key step, it claims that contact from the router until release. The
router sends a single cancellation frame to the pointer, so subsequent edge
motion cannot continue to move the cursor. Two-or-more-finger contacts cannot
be claimed by the edge dial.

## Procedure

1. Flash `travelers-pad-edge-pointer-arbitration.uf2` and open a text editor.
2. Move one finger in the centre. The pointer must move and no letter must be
   typed.
3. Trace an enabled edge until its test letter starts repeating, then continue
   through the local circle. The letter must continue, and the pointer must stop
   after the first edge key step.
4. Make a two-finger scroll gesture near an enabled edge. It must remain a
   normal scroll, never an edge letter.

## Expected log

```text
edge dial mode=linear|circle ...
edge dial owns 1F contact stream
edge dial contact released; router idle
```

The ownership line must not appear for a central one-finger pointer movement or
a two-finger scroll.

## 日本語

# エッジダイヤルとポインタ排他試験

## 意図

エッジダイヤルは通常の1本指接触として開始するが、最初の仮想キーを出力した時点で、
解放までその接触をルーターから取得する。ルーターはポインタへ1回だけ終了フレームを送り、
以後のエッジ移動がカーソルを動かし続けないようにする。2本指以上の接触はエッジダイヤルに
取得されない。

## 手順

1. `travelers-pad-edge-pointer-arbitration.uf2`を書き込み、テキストエディターを開く。
中央を1本指で動かす。ポインタが動き、文字入力がないことを確認する。
有効辺を試験文字が繰り返されるまでなぞり、局所円へ
3. 続ける。文字が継続し、最初のエッジキー出力後はポインタが停止することを確認する。
有効辺の近くで2本指スクロールを行う。通常の
4. スクロールとなり、エッジ文字が出ないことを確認する。

## 期待ログ

```text
edge dial mode=linear|circle ...
edge dial owns 1F contact stream
edge dial contact released; router idle
```

中央1本指ポインタ移動と2本指スクロールでは、所有権ログが出ない。
