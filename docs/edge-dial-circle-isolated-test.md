## English

# Edge Dial: Directional Corner Circle Isolated Test

## Intent

Circular motion extends an edge action when the useful straight length of that
edge runs out. It is not a separate command: linear and circular motion use the
same virtual KSCAN positions and therefore the same keymap binding (`D`/`U` in
this test).

While the finger remains in the acquired edge band, it can reverse normally:
down emits `D` and up emits `U`. When it leaves that band for the local circle,
the last direction from within the edge band is captured. Circular direction is
then deliberately ignored: the circle supplies only the amount of continued
input. Consequently, a bend back upward while approaching the circle after a
downward left-edge trace, and every circular movement in that contact, emit only
`D`.

The local circle centre is selected from the acquired edge and the last
meaningful direction along that edge. It is not fixed to one corner.

| Acquired edge| Linear direction| Circle centre|
| --- | --- | --- |
| Left| Down| Lower-left|
| Left| Up| Upper-left|
| Right| Down| Lower-right|
| Right| Up| Upper-right|
| Top| Left| Upper-left|
| Top| Right| Upper-right|
| Bottom| Left| Lower-left|
| Bottom| Right| Lower-right|

This isolated UF2 enables only the left and bottom edges. The configured four local centres are
`(360,313)`, `(1640,313)`, `(360,900)`, and `(1640,900)`.

## Test bindings

| Edge| Positive direction| Negative direction|
| --- | --- | --- |
| Left| Down| Up|
| Right| Down| Up|
| Top| Right| Left|
| Bottom| Right| Left|

## Procedure

1. Flash `travelers-pad-edge-dial-selectable-edges.uf2`, reconnect USB if
   needed, and open a plain text editor.
2. Start near the upper part of the left edge. Trace downward and confirm `D`
   repeats. At the lower-left area, curve about 80 counts inward and draw a
   small circle: the output remains `D`/`U`, never a new key code.
3. Start again near the lower part of the left edge. Trace upward, then curve
   inward at the upper-left area and make a small circle. It must select the
   upper-left centre and continue `U` instead.
4. Repeat the positive and negative trace plus local circle on the bottom edge.
   Confirm `B/N`, including no change after leaving the bottom edge band.
5. Trace the right and top edges in both directions. No letter and no `edge dial
   acquired edge=1|2` log may occur because these edges are disabled.
6. Start at an enabled edge and move inward before first moving along that edge.
   Circle mode must not start because a direction has not yet been established.

Pointer, scrolling, tap/drag, multi-finger gestures, and the 3x3 grid remain
disabled.

## Expected log

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
bindings.

The `at=(x,y)` values are calibration evidence: report the physical location
used for the stroke together with these lines if a circle is misplaced.

## 日本語

# エッジダイヤル: 方向連動コーナー円形単体試験

## 意図

円形操作は、辺の直線部分を使い切った後も同じ辺操作を続けるためのものです。別コマンド
ではないため、直線と円形は同じ仮想KSCAN位置、したがって本試験では同じ`D`/`U`を
出力します。

指が取得した辺の帯域内にある間は、通常どおり反転できます。下方向は`D`、上方向は`U`です。
局所円へ向けてその帯域を離れる時点で、辺帯域内における最後の方向を保持します。以後、円の
回転方向は意図的に無視し、円は継続入力の量だけを与えます。そのため左辺を下へなぞった後、
円へ近づくため上へ曲がった場合も、その接触中の円操作でも、`D`だけを出力します。

局所円の中心は、取得した辺と、その辺に沿った最後の有意な移動方向から選びます。一つの
角に固定しません。

|取得辺 |直線方向 |円の中心 |
| --- | --- | --- |
|左 |下 |左下 |
|左 |上 |左上 |
|右 |下 |右下 |
|右 |上 |右上 |
|上 |左 |左上 |
|上 |右 |右上 |
|下 |左 |左下 |
|下 |右 |右下 |

この単体UF2では左辺と下辺だけを有効にします。4つの局所中心は`(360,313)`、`(1640,313)`、
`(360,900)`、`(1640,900)`です。

## 試験用割当

|辺 |正方向 |負方向 |
| --- | --- | --- |
|左 |下: `D` |上: `U` |
|右 |下: `R` |上: `I` |
|上 |右: `T` |左: `Y` |
|下 |右: `B` |左: `N` |

## 手順

1. `travelers-pad-edge-dial-selectable-edges.uf2`を書き込み、必要ならUSBを再接続
   してプレーンテキストエディターを開きます。
2. 左辺の上側から始めて下へなぞり、`D`が繰り返されることを確認します。左下付近で
   約80カウント内側へ曲げ、小さな円を描きます。出力は`D`/`U`のままで、新しいキー
   コードには変わりません。
3. 今度は左辺下側から開始して上へなぞり、左上付近で内側へ曲げて小さな円を描きます。
   この場合は左上中心が選択されることを確認します。
4. 下辺についても正負の辺移動と局所円を試し、`B/N`となること、下辺帯域を離れた後も
   文字が変わらないことを確認します。
5. 右辺・上辺を両方向へなぞります。これらの辺は無効なので、文字出力も
   `edge dial acquired edge=1|2`ログも出ないことを確認します。
6. 有効な辺から始め、最初に辺方向へ動かさずに内側へ移動します。辺方向が未確定なので、
   円形モードへ入らないことを確認します。

ポインタ、スクロール、タップ/ドラッグ、多指ジェスチャー、3x3格子は無効です。

## 期待ログ

```
edge dial acquired edge=0
edge dial circle acquired edge=0 direction=1 at=(x,y) center=(360,900)
edge dial mode=circle edge=0 row=0 column=0 at=(x,y)
```

```
edge dial circle acquired edge=0 direction=-1 at=(x,y) center=(360,313)
edge dial mode=circle edge=0 row=0 column=0|1
```

`mode`は診断用だけで、両モードとも`row=0`、同じキーマップ割当です。

`at=(x,y)`は位置校正のための値です。円の位置がずれる場合は、操作した物理的な位置と
この行を合わせて報告してください。
