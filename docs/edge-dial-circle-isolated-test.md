# Edge Dial: Directional Corner Circle Isolated Test / エッジダイヤル: 方向連動コーナー円形単体試験

## Intent / 意図

Circular motion extends an edge action when the useful straight length of that
edge runs out. It is not a separate command: linear and circular motion use the
same virtual KSCAN positions and therefore the same keymap binding (`D`/`U` in
this test).

円形操作は、辺の直線部分を使い切った後も同じ辺操作を続けるためのものです。別コマンド
ではないため、直線と円形は同じ仮想KSCAN位置、したがって本試験では同じ`D`/`U`を
出力します。

While the finger remains in the acquired edge band, it can reverse normally:
down emits `D` and up emits `U`. When it leaves that band for the local circle,
the last direction from within the edge band is captured. Circular direction is
then deliberately ignored: the circle supplies only the amount of continued
input. Consequently, a bend back upward while approaching the circle after a
downward left-edge trace, and every circular movement in that contact, emit only
`D`.

指が取得した辺の帯域内にある間は、通常どおり反転できます。下方向は`D`、上方向は`U`です。
局所円へ向けてその帯域を離れる時点で、辺帯域内における最後の方向を保持します。以後、円の
回転方向は意図的に無視し、円は継続入力の量だけを与えます。そのため左辺を下へなぞった後、
円へ近づくため上へ曲がった場合も、その接触中の円操作でも、`D`だけを出力します。

The local circle centre is selected from the acquired edge and the last
meaningful direction along that edge. It is not fixed to one corner.

局所円の中心は、取得した辺と、その辺に沿った最後の有意な移動方向から選びます。一つの
角に固定しません。

| Acquired edge / 取得辺 | Linear direction / 直線方向 | Circle centre / 円の中心 |
| --- | --- | --- |
| Left / 左 | Down / 下 | Lower-left / 左下 |
| Left / 左 | Up / 上 | Upper-left / 左上 |
| Right / 右 | Down / 下 | Lower-right / 右下 |
| Right / 右 | Up / 上 | Upper-right / 右上 |
| Top / 上 | Left / 左 | Upper-left / 左上 |
| Top / 上 | Right / 右 | Upper-right / 右上 |
| Bottom / 下 | Left / 左 | Lower-left / 左下 |
| Bottom / 下 | Right / 右 | Lower-right / 右下 |

This isolated UF2 enables only the left edge. The configured four local centres
are `(360,313)`, `(1640,313)`, `(360,900)`, and `(1640,900)`.

この単体UF2で有効なのは左辺だけです。4つの局所中心は`(360,313)`、`(1640,313)`、
`(360,900)`、`(1640,900)`です。

## Procedure / 手順

1. Flash `travelers-pad-edge-dial-directional-circle.uf2`, reconnect USB if
   needed, and open a plain text editor.
   `travelers-pad-edge-dial-directional-circle.uf2`を書き込み、必要ならUSBを再接続
   してプレーンテキストエディターを開きます。
2. Start near the upper part of the left edge. Trace downward and confirm `D`
   repeats. At the lower-left area, curve about 80 counts inward and draw a
   small circle: the output remains `D`/`U`, never a new key code.
   左辺の上側から始めて下へなぞり、`D`が繰り返されることを確認します。左下付近で
   約80カウント内側へ曲げ、小さな円を描きます。出力は`D`/`U`のままで、新しいキー
   コードには変わりません。
3. Start again near the lower part of the left edge. Trace upward, then curve
   inward at the upper-left area and make a small circle. It must select the
   upper-left centre instead.
   今度は左辺下側から開始して上へなぞり、左上付近で内側へ曲げて小さな円を描きます。
   この場合は左上中心が選択されることを確認します。
4. Start at the left edge and move horizontally inward before first moving
   vertically. Circle mode must not start because a direction has not yet been
   established.
   左辺から始め、最初に縦方向へ動かさずに内側へ横移動します。辺方向が未確定なので、
   円形モードへ入らないことを確認します。

Pointer, scrolling, tap/drag, multi-finger gestures, and the 3x3 grid remain
disabled. / ポインタ、スクロール、タップ/ドラッグ、多指ジェスチャー、3x3格子は無効です。

## Expected log / 期待ログ

Downward on the left edge:

```
edge dial acquired edge=0
edge dial circle acquired edge=0 direction=1 at=(x,y) center=(360,900)
edge dial mode=circle edge=0 row=0 column=0 at=(x,y)
```

Upward on the left edge changes only the selected centre and direction:

```
edge dial circle acquired edge=0 direction=-1 at=(x,y) center=(360,313)
edge dial mode=circle edge=0 row=0 column=0|1
```

`mode` is diagnostic only; both modes retain `row=0` and the same keymap
bindings. / `mode`は診断用だけで、両モードとも`row=0`、同じキーマップ割当です。

The `at=(x,y)` values are calibration evidence: report the physical location
used for the stroke together with these lines if a circle is misplaced.

`at=(x,y)`は位置校正のための値です。円の位置がずれる場合は、操作した物理的な位置と
この行を合わせて報告してください。
