# XIAO USB Reconnect Reliability Test / XIAO USB再接続信頼性試験

## Intent / 意図

When USB power is restored, the XIAO application can begin before the IQS9150
has completed its own power-up/reset sequence. The driver now remains loaded
and retries the product-number read for up to one second instead of treating a
transient `0xeeee` as a permanent missing device.

USB電源の復帰時には、IQS9150自身の電源投入/リセット完了より先にXIAOアプリが開始する
場合がある。ドライバは一時的な`0xeeee`を恒久的なデバイス不在と扱わず、最大1秒間、
製品番号を再試行したまま有効状態を維持する。

The diagnostic UF2 also enables the XIAO boot probe. The red LED flashes only
while the application is running but USB is not configured; it turns off once
USB is configured.

診断UF2ではXIAO起動プローブも有効にする。赤LEDはアプリ実行中かつUSB未設定の間だけ
点滅し、USB設定完了時に消灯する。

## Procedure / 手順

1. Flash `travelers-pad-usb-reconnect-reliability.uf2`. Do not unplug it after
   copying; wait up to 10 seconds for the red LED to turn off and for the serial
   port to appear.
   `travelers-pad-usb-reconnect-reliability.uf2`を書き込む。コピー後はUSBを抜かず、
   最大10秒、赤LED消灯とシリアルポート出現を待つ。
2. Confirm this boot evidence, then move one finger to confirm the pad works.
   次の起動ログを確認し、続けて1本指でポインタが動くことを確認する。

   ```text
   application reached XIAO USB boot probe
   IQS9150 product 0x076a ready after N startup attempt(s)
   STEP1B complete: Event Mode active
   USB configured; boot probe complete
   ```

3. Repeat USB unplug/replug at least ten times. For each cycle, wait up to 10
   seconds, verify the serial port and pointer, and record any cycle that does
   not reach Event Mode.
   USBの抜き差しを最低10回繰り返す。各回で最大10秒待ち、シリアルポートとポインタを
   確認する。Event Modeへ到達しない回があれば記録する。

## Interpretation / 判定

- A few `IQS9150 startup wait` lines followed by a valid product number are an
  expected successful recovery.
- `IQS9150 product did not become ready` means the sensor startup retry expired.
- A continuously flashing red LED means the application is alive but USB was
  not configured; no LED activity means the application did not reach the boot
  probe.

- 数行の`IQS9150 startup wait`の後に有効な製品番号が出るのは、期待どおりの回復である。
- `IQS9150 product did not become ready`はセンサー起動再試行の時間切れを示す。
- 赤LEDが点滅し続ける場合はアプリは動作中だがUSB未設定、LEDが一切動かない場合は
  アプリが起動プローブまで到達していない。
