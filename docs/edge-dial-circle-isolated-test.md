# Edge Dial: Circle Continuation Isolated Test / エッジダイヤル: 円形継続単体試験

## Scope / 対象

This UF2 enables only the left edge and uses a two-row virtual KSCAN layout.
The first row represents linear edge motion; the second represents circular
motion around the pad centre.  This keeps the future keymap actions for the
two modes independent.

このUF2では左辺だけを有効にし、仮想KSCANを2行に分けます。第1行が直線の辺操作、
第2行がパッド中心を基準にした円形操作です。将来、両モードの機能をキーマップで独立して
設定できます。

| Mode / モード | Motion / 操作 | Test output / 試験出力 |
| --- | --- | --- |
| Linear / 直線（行0） | Left edge down / up / 左辺を下 / 上 | `D` / `U` |
| Circle / 円形（行1） | Clockwise / counter-clockwise / 時計 / 反時計 | `C` / `V` |

Pointer, scrolling, tap/drag, multi-finger gestures, and the 3x3 grid stay
disabled.  This profile tests only the edge-dial source.

ポインタ、スクロール、タップ/ドラッグ、多指ジェスチャー、3x3格子は無効のままです。
このプロファイルはエッジダイヤル入力源だけを検証します。

## Procedure / 手順

1. Flash `travelers-pad-edge-dial-circle.uf2`, reconnect USB if necessary, and
   open a plain text editor.
   `travelers-pad-edge-dial-circle.uf2`を書き込みます。必要ならUSBを再接続し、
   プレーンテキストエディターを開きます。
2. Verify the already accepted linear path: start on the middle of the left
   edge and trace vertically. Down repeatedly sends `D`; up repeatedly sends
   `U`.
   合格済みの直線操作を再確認します。左辺中央から縦になぞり、下は`D`、上は`U`が
   繰り返し入力されます。
3. Lift your finger. Start again near the middle of the left edge, then slide
   horizontally to the right by roughly one quarter of the pad width without
   lifting. This moves at least 320 counts inward and switches to circle mode.
   いったん指を離します。再度、左辺中央から始め、指を離さずパッド幅の約1/4だけ右へ
   水平に移動します。内側へ320カウント以上入ると円形モードに切り替わります。
4. Continue holding the same finger and draw a broad circle around the physical
   centre of the pad. Clockwise rotation must repeatedly insert `C`; reverse
   direction and counter-clockwise rotation must repeatedly insert `V`.
   同じ指を保持したまま、パッドの物理的な中心を囲む大きな円を描きます。時計回りでは
   `C`、反時計回りでは`V`が繰り返し入力されることを確認します。
5. Confirm that a touch beginning in the middle, right, top, or bottom area
   emits nothing. A second finger must cancel the edge/circle session.
   中央、右辺、上辺、下辺から始めた接触では何も入力されないことを確認します。2本目の
   指を置いた場合は、辺/円形セッションが解除されます。

## Expected log / 期待ログ

The circle path produces these module lines in sequence:

```
edge dial acquired edge=0
edge dial circle acquired edge=0 center=(1000,606)
edge dial mode=circle edge=0 row=1 column=0   # clockwise / 時計回り
edge dial mode=circle edge=0 row=1 column=1   # counter-clockwise / 反時計回り
```

Linear messages use `mode=linear row=0`. No warning or error from
`travelers_pad_edge_dial` is expected.

直線操作のログは`mode=linear row=0`です。`travelers_pad_edge_dial`由来のwarning/error
は出ないことが期待値です。

## Acceptance / 合格条件

- Linear and circle modes emit only their own KSCAN row.
  / 直線と円形は、それぞれ専用のKSCAN行だけを発火する。
- Circular motion emits repeated events without lifting after the left-edge
  start. / 左辺から始めて指を離さず、円形操作で繰り返し発火する。
- Direction reverses immediately when rotation reverses.
  / 回転方向を反転すると、出力方向も即時に反転する。
- No other isolated-profile input function leaks through.
  / この単体プロファイルへ他の入力機能が混入しない。
