# Time-based scroll inertia / 時間基準スクロール慣性

Branch / ブランチ: `feat/time-based-scroll-inertia`
Baseline / 基準: `d003657` (edge20 tuning / 辺幅20調整)

## Changed / 変更点

Estimate counts/ms from wheel-report timing and recent contact cadence, smooth
velocity, decay by actual elapsed time, and accumulate fractional travel. New
contact/cancel and broker lease authorization are unchanged. This is trackball-like
scroll momentum, not pointer inertia or true mechanical simulation. Queue timing
is used, not a sensor hardware timestamp; the initial report uses the latest contact
interval (or repeat period if unavailable).

wheel report時刻と直近接触周期からcount/ms速度を推定し、速度を平滑化、実経過時間で減衰、
移動端数を蓄積する。再接触・取消し・broker lease認可は変更しない。トラックボールに近い
スクロールの運動量であり、ポインタ慣性や厳密な機械シミュレーションではない。
時刻はqueue処理時刻でありsensor hardware timestampではない。初回は直近接触周期、
未取得なら繰返し周期を推定に使う。

| Setting / 設定 | Current / 現在値 | Meaning / 意味 |
|---|---|---|
| `half-life-ms` | 250 | Speed halves in this time; larger = longer glide / 速度の半減時間、大きいほど長く継続 |
| `velocity-window-ms` | 80 | Velocity smoothing interval / 速度平滑化時間 |
| `release-timeout-ms` | 120 | Do not start from older motion / これより古い移動で開始しない |
| `stop-speed` | 20 | Raw counts/s stopping threshold / 生count毎秒の停止閾値 |
| `max-duration-ms` | 3000 | Maximum glide lifetime / 最大継続時間 |
| `start-delay-ms` / `repeat-ms` | 10 / 10 | Start delay and report period / 開始遅延と出力周期 |
| `max-delta` | 300 | Clamp input/report magnitude, not gain / 入力・出力上限であり倍率ではない |
| Wheel scaler / 倍率 | 1/12 | Shared with normal scroll and edge hold / 通常scroll・端保持と共通 |
| Edge dial band / 辺幅 | 20 | Unchanged / 維持 |

With `half-life-ms > 0`, legacy `decay-numerator`, `decay-denominator`, and
`stop-delta` do not control the new glide. Set `half-life-ms = <0>` to select the
legacy algorithm with the retained 9/10 and stop-delta 2. Pinch/rotation remain off.

`half-life-ms > 0`では旧`decay-numerator`・`decay-denominator`・`stop-delta`を新慣性の
制御に使わない。`half-life-ms = <0>`で従来方式（保持した9/10・停止閾値2）へ戻せる。
ピンチ／回転は引き続き無効。

## Verification / 検証

Host math regression / host計算回帰試験（workspace root）:

```sh
nix develop --command bash -c 'cc -std=c11 -Wall -Wextra -Werror -fsanitize=undefined -I module/zmk-features-trackpad/include config/zmk-config-travelers_pad/tests/inertia-math.c -o /tmp/travelers-pad-inertia-math-test && /tmp/travelers-pad-inertia-math-test'
```

UF2: `C:\Users\takec\Downloads\travelers-pad-time-based-inertia-20261006.uf2`

1. Scroll vertically/horizontally slowly, then quickly, lifting while moving.
   Faster motion should give a stronger glide that decays smoothly, without
   reversal or pointer jumps. Confirm both directions on both axes.
   縦横を遅く／速くscrollし、動かしながら離す。速い操作ほど強い滑走となり、
   滑らかに減衰し、逆行・ポインタ飛びがないこと。両軸の正負方向を確認。
2. Move, stop with fingers still down for about 0.5s, then lift: no inertia.
   動かした後、接触したまま約0.5秒停止して離す：慣性を開始しないこと。
3. During inertia, touch again: immediate stop, no spurious click or stuck button.
   慣性中に再接触：停止し、誤クリック・押しっぱなしがないこと。
4. Brief regressions: 2F tap, 1F tap/drag, edge actions, scroll edge-hold and release.
   簡単な回帰：2Fタップ・1Fタップ／ドラッグ・辺操作・scroll端保持と解放。

Capture boot `inertia mode=time-based`, `inertia seed speed ... counts/s`, lease
and stop logs. If glide hits the 3s cap, it stops safely without an additional
report. Hardware approval pending.

起動時`inertia mode=time-based`、`inertia seed speed ... counts/s`、lease・停止ログを採取。
3秒上限では追加出力せず安全に停止する。実機承認待ち。
