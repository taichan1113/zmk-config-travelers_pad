# Edge Dial: Linear Isolated Test / エッジダイヤル: 直線単体試験

## Scope / 対象

This UF2 enables only the left edge of the IQS9150 surface.  A one-finger
contact that **starts** inside the leftmost 140 coordinate counts acquires the
edge for that contact.  Every 80 counts of vertical movement then emits one
virtual key.  Downward movement maps to `D`; upward movement maps to `U`.

このUF2ではIQS9150表面の**左辺だけ**を有効にします。左端から140座標カウント
以内で**接触を開始**した1本指だけが、その接触中は辺操作として取得されます。その後、
縦方向に80カウント移動するごとに仮想キーを1回発火します。下方向は`D`、上方向は
`U`です。

Pointer, scrolling, tap/drag, multi-finger gestures, and the 3x3 grid are
disabled.  This is intentional: it proves the new source in isolation and
cannot alter the accepted integration tests.

ポインタ、スクロール、タップ/ドラッグ、多指ジェスチャー、3x3格子はすべて無効です。
これは新しい入力源を単体で検証し、合格済みの結合試験へ影響させないための意図的な設定
です。

## Procedure / 手順

1. Flash `travelers-pad-edge-dial-linear.uf2`, reconnect USB if the XIAO
   bootloader does not re-enumerate automatically, and open a plain text editor.
   `travelers-pad-edge-dial-linear.uf2`を書き込みます。XIAOのブートローダーが自動で
   再列挙しない場合はUSBを再接続し、プレーンテキストエディターを開きます。
2. Touch near the middle of the left edge and hold one finger down. Do not move
   yet: no character must appear.
   左辺の中央付近に1本指で触れて保持します。まだ動かさない場合、文字は出ないことを
   確認します。
3. Without lifting, trace straight down roughly half the pad. Several `D`
   characters should be inserted. Trace back upward: several `U` characters
   should be inserted.
   指を離さずにパッドの半分程度をまっすぐ下へなぞります。`D`が複数入力されます。
   続けて上へ戻すと、`U`が複数入力されます。
4. Repeat from the middle, right, top, and bottom areas. No character should be
   inserted because those edges are disabled for this first stage.
   中央、右辺、上辺、下辺から同様に操作します。この第1段階では無効なので、文字が
   入力されないことを確認します。
5. In the serial log, the expected sequence is `edge dial acquired edge=0`,
   followed by one or more `edge dial edge=0 column=0` (down) or `column=1`
   (up) messages. No warning or error from `travelers_pad_edge_dial` is expected.
   シリアルログには `edge dial acquired edge=0` の後、下方向なら
   `edge dial edge=0 column=0`、上方向なら `column=1` が複数出ます。
   `travelers_pad_edge_dial`由来のwarning/errorは出ないことが期待値です。

## Acceptance / 合格条件

- A stationary left-edge touch emits no event. / 左辺で静止しているだけでは発火しない。
- One uninterrupted vertical trace emits repeated events in its direction.
  / 1回の連続した縦トレース中に、方向に応じたイベントが繰り返し出る。
- A touch beginning away from the enabled edge emits nothing.
  / 有効辺以外で開始した接触は発火しない。
- No pointer movement, click, scroll, grid key, or multi-finger gesture leaks
  into this isolated profile. / この単体プロファイルにポインタ、クリック、スクロール、
  格子キー、多指ジェスチャーが混入しない。

## Deliberately deferred / この段階では未実装

Circular continuation (edge-to-circle), arbitrary-edge enablement in an
integration profile, and router/composite-KSCAN arbitration are the following
stages. They are intentionally not mixed into this first hardware test.

辺から円への継続操作、結合プロファイルでの任意辺有効化、ルーター/composite-KSCANの
排他統合は次段階です。この最初の実機試験には意図的に混在させません。
