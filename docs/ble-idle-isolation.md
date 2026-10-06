# BLE idle isolation / BLE放置停止の切り分け

## English

The report describes a battery/BLE pointer stopping after three minutes idle,
then recovering when USB is connected. The captured log starts after USB
attachment: driver state is RUNNING, reports progress, errors are zero, and no
reset/recovery appears. It does not establish the state before attachment or
whether USB power or a transport change caused recovery. Deep sleep and device
PM are disabled in this build. The CDC poll-output implementation drops data
when disconnected; it is not a blocking full-buffer UART path.

The diagnostic `tests/ble-idle/ble-only.conf` disables USB HID only. The USB
logging snippet retains CDC serial and USB power; BLE remains the only HID
transport. Sensor, recognizers, inertia, pin assignments and tuning are unchanged.
The original dual-transport UF2 remains available for returning to normal use.
This is a diagnostic profile, not a confirmed fix.

Hardware checks:

1. Keep USB connected for power and serial logging. Confirm BLE-connected pointer
   operation, idle three minutes, and retest pointer, tap and scroll without RESET.
   Save the whole log through the idle and touch attempts.
2. With the same firmware, disconnect USB and use battery/BLE only. Idle three
   minutes and retest. Check the PC's connection status if it stops. Only then
   reconnect USB without RESET and record whether motion resumes.
3. If both pass, repeat for thirty minutes. Avoid changing PC Bluetooth power
   policies during these tests so the comparison remains meaningful.

If step 1 fails, the attached log can show whether contact reports, recognizer
output, connection state or BLE notifications stop. If step 1 passes but step 2
fails, battery-only operation or the absence of USB remains implicated; this
alone does not prove a supply-voltage defect. If USB reconnect recovers motion
with this profile, switching HID from BLE to USB cannot be the explanation.

The serial COM port may change after the USB descriptor changes. Do not clear
bonds or pair again unless the existing pairing genuinely stops connecting.

## 日本語

バッテリー/BLEで3分放置後にポインタが停止し、USB接続で復帰した報告です。
取得ログはUSB接続後からで、driverはRUNNING、レポート数は増加、エラーは0です。
reset/recoveryも出ていません。接続前の状態と、USB給電・送信先切替のどちらが
復帰の要因かは確定できません。現在の設定ではdeep sleepとdevice PMは無効です。
CDCのpoll出力実装はUSB未接続時にデータを捨てるため、バッファ満杯で待ち続けるUART経路ではありません。

診断設定 `tests/ble-idle/ble-only.conf` はUSB HIDだけを無効にします。
USBログsnippetでCDCシリアルとUSB給電は維持し、HIDの送信先をBLEだけに限定します。
センサー・認識器・慣性・配線・調整値は変えません。通常使用へ戻すための
USB/BLE両対応UF2も残っています。この設定は原因切り分け用で、確定した対策ではありません。

実機確認：

1. USBを接続して給電・シリアルログを継続します。BLEでポインタが動くことを確認し、
   3分放置後、RESETせずポインタ・タップ・スクロールを試します。
   放置中から操作を試した後までのログを保存してください。
2. 同じファームでUSBを外し、バッテリー/BLEのみで3分放置して確認します。
   止まったらPC側の接続表示を確認してから、RESETせずUSBをつなぎ、復帰の有無を記録します。
3. 両方通れば30分でも繰り返します。比較のため、この間はPCのBluetooth省電力設定を変更しません。

1で止まる場合は、接触レポート・認識器出力・BLE接続状態・通知のどこで止まるかをログで調べます。
1が通り2だけ失敗する場合は、バッテリー動作またはUSBがない状態が関係しますが、
それだけで電圧不良とは断定できません。この版でUSB再接続時に復帰した場合、
HIDがBLEからUSBへ切り替わったことは原因から除外できます。

USB descriptorが変わるためCOM番号が変わる場合があります。
既存ペアリングが実際につながらなくなった場合を除き、BT_CLRや再ペアリングは不要です。
