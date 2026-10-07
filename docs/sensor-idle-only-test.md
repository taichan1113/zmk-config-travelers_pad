# Sensor Idle-only comparison

## English

The active-only comparison passed the user's 5-minute and 15-minute battery/BLE
idle checks. The RAM history additionally shows contact and pointer/wheel output
before USB reconnection, with zero runtime errors/rescues. This supports an
automatic sensor mode transition/wake issue, but does not identify the exact mode
or exclude electrical supply effects.

This narrower comparison zeros only the Idle Mode Timeout word at **0x11B0**.
Active-to-Idle (1500 ms), stationary touch (10 s), Idle-Touch (60 s), and LP1
(40 s) timeouts retain their initialization values. Preventing Idle-to-LP1
blocks the normal automatic path to LP1/LP2 while allowing Active/Idle-Touch/Idle.
Sampling periods, automatic reference management, contact stabilization,
features, BLE latency zero and RAM history are unchanged. No runtime I2C polling
or periodic reset is added. USB remains power/serial only; HID is BLE only.

The optional driver property `diagnostic-disable-lp-entry` is enabled only by
`tests/ble-idle/sensor-idle-only.overlay`. Both diagnostic properties are absent
by default; active-only takes precedence if both are set. The original Azoteq
initialization arrays are unchanged. Before ATI, RDY-gated readback checks the
two modified bytes; failure stops startup instead of silently running a different
comparison. The boot confirmation is:

```text
diagnostic idle-only: Idle-to-LP1 timeout verified zero; Active/Idle transitions retained
```

Firmware: `C:\Users\takec\Downloads\travelers-pad-sensor-idle-only-20261007.uf2`.
Driver/pad branch: `debug/sensor-idle-only`. Features remain at `13d16ea`.
Local module overrides are used; main and remote manifest revisions are unchanged.

1. Flash and save the boot confirmation. With USB removed, check pointer/taps/scroll.
2. Leave untouched for 5 minutes. Before reconnecting USB or resetting, move one
   finger for 30 seconds, then test taps and two-finger scroll.
3. Repeat after 15 minutes untouched. If practical, repeat the cycle once more.
4. Reconnect USB and capture the complete RAM history, noting whether any input
   stopped and which operations were attempted immediately before reconnection.

If this passes, LP-entry/wake is the narrower suspect and Idle may be usable as
a workaround. If it fails while active-only passes, investigate Active/Idle
transitions and reference handling too. Neither outcome alone proves the exact
silicon/electrical cause. Current consumption has not been measured; this is
still diagnostic firmware, not a validated production power configuration.
Fallback: `travelers-pad-sensor-active-only-20261007.uf2`.

Reference: [Azoteq datasheet, sections 6.2/6.3 and mode register map](https://www.azoteq.com/images/stories/pdf/IQS9150_IQS9151_datasheet.pdf).

## 日本語

Active維持比較版は、実機のバッテリー/BLEで5分・15分放置後の操作に合格しました。
historyでもUSB再接続前の接触・ポインタ/スクロール出力と、runtime errors/rescues=0を
確認しています。センサーの自動mode遷移/復帰が有力ですが、特定modeや給電要因までは未確定です。

今回は **0x11B0のIdle Mode Timeoutだけを0** にします。Active→Idle（1500ms）、
静止タッチ（10秒）、Idle-Touch（60秒）、LP1（40秒）のtimeoutは初期値を維持します。
Idle→LP1を止めて通常の自動LP1/LP2経路を遮断し、Active/Idle-Touch/Idleへの遷移は残します。
sampling、自動reference管理、接触安定化、機能モジュール、BLE latency=0、RAM historyは
変更しません。runtime I2Cポーリングや定期リセットは追加しません。
USBは給電/シリアルのみ、HIDはBLEのみのままです。

driverの任意設定 `diagnostic-disable-lp-entry` を比較専用overlayで指定します。
通常は両診断設定とも未指定です。両方指定時はActive維持が優先します。
Azoteq初期化配列は変更せず、送信時に対象2バイトだけを差し替え、ATI前にRDY通信窓で
読み戻し確認します。不一致なら起動を止めます。起動ログの確認行は以下です。

```text
diagnostic idle-only: Idle-to-LP1 timeout verified zero; Active/Idle transitions retained
```

UF2：`C:\Users\takec\Downloads\travelers-pad-sensor-idle-only-20261007.uf2`。
driver/pad：`debug/sensor-idle-only`。features：`13d16ea` のまま。
ローカルmodule overrideでビルドし、mainやremote manifest revisionは変更しません。

1. UF2を書き込み、上記起動確認行を保存。USBを外してポインタ・タップ・スクロールを確認。
2. 5分間触らず放置。USB再接続/RESET前に1本指移動を30秒、タップ・2本指スクロールを確認。
3. 15分間放置でも同じ操作を確認。可能ならもう1サイクル繰り返す。
4. USBを再接続してhistory全文を保存。停止の有無と、再接続直前に試した操作も共有。

成功すればLP移行/復帰に対象を狭め、Idleを残した回避策を検討できます。
Active維持のみ成功し今回失敗する場合、Active/Idle遷移やreference管理も調査対象です。
消費電流は未測定なので、本番向け省電力設定の合格とはしません。
戻す場合は `travelers-pad-sensor-active-only-20261007.uf2` を使用してください。

根拠：[Azoteq公式データシート6.2/6.3節とmode register map](https://www.azoteq.com/images/stories/pdf/IQS9150_IQS9151_datasheet.pdf)。
