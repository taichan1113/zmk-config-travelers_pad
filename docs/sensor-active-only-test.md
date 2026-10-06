[English](#english) | [日本語](#日本語)

# Sensor automatic-power-transition comparison

## English

### Baseline and evidence

Contact stabilization was accepted on hardware. The separate idle failure was
captured with the older contact-transition diagnostic: after the last release
at 584.498 seconds, raw frames stayed at 2177 and IRQs at 2235 until USB returned
around 955.847 seconds. The user attempted one-finger movement during the final
30 seconds before reattachment. BLE stayed connected at latency zero and the
system-workqueue heartbeat progressed. Reattaching USB restored movement without
RESET. This implicates the sensor/RDY path rather than recognition suppression,
but does not establish whether power-mode wakeup, supply/electrical conditions
or interrupt handling is responsible.

### Single-variable diagnostic

The [IQS9150/IQS9151 datasheet §6.2 and memory map](https://www.azoteq.com/images/stories/pdf/IQS9150_IQS9151_datasheet.pdf)
specifies that timeout zero means never. The diagnostic zeros these five words:

| Address | Timeout | Baseline | Diagnostic |
| --- | --- | --- | --- |
| 0x11AC | Stationary touch | 10 seconds | 0 |
| 0x11AE | Idle-touch | 60 seconds | 0 |
| 0x11B0 | Idle | 5 seconds | 0 |
| 0x11B2 | LP1 | 40 seconds | 0 |
| 0x11B4 | Active to idle | 1500 ms | 0 |

Automatic reference management remains enabled; Manual Control is not selected.
Sampling periods and Event Mode remain unchanged. No runtime I2C polling, forced
communication or periodic reset is introduced. Thus normal Event Mode silence
still produces no input reports. Zeroing these timers is intended to prevent
automatic departure from the reset/initialized Active state; it is not continuous
observation of the chip's actual mode.

The driver property `diagnostic-disable-mode-timeouts` is absent by default.
Only `tests/ble-idle/sensor-active-only.overlay` enables it. The initializer
patches a temporary payload, not the original Azoteq data. After configuration,
an RDY-gated readback verifies all ten bytes as zero before ATI proceeds. A read
failure or mismatch fails initialization explicitly; check the boot line:

```text
diagnostic active-only: five mode timeouts verified zero; automatic reference management retained
```

The accepted driver contact normalization and feature stabilization remain
included, as do BLE-only HID, actual preferred latency zero, and RAM history.
Battery consumption can increase. This is not a permanent production fix.

### Hardware procedure

1. Flash `travelers-pad-sensor-active-only-20261007.uf2`. Save the boot verification
   line and confirm pointer/tap/scroll with USB present. Battery switch ON.
2. Remove USB, confirm BLE operation, leave untouched for five minutes, then try
   one-finger movement for 30 seconds, plus tap and two-finger scroll. Do not
   reconnect USB or RESET before checking.
3. If working, repeat with 15 minutes idle. If stopped at either duration, keep
   attempting movement for 30 seconds, then reconnect USB without RESET and save
   the complete history dump. Record elapsed idle time and whether USB restored it.
   Capture within eight minutes after the failed attempts so they remain in RAM.

If it passes, repeat once to reduce the chance of an intermittent false pass.
A pass strengthens the automatic-mode-transition/wakeup hypothesis but does not
identify the particular mode/register or exclude electrical effects caused by
different sensor operating load. A failure with frozen IRQ/raw counters lowers
the priority of this hypothesis and keeps sensor supply/RDY handling in scope.
Restore `travelers-pad-contact-stability-fix-20261007.uf2` to return to the accepted
automatic-power profile; do not clear bonds or change PC Bluetooth settings.

The comparison uses local driver branch `debug/sensor-active-only`, features
`fix/transient-contact-count`, and pad `debug/sensor-active-only`. No main or
remote manifest revision is changed. Host tests cover only-timeout patching,
split-byte boundaries, default unchanged, RDY-gated verification and mismatch
failure; contact, startup/recovery, inertia and architecture regressions pass.

## 日本語

### 比較元と根拠

接触安定化は実機合格です。別件の放置停止は旧接触変化診断版で採取され、
最後の解除584.498秒以降、USBが戻る955.847秒頃までraw=2177、IRQ=2235のままでした。
利用者は再接続前の最後30秒に1本指移動を試しています。BLEはlatency=0で接続を維持し、
system workqueueも進行しています。USBを戻すだけでRESETせず復帰しました。
認識抑制より手前のセンサー/RDY経路が対象ですが、省電力復帰・給電/電気的条件・割込みの
どれが原因かはまだ確定していません。

### 変更対象を絞った診断版

[IQS9150/IQS9151公式datasheet §6.2・メモリマップ](https://www.azoteq.com/images/stories/pdf/IQS9150_IQS9151_datasheet.pdf)
ではtimeout=0はタイムアウトなしです。次の5ワードだけ0にします。

| アドレス | タイムアウト | 比較元 | 診断版 |
| --- | --- | --- | --- |
| 0x11AC | 静止タッチ | 10秒 | 0 |
| 0x11AE | Idle-touch | 60秒 | 0 |
| 0x11B0 | Idle | 5秒 | 0 |
| 0x11B2 | LP1 | 40秒 | 0 |
| 0x11B4 | ActiveからIdle | 1500ms | 0 |

Manual Controlにはせず、自動reference管理を維持します。sampling周期とEvent Modeも変更しません。
実行中のI2Cポーリング・通信強制・定期RESETは追加しません。Event Modeで未操作中の
レポートが出ないこと自体は引き続き正常です。初期化後のActive状態から自動的に抜けないように
する設定ですが、センサー実モードを常時観測しているわけではありません。

driverの `diagnostic-disable-mode-timeouts` は通常未指定です。
`tests/ble-idle/sensor-active-only.overlay` だけで有効化し、元のAzoteq初期化データは変更せず、
一時送信バッファのtimeout部分だけ書き換えます。設定完了後、RDY窓で10バイトを読み戻し、
全て0であることを確認してからATIへ進みます。読み戻し失敗・不一致なら初期化を明示的に失敗させます。
起動時には次の行を確認してください。

```text
diagnostic active-only: five mode timeouts verified zero; automatic reference management retained
```

合格したdriver接触正規化・features接触安定化、BLE限定HID・latency=0要求・RAM履歴は維持します。
電池消費が増える可能性があり、恒久対策ではなく比較診断版です。

### 実機手順

1. `travelers-pad-sensor-active-only-20261007.uf2` を書き込み、上記の起動確認行を保存します。
   USB接続中の移動・tap・scrollを確認し、バッテリースイッチをONにします。
2. USBを外してBLE操作を確認後、触らず5分放置します。USBを戻す/RESETする前に、
   1本指を30秒動かし、tap・2本指scrollも確認します。
3. 動けば15分放置でも繰り返します。どちらかで止まれば30秒操作を試してから、
   RESETせずUSBを戻し履歴全体を保存します。放置時間・USBで復帰したかも報告してください。
   失敗時の操作を履歴に残すため、操作確認後8分以内に採取します。

通ればもう一度繰り返し、偶然の成功を避けます。成功は自動省電力遷移/復帰の仮説を強めますが、
特定のモード/レジスターまでは確定せず、動作負荷変更による電気的影響も完全には除外できません。
IRQ/rawが停止したまま再発するなら、この仮説の優先度を下げ、センサー給電・RDY処理の調査を継続します。
合格済みの通常自動省電力設定へ戻す場合は `travelers-pad-contact-stability-fix-20261007.uf2` を使います。
BT_CLR・PCのBluetooth設定変更はしません。

比較試験はローカルdriver `debug/sensor-active-only`、features `fix/transient-contact-count`、
pad `debug/sensor-active-only` で管理し、mainやremote manifest revisionは変更していません。
host試験はtimeout範囲だけの書換え・分割境界・通常設定維持・RDY窓での読み戻し・不一致時失敗を検証し、
接触・起動/復旧・慣性・構造の回帰試験も通過しています。
