# LP2 Auto-Prox-off comparison

## English

Driver/pad branch: `debug/lp2-autoprox-off`. Feature module unchanged.
UF2: `C:\Users\takec\Downloads\travelers-pad-lp2-autoprox-off-20261007.uf2`.

The unrestricted passive diagnostic reproduced no input after idle. Last valid
report was approximately 71s; at approximately 610s there were no new raw frames,
BLE/workqueue remained alive, errors=0, and rescues=1 (startup only). Cached mode
was old Active (age approximately 539s), not proof of current Active or LP2.
USB reconnection had not restored reports within the captured history. The first
input after boot was around 55s, but the user touch time is unknown, so the initial
USB-removal delay cannot be attributed to a particular mode from this capture.

This comparison clears **Other Settings 0x11C0 bit 5 only**, changing 0x80A8 to
0x8088. LP2 entry remains allowed (`power-mode-limit = "none"`); sampling,
timeouts, reference management, ALP enable/electrodes/thresholds/debounce/filters,
LP1 Auto-Prox, BLE settings and feature behavior are unchanged. This is not an
ALP-disable test. Original initialization arrays stay unchanged. Optional driver
`diagnostic-disable-lp2-autoprox` patches the write payload and requires the full
Other Settings word to match before ATI, even if `power-diagnostics` is absent.
Passive observations remain enabled in this firmware; no runtime polling/wake
or periodic reset is added. USB is still power/serial only and HID BLE only.

Boot confirmation:

```text
diagnostic LP2 Auto-Prox disabled: Other Settings verified 0x8088
```

Also save `power setup:` rows: timeout=10/60/5/40, active-idle-ms=1500,
other=0x8088, alp-enabled=1, lp1-autoprox=0, lp2-autoprox=0. This verifies
configuration, not actual mode residency.

1. Flash, save boot setup lines, disconnect USB and check pointer/taps/scroll.
   If initially unresponsive, record approximate delay and attempted touch time.
2. Leave untouched for 5 minutes; before USB/RESET move one finger for 30s,
   then tap and scroll. If it does not react, stop the test and reconnect USB
   without resetting to preserve RAM history.
3. Save the complete history including `sensor` rows; share whether USB restored
   input and any first-response delay. No mandatory 30min check for this isolation.

Success strengthens the LP2 Auto-Prox-path hypothesis but does not exclude
hardware/supply/reference issues or prove LP2 was entered. Failure means Auto-Prox
alone is insufficient as a workaround. Cached observations still become stale
during Event Mode silence; passive observations cannot time the physical touch.
Fallback: accepted `travelers-pad-sensor-idle-only-20261007.uf2`.

Reference: [Azoteq datasheet](https://www.azoteq.com/images/stories/pdf/IQS9150_IQS9151_datasheet.pdf), sections 5.6.3/6 and Appendix A.14.

## 日本語

driver/pad：`debug/lp2-autoprox-off`。機能モジュールは変更しません。
UF2：`C:\Users\takec\Downloads\travelers-pad-lp2-autoprox-off-20261007.uf2`。

前回のLP2許可版で停止再現。最終report約71秒、約610秒でもraw更新なし。
BLE/workqueueは動作、errors=0、rescueは起動時1から増加なし。
最後のActive採取値は約539秒古く、現在modeの証拠ではありません。
USB再接続後のhistory内でもrawは増えておらず、その範囲では復帰未確認。
起動後約55秒に最初の入力がありますが、実際に指を置いた時刻が不明なので、USB切断直後の
無反応がどのmodeかは今回の記録から判定しません。

今回の変更は **Other Settings 0x11C0 bit5だけを0**（0x80A8→0x8088）です。
`power-mode-limit = "none"` でLP2移行は許可したまま。sampling、timeout、reference、
ALP有効/電極/閾値/debounce/filter、LP1 Auto-Prox、BLE、既存機能は維持。
ALP自体を無効化する試験ではありません。元の初期化配列は変更しません。
任意設定 `diagnostic-disable-lp2-autoprox` により送信payloadの対象ビットを差し替え、
ATI前にOther Settings全wordを読み戻し確認。power-diagnostics未指定でも確認必須です。
このUF2では受動診断を継続し、runtimeポーリング/強制復帰/定期RESETは追加しません。
USBは給電/シリアルのみ、HIDはBLEのみ。

起動確認行：

```text
diagnostic LP2 Auto-Prox disabled: Other Settings verified 0x8088
```

`power setup:` 全行も保存。timeout=10/60/5/40、active-idle-ms=1500、other=0x8088、
alp-enabled=1、lp1-autoprox=0、lp2-autoprox=0を確認。設定のreadbackであり実mode証明ではありません。

1. 書き込み・起動ログ保存後、USBを外して基本操作確認。初動が無反応なら指を動かし始めた時刻と
   およその待ち時間も記録。
2. 5分無接触で放置。USB/RESET前に1本指移動30秒、タップ・スクロール確認。
   無反応ならそこでUSBを戻し、RESETせずRAM履歴を保存。
3. `sensor` を含むhistory全文、復帰ラグ、USBだけで復帰したかを共有。
   今回の切り分けに30分試験は必須としません。

成功すればLP2 Auto-Prox経路の仮説が強まりますが、給電/reference要因やLP2未滞在の可能性は
残ります。失敗ならAuto-Prox停止だけでは回避策として不十分です。
無通信中の採取値は古くなるため、現在modeや実接触からの遅延は受動historyだけでは測れません。
戻し先：合格した `travelers-pad-sensor-idle-only-20261007.uf2`。

根拠：[Azoteq公式データシート](https://www.azoteq.com/images/stories/pdf/IQS9150_IQS9151_datasheet.pdf) 5.6.3/6節、付録A.14。
