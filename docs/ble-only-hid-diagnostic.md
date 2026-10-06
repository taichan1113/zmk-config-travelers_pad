# BLE-only HID diagnostic

## English

This temporary profile disables USB HID, not USB CDC. USB supplies power and
serial logs; pointer, button, wheel and keyboard reports use BLE only. It keeps
the existing device name, bonds, trackpad tuning and battery-measurement path.
Battery sampling is shortened to five seconds for diagnosis. This is not a
battery or USB-reconnection fix.

Build with the normal `iqs9150_xiao` shield and `zmk-usb-logging` snippet, plus
`-DEXTRA_CONF_FILE=<repository>/diagnostics/ble_only_hid.conf`.

1. Flash the diagnostic UF2 normally. Do not flash settings-reset firmware or
   clear existing bonds. Keep another pointing device available.
2. Keep USB connected and open the CDC COM port at 115200 baud. Confirm the PC
   connects to `IQS9150 EV02` over BLE. Wait for ATI completion.
3. Move the pointer, tap and scroll for at least 30 seconds. All HID must use BLE
   even with the USB data cable connected. Record battery voltage/percentage,
   BLE connection changes, endpoint selection and any errors.
4. With the battery switch on, remove USB and repeat. USB logs are unavailable
   while disconnected; note whether BLE disconnects or input alone stops.
5. Reconnect USB without resetting first and capture logs. If input stays dead,
   press RESET once while logging and capture the full startup sequence.

A battery percentage alone does not establish actual battery voltage. A working
CDC port does not guarantee BLE reports are received. Roll back by flashing the
previous normal UF2; the diagnostic profile does not intentionally erase settings.

### Stack-probe variant

The first BLE-only trace reports 3766–3772 mV and 43%, then ends mid-line during
pointer processing at 12.258 seconds. The user confirmed that the COM connection
remains but periodic battery logs stop. This does not prove a stack overflow.
The initial firmware uses the default 512-byte input-thread stack with no
hardware stack protection.

The second variant additionally applies `ble_only_hid_stack_probe.conf`:
input stack 4096 bytes, hardware stack protection/assertions, and five-second
thread-analyzer reports. BLE send-stack size and queue depth remain unchanged.
Build using both files in `EXTRA_CONF_FILE`, separated by a semicolon.
Capture at least 60 seconds: pointer, taps and scroll, plus `STACK: ... usage`
lines for input, HID Over GATT Send Work, and other threads. Report any fatal
error and whether periodic battery/thread logs continue after input stops.
This variant provides evidence for sizing; it is not a final production profile.

### Follow-up: normal USB/BLE profile

The guarded 4096-byte diagnostic recorded an input-thread high-water mark of
768 bytes, above the original 512-byte allocation. No errors were observed in
the submitted trace. The user confirmed stable operation with the battery
switch off/on, LiPo-only power, and USB reconnection without RESET. There was a
brief pause after USB removal, followed by resumed BLE input.

These results strongly support insufficient input-stack capacity as a cause,
but do not constitute a captured overflow fault in the original binary.
The normal shield now reserves 2048 bytes and enables hardware stack protection.
It retains USB and BLE HID, without the diagnostic analyzer, diagnostic
assertions or five-second battery interval. Validate both transports again.
Do not apply either diagnostic EXTRA_CONF_FILE when building this normal image.

Battery readings changed between roughly 3.75 V / 40% and 4.16 V / 95% in the
same diagnostic trace. Battery-measurement accuracy remains a separate issue;
the input-stack change does not fix or certify those readings.

## 日本語

一時的にUSB HIDだけを無効化し、USB CDCは維持します。USBは給電とシリアルログに使い、
ポインタ・ボタン・スクロール・キー入力はBLEだけで送ります。デバイス名・bond・pad調整値・
残量測定経路は維持し、調査用に残量測定間隔だけ5秒へ短縮します。
これは残量表示やUSB再接続の不具合修正ではなく、原因切り分け用です。

通常の `iqs9150_xiao` shieldと `zmk-usb-logging` snippetに加えて、
`-DEXTRA_CONF_FILE=<repository>/diagnostics/ble_only_hid.conf` を指定してビルドします。

1. 診断UF2を通常の方法で書き込みます。settings-reset版の書込み・bond消去は不要です。
   別のポインティングデバイスを使える状態にしてください。
2. USBを挿したままCDCのCOMポートを115200 baudで開き、PCがBLEで `IQS9150 EV02` に
   接続することを確認します。ATI完了を待ちます。
3. 30秒以上ポインタ・タップ・スクロールを試します。USBデータ接続中もHIDはBLEのみです。
   電池電圧・残量、BLE接続変化、出力端点、エラーをログに残してください。
4. バッテリーのスイッチをONにしたままUSBを抜いて同じ操作をします。
   USBを外している間はCOMログを取れないため、BLE接続が切れるのか、入力だけ止まるのかを
   メモしてください。
5. RESETせずにUSBを再接続してログを取ります。操作不能のままなら、ログ取得中にRESETを
   一度押し、起動直後から記録してください。

残量表示だけでは実際の電池電圧は確定しません。CDCが動いてもBLE入力成功とは限りません。
元に戻すには以前の通常UF2を書き込みます。この診断設定は意図的な設定消去を行いません。

### スタック調査版

最初のBLE専用ログでは3766〜3772mV・43%が記録され、12.258秒のポインタ処理中に
行の途中で終わっています。COM接続は残る一方、定期的な残量ログも止まるとの報告です。
これだけでstack overflowとは確定しません。初版は入力threadの既定512バイトを使用し、
ハードウェアstack保護は無効でした。

次の版は `ble_only_hid_stack_probe.conf` を追加し、入力stackを4096バイトへ拡張、
stack保護・assertを有効化し、5秒ごとのthread使用量を記録します。
BLE送信threadのstack容量とqueue深さは変えません。ビルド時は2つのconfをセミコロンで
区切って `EXTRA_CONF_FILE` へ指定します。
60秒以上ポインタ・タップ・スクロールを試し、input・HID Over GATT Send Workなどの
`STACK: ... usage` 行を含めてログを提出してください。fatal errorの有無と、操作停止後も
残量・threadログが続くかを確認します。容量決定のための診断版であり、正式版の設定ではありません。

### 通常USB/BLE版への反映

4096バイト・保護付き診断版で、入力threadの最大使用量768バイトを観測しました。
元の割当512バイトを超えています。提出ログにエラーはなく、バッテリースイッチOFF/ON、
LiPo単独、RESETなしのUSB再接続でも安定動作との実機報告がありました。
USBを抜いた直後は短い停止がありましたが、その後BLE入力は復帰しました。

入力stack不足が原因という見立てを強く支持しますが、元のバイナリでoverflowのfaultを
取得したわけではありません。通常shieldでは入力stackを2048バイトにし、
ハードウェアstack保護を有効化しました。USB/BLEのHIDを維持し、診断analyzer・
診断assert・5秒の残量間隔は含めません。両方の送信経路を改めて実機検証します。
通常版のビルドには診断用EXTRA_CONF_FILEを指定しないでください。

同じ診断ログ内で残量が約3.75V/40%と4.16V/95%の間で変動しています。
残量測定の正確性は別課題です。入力stackの変更でこの測定を修正・保証したとはしません。
