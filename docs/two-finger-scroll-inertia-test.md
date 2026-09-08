# Two-finger scroll inertia test / 2本指スクロール慣性試験

Inertia observes only router-committed, axis-locked two-finger wheel reports.
Physical release is read from the raw contact stream. When both fingers are
released, it emits a decaying continuation through the normal pointer listener
chain. It does not run during a held stationary touch, tap, geometry gesture,
or three-or-more-finger gesture.

慣性はrouter確定済みかつ軸ロック済みの2本指ホイールレポートだけを観測し、物理的な
解放は生接触ストリームから判定する。両指を離した時、通常ポインタのlistener chainを
通じて減衰する継続値を出力する。保持中の静止タッチ、タップ、ジオメトリ操作、3本指
以上の操作では動作しない。

## Procedure / 手順

1. Flash `travelers-pad-two-finger-scroll-inertia.uf2`. Confirm the startup
   line `scroll inertia active: delay=20ms period=24ms decay=3/4 stop=2`.
   
   `travelers-pad-two-finger-scroll-inertia.uf2`を書き込む。起動時に
   `scroll inertia active: delay=20ms period=24ms decay=3/4 stop=2`を確認する。

2. Make a clear, moderately fast vertical two-finger scroll, then lift both
   fingers promptly. The page must coast briefly in the same direction and
   stop smoothly. Logs must contain `scroll inertia started h=... v=...` then
   `scroll inertia stopped`.
   
   明確で中程度に速い縦2本指スクロールを行い、すぐに両指を離す。ページは同じ方向へ
   短く惰性移動し、滑らかに停止する。ログには`scroll inertia started h=... v=...`の後に
   `scroll inertia stopped`が出る。

3. Repeat with a deliberately slow scroll. It must either coast very little or
   not coast, but must never reverse direction or jump.
   
   意図的に低速のスクロールでも繰り返す。惰性は非常に小さいか出なくてよいが、方向反転
   またはジャンプは絶対に起きてはならない。

4. Repeat for horizontal scroll. Verify that only the locked horizontal axis
   coasts. Then test edge-hold scroll: releasing after its continuous phase may
   coast in the same direction, but lifting must always end it without stale
   reports.
   
   横スクロールでも繰り返し、軸ロックした横方向だけが惰性移動することを確認する。
   続いてエッジホールド・スクロールを試す。連続操作の後に離すと同じ方向への慣性が
   あってよいが、指を離せば残留レポートなく必ず終了しなければならない。

5. Verify that a stationary 2F tap, pinch/rotation, 3F swipe, one-finger
   pointer motion, and tap/drag never start inertia.
   
   静止2本指タップ、ピンチ／回転、3本指スワイプ、1本指ポインタ、タップ／ドラッグで
   慣性が開始しないことを確認する。

## Tuning / 調整

`start-delay-ms`, `repeat-ms`, `decay-numerator`, `decay-denominator`, and
`stop-delta` belong to `&iqs9150_scroll_inertia`. A smaller decay ratio such as
`2/3` stops sooner; a ratio nearer `1/1` coasts longer. Keep the denominator
greater than the numerator.

`start-delay-ms`、`repeat-ms`、`decay-numerator`、`decay-denominator`、`stop-delta`
は`&iqs9150_scroll_inertia`に属する。`2/3`のように減衰比を小さくすると早く止まり、
`1/1`に近づけるほど長く惰性移動する。分母は分子より大きく保つ。
