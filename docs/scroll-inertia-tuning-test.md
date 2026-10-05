# Scroll/inertia tuning / スクロール・慣性調整（2026-10-06）

Branch / ブランチ: `feat/scroll-inertia-tuning`
Pinch/rotation identification remains deferred; both recognitions are disabled.
ピンチ／回転の識別方針は保留とし、調整中は両方の認識を無効化する。

| Parameter / 設定 | Value / 値 |
|---|---|
| Edge dial `edge-size` | 20 (user correction / 指定訂正) |
| Pointer listener scaler | `zip_scroll_scaler 1 12` |
| Inertia `start-delay-ms` | 10 |
| Inertia `repeat-ms` | 10 |
| Inertia decay / 減衰率 | 9/10 |
| Inertia `max-delta` | 300 |
| Inertia `stop-delta` | 2 (unchanged / 維持) |

Only configuration changes; no algorithm change. Normal 2F scroll, scroll edge-hold
and inertia share the scaler. Edge behavior/macro scrolling is unaffected.
`max-delta` is a clamp, not a gain; the pointer still rejects per-frame coordinate
delta above `scroll-max-frame-delta=128`, so raising the clamp does not bypass that filter.

設定だけの変更でアルゴリズム変更なし。通常2Fスクロール・スクロール端保持・慣性は倍率を共有。
辺Behavior／マクロのスクロール速度は変わらない。`max-delta`は倍率でなく上限。
pointerの`scroll-max-frame-delta=128`による異常座標除外は維持し、上限300でも解除されない。

UF2: `C:\Users\takec\Downloads\travelers-pad-scroll-inertia-tuning-edge20-20261006.uf2`

The edge-hold continuation band remains 80. Only edge-dial acquisition uses 20.
エッジ保持継続の帯幅は80のまま。辺操作の取得だけ20とする。

Inertia implementation review: the current initial value is the last nonzero
wheel report, not a time-normalized velocity. Integer truncation during decay
shortens low-speed tails (8 at 9/10 stops on the seventh 10ms tick). A future
separate change should estimate recent physical velocity and preserve fractional
velocity/output residuals, while retaining arbiter leases and the existing scaler.
Do not treat this width-only firmware as a new inertia algorithm.

慣性実装調査：初速は最後の非ゼロwheel reportであり時間正規化速度ではない。
整数切捨て減衰により低速の余韻が短い（初速8・9/10は10msの7回目で停止）。
次段階の独立改修候補は直近の物理速度推定と速度／出力端数の保持。
arbiter leaseと既存scalerは維持する。この幅修正版に新慣性アルゴリズムは含まない。

## Hardware checks / 実機確認

1. Slow and fast vertical/horizontal 2F scroll; verify the new speed and stable direction.
   2F縦横スクロールをゆっくり／速く動かし、新しい速度と方向の安定性を確認する。
2. Scroll then release; verify natural continuation/stop. Touch again during inertia;
   continuation must stop without a spurious click or pointer jump.
   スクロール後に離し、慣性の継続・停止を確認。慣性中に再接触すると停止し、
   誤クリックやポインタ飛びがないこと。
3. Briefly check edge scrolling, 2F scroll edge-hold, 1F pointer/tap/drag.
   辺スクロール、2Fスクロール端保持、1Fポインタ・タップ・ドラッグを簡単に確認する。

Hardware approval pending / 実機承認待ち。
