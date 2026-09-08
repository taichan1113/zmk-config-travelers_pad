# Two-finger scroll acceleration test / 2本指スクロール加速試験

The acceleration processor is listener-side policy, after IQS9150 tracking and
axis lock but before the normal wheel scaler. A raw wheel delta of 12 or more
is multiplied by two; smaller deltas remain unchanged. This preserves precise
slow scrolling while increasing travel for fast gestures.

加速processorは、IQS9150追跡と軸ロックの後、通常のホイールscalerの前に置く
listener側ポリシーである。生ホイール値が12以上なら2倍にし、小さい値は変えない。
これにより低速スクロールの精密さを保ちつつ、高速操作の移動量を増やす。

## Procedure / 手順

1. Flash `travelers-pad-two-finger-scroll-acceleration.uf2`. Perform a normal
   two-finger vertical scroll until `2F route=scroll committed` and `scroll axis
   locked: vertical` appear.
   
   `travelers-pad-two-finger-scroll-acceleration.uf2`を書き込む。通常の縦2本指
   スクロールを行い、`2F route=scroll committed` と `scroll axis locked: vertical`
   が出ることを確認する。

2. Scroll slowly over a short distance. It must remain controllable and must
   not show `scroll acceleration tier=fast`.
   
   短い距離をゆっくりスクロールする。精密に操作でき、`scroll acceleration tier=fast`
   が出ないことを確認する。

3. Scroll rapidly in the same direction. The page movement must become clearly
   larger than the slow gesture. `scroll acceleration tier=fast raw=...` is
   expected; slowing down again should log `tier=normal`.
   
   同じ方向へ素早くスクロールする。低速操作より明確に大きくページが動き、
   `scroll acceleration tier=fast raw=...` が出ることを確認する。再び遅くすると
   `tier=normal` が出る。

4. Repeat for horizontal scroll and edge-hold scroll continuation. The ordinary
   fast physical gesture may accelerate; the default edge-continuation delta is
   below the fast threshold and must remain steady.
   
   横スクロールとエッジホールド継続でも繰り返す。通常の高速物理操作は加速してよいが、
   既定のエッジ継続値は高速閾値未満なので一定速度のままでなければならない。

5. Check pointer motion, taps, drag, 3–5F gestures, and pinch/rotation. They
   must be unchanged because this processor accepts only `REL_WHEEL/HWHEEL`.
   
   ポインタ移動、タップ、ドラッグ、3〜5本指ジェスチャー、ピンチ／回転を確認する。
   本processorは`REL_WHEEL/HWHEEL`だけを処理するため、これらは変化してはならない。

## Tuning / 調整

Edit `&travelers_pad_scroll_acceleration` in the shield or keyboard overlay.
Increase `fast-threshold` to make acceleration harder to enter; decrease it to
enter earlier. Set `fast-multiplier` to change the fast-speed gain. Remove
`&travelers_pad_scroll_acceleration` from `input-processors` to disable it.

shieldまたはキーボードoverlayの`&travelers_pad_scroll_acceleration`を編集する。
`fast-threshold`を大きくすると加速に入りにくく、小さくすると早く入る。
`fast-multiplier`で高速時の倍率を調整する。`input-processors`から
`&travelers_pad_scroll_acceleration`を外せば無効化できる。
