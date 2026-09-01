# Selectable edge isolated test / 任意辺単体試験

This UF2 enables only the left and top edge zones (140 sensor counts). Pointer,
scroll, tap/drag, grid, and multi-finger gestures are disabled.

このUF2では左辺と上辺だけ（幅140カウント）を有効にする。ポインタ、スクロール、
タップ/ドラッグ、格子、多指ジェスチャーは無効である。

1. Open Notepad and flash `travelers-pad-selectable-edge-isolated.uf2`.
   メモ帳を開き、`travelers-pad-selectable-edge-isolated.uf2`を書き込む。
2. Tap near the middle of the left edge: exactly `L` must appear.
   左辺中央をタップする: `L`だけが入力される。
3. Tap near the middle of the top edge: exactly `T` must appear.
   上辺中央をタップする: `T`だけが入力される。
4. Tap the right edge, bottom edge, and centre: no character and no mouse
   output must occur. 右辺、下辺、中央をタップする: 文字もマウス出力も無い。

The log for enabled edges contains `grid tap row=0 column=0` (left) or
`grid tap row=0 column=2` (top), followed by one press/release pair.
有効辺のログは左が `grid tap row=0 column=0`、上が `grid tap row=0 column=2` で、
それぞれ一組の押下/解放となる。
