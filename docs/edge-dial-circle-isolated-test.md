# Edge Dial: Local Circle Continuation Isolated Test / エッジダイヤル: 局所円形継続単体試験

## Intent / 意図

Circular movement is not another command. It is an alternative path that lets
one edge action continue after the usable straight section of an edge runs out.
Linear and circular motion therefore emit the **same** virtual KSCAN positions.

円形操作は別コマンドではありません。辺の直線部分を使い切った後も、同じ辺操作を継続
するための代替軌道です。このため直線と円形は**同じ**仮想KSCAN位置を発火します。

Only the left edge is enabled in this UF2. Downward/right-handed positive motion
emits `D`; reverse motion emits `U`. The local circle centre is `(360,900)`,
left and below the physical pad centre, so a stroke from the upper left down the
left edge can curve into a small lower-left circle without returning to centre.

このUF2で有効なのは左辺だけです。下方向/正方向は`D`、逆方向は`U`です。局所円の
中心は`(360,900)`、すなわちパッド中央より左下に置きます。左上から左辺を下へなぞった
後、中央まで戻らずに左下の小さな円へ自然に曲げられます。

Pointer, scrolling, tap/drag, multi-finger gestures, and the 3x3 grid remain
disabled. / ポインタ、スクロール、タップ/ドラッグ、多指ジェスチャー、3x3格子は無効です。

## Procedure / 手順

1. Flash `travelers-pad-edge-dial-local-circle.uf2`, reconnect USB if needed,
   and open a plain text editor.
   `travelers-pad-edge-dial-local-circle.uf2`を書き込み、必要ならUSBを再接続して
   プレーンテキストエディターを開きます。
2. Start at the upper portion of the left edge and trace downward. `D` should
   repeat as in the accepted linear test.
   左辺の上側から開始して下へなぞります。合格済み直線試験と同様に`D`が繰り返し
   入力されます。
3. Without lifting, when reaching the lower-left area, curve about 80 counts to
   the right and draw a small circle around that lower-left area. The output
   remains `D` or `U`; it must never change to another key code merely because
   the path became circular.
   指を離さず、左下付近で80カウント程度だけ右へ曲げ、左下の周りに小さな円を描きます。
   出力は`D`または`U`のままであり、円形になっただけで別キーへ変わらないことを確認します。
4. Reverse the rotational direction. The output direction must change promptly
   between `D` and `U`.
   回転方向を反転します。`D`と`U`の出力方向がすみやかに反転することを確認します。
5. Start touches away from the left edge, or add a second finger. No key must be
   emitted; the latter cancels an active edge session.
   左辺以外から接触を開始するか、2本目の指を追加します。キーを発火しないこと、後者は
   アクティブな辺セッションを解除することを確認します。

## Expected log / 期待ログ

```
edge dial acquired edge=0
edge dial mode=linear edge=0 row=0 column=0
edge dial circle acquired edge=0 center=(360,900)
edge dial mode=circle edge=0 row=0 column=0
edge dial mode=circle edge=0 row=0 column=1
```

`mode` is diagnostic only; both modes use `row=0`, and thus the same keymap
bindings. No warning/error from `travelers_pad_edge_dial` is expected.

`mode`は診断用だけで、両モードとも`row=0`、したがって同じキーマップ割当です。
`travelers_pad_edge_dial`由来のwarning/errorは期待しません。
