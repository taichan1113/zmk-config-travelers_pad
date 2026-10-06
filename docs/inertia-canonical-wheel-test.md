## English

# Canonical wheel isolation

## Diagnosis

Hardware log session 41: last physical wheel -84 was scaled to -7; inertia
stored -7 and estimated -374 counts/s. Its first generated -4 was scaled again
to -1. Zephyr sends the same mutable event to listeners, and ZMK's scaler
modifies event->value before the inertia callback in this build.

## Correction

The broker registers an init-only source/sink observation route. After successful,
authorized live wheel output, it copies the original value into a separate input
queue event for `iqs9150_wheel_observation`. This device has no host listener or
processors. Inertia subscribes to this source and still emits through the normal
pointer device/listener. Routed scroll and delegated edge-hold are copied;
post-session output is not copied, preventing feedback. Callback ordering cannot
make a host scaler change the separate observation event.

External overlays must migrate `wheel-input-device` to a private
`zmk,travelers-pad-canonical-wheel-input` device. Source and sink may not coincide.
With inertia disabled, no route is registered and no extra observation is queued.
Copy failure is logged and does not fail/replay the successful host wheel output.

No tuning or decay changes: edge20, scaler1/12, half-life250ms, smoothing80ms,
start delay/period10ms, input limit128 and inertia clamp300 are unchanged.

## Hardware check

UF2: `C:\Users\takec\Downloads\travelers-pad-inertia-single-scale-20261006.uf2`

1. Repeat medium-speed vertical/horizontal scroll and lift while moving. Compare
   the handoff jerk. Avoid the fastest flicks initially: >128 frame jumps remain
   deliberately discarded and must not be confused with double scaling.
2. Capture the last physical `scale_val` input and inertia's pending `h/v`.
   The pending value must be a raw input (e.g. -84), not its scaled -7. It may
   be an earlier sample if the final sample was rejected. Seed speed is smoothed;
   first inertia delta need not equal the last contact report (different periods).
3. Touch during inertia: stop without false click; stop with fingers down for
   0.5s then lift: no glide. Briefly check scroll edge-hold and release.

Hardware approval pending

## 日本語

# 倍率適用前wheelの隔離

## 原因

実機ログsession 41で、生wheel -84が-7へ縮小され、慣性も-7を保存して初速-374count/sを
推定した。最初の慣性-4はさらに-1へ縮小。Zephyrは同じ可変eventをlistenerへ渡し、
本buildではscalerが慣性callbackより先に値を書き換えていた。

## 修正

brokerで初期化時に観測source/sinkを登録。認可されたlive wheel出力のqueue投入成功後、
元の値を別input queue eventとして`iqs9150_wheel_observation`へ複製する。
このdeviceにはHID listener・processorを接続しない。慣性はこの専用sourceを購読し、
出力は従来のpointer/listenerへ戻す。通常scrollと委譲された端保持を複製し、
post-session出力は複製しないため自己フィードバックしない。callback順が変わっても
HID scalerが専用観測eventの値を書き換えることはない。

外部overlayは`wheel-input-device`を専用compatibleのdeviceへ移行する。source/sinkの
同一指定は禁止。慣性無効時は登録・追加観測なし。複製失敗はログ出力するが、成功済み
HID出力の失敗扱いや再送はしない。

設定・減衰方式の変更なし：辺20・倍率1/12・半減250ms・平滑80ms・遅延周期10ms・
入力異常除外128・慣性上限300を維持。

## 実機確認

1. 縦横を中程度の速さで動かしながら離し、切替の段差を比較。最初は極端な高速操作を
   避け、128超えの除外warningを二重倍率と混同しない。
2. 最後の通常`scale_val`入力と慣性pendingの`h/v`を採取。pendingは例として-84の
   生値であり、縮小後-7ではないこと。最終sample除外時は前のsampleになり得る。
   初速は平滑化済みで、周期も違うため初回慣性量と最終接触report量の一致は不要。
3. 慣性中再接触で停止・誤クリックなし。接触したまま0.5秒停止後の解放で慣性なし。
   scroll端保持と解放も簡単に確認する。

実機承認待ち。
