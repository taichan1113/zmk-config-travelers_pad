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

### Battery-only RAM history

The follow-up log after reattachment showed `age=549033 ms`, no new reset or
recovery, and normal movement after USB returned. This indicates no successful
sensor report for about nine minutes, but does not distinguish absent RDY from
blocked register/input processing before attachment.

The diagnostic now enables `CONFIG_TRAVELERS_PAD_BLE_IDLE_HISTORY`. A separate
thread stores 96 snapshots at five-second intervals in RAM (nominally eight
minutes), independently of the system workqueue. A second GPIO callback counts
existing RDY interrupts without changing the interrupt configuration. Input
callbacks count raw synchronized frames and nonzero pointer/wheel REL events.
The sampler also records a system-workqueue heartbeat, VBUS presence, BLE
connection state/parameters, selected endpoint and cached battery percentage.
It performs no sensor I2C, output, reset or flash writes. It does not capture
actual supply voltage or prove delivery of BLE notifications to the PC.

When USB VBUS and serial DTR become active, it dumps retained samples between
`history begin` and `history end`. The `t` field is sampling uptime, not USB
print time. Dumping is paced on the diagnostic thread and temporarily pauses
sampling; diagnostic scheduling can affect timing. History survives USB removal,
but not MCU RESET or loss of MCU power.

With the history UF2, first verify a history dump with USB/serial open. Then
disconnect USB with battery ON, wait for motion to work, and idle for three to
five minutes. If stopped, attempt central pointer movement, tap and scroll for
ten seconds; reconnect USB without RESET and save the entire history dump.
Keep the pad untouched while the dump finishes. If no dump appears, close and
reopen the serial monitor without RESET. Do not wait more than eight minutes
after the failure before collecting the history.

| History while attempting touch | Investigation focus |
| --- | --- |
| IRQ unchanged, RDY inactive, queue heartbeat progresses | Sensor/RDY signal path or sensor power; no voltage conclusion yet |
| IRQ increases or RDY remains active, raw frames unchanged | Sensor communication or driver/input processing |
| Queue heartbeat stops while sampler timestamps continue | System workqueue blockage |
| Raw frames increase but REL does not | Recognition/output ownership, depending on attempted action |
| REL increases, no visible host motion | BLE HID/output path or host; connection state alone does not prove delivery |

The new callbacks are diagnostic-only observers, not an architectural change to
the reusable modules. The original driver and features revisions remain unchanged.

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

### バッテリー動作中のRAM履歴

再接続後の追加ログは `age=549033 ms` で、新たなreset/recoveryはなく、USB接続後に
通常移動が再開しました。約9分間、センサーレポートが正常に読まれていなかったことを示しますが、
接続前にRDYが来なかったのか、通信・入力処理が止まっていたのかは区別できません。

診断版に `CONFIG_TRAVELERS_PAD_BLE_IDLE_HISTORY` を追加しました。
system workqueueとは別のスレッドで、5秒間隔・96件、通常約8分ぶんの状態をRAMに保持します。
既存の割込み設定を変更せずGPIO callbackを追加してRDY割込みを数え、入力callbackで
生の同期フレーム数と、非0のポインタ・ホイールRELイベント数を数えます。
system workqueueの進行、VBUS、BLE接続状態・パラメータ、送信先、キャッシュ済み電池残量も記録します。
I2C通信・操作出力・リセット・flash書込みは行いません。
実際の電源電圧や、BLE通知がPCへ届いたかはこの記録だけでは測れません。

USBのVBUSとシリアルDTRが有効になると、`history begin`〜`history end` の間に履歴を出します。
`t` はUSB出力時刻ではなく、状態を採取した時点の稼働時間です。
履歴出力中は診断スレッドで出力を間引き、採取を一時停止します。診断処理がタイミングに影響する可能性はあります。
USB抜去では履歴を保持しますが、MCU RESETや電源喪失では消えます。

履歴版UF2で、まずUSB・シリアル接続中に履歴出力が出ることを確認します。
バッテリーONでUSBを外し、動作を確認してから3〜5分放置します。
停止したら中央で移動・タップ・スクロールを約10秒試し、RESETせずUSBを接続して履歴全体を保存してください。
`history end` まで触らず待ちます。履歴が出なければRESETせずシリアルモニターを閉じて開き直します。
停止してから履歴取得まで8分以上待たないでください。

| 操作を試している間の履歴 | 調査対象 |
| --- | --- |
| IRQが増えずRDY無効、queueは進む | センサー・RDY信号経路・センサー給電。電圧原因とは未確定 |
| IRQ増加またはRDY有効のまま、rawが増えない | センサー通信・driver・入力処理 |
| 採取時刻は進むがqueueが止まる | system workqueueの停止 |
| rawは増えるがRELが増えない | 試した操作に応じて認識・出力所有権 |
| RELが増えるのにPCでは動かない | BLE HID送信・PC側。接続表示だけでは通知到達を証明できない |

追加callbackは診断限定の観測者です。再利用モジュールのアーキテクチャ変更ではなく、
driver・featuresの参照revisionも維持しています。

USB descriptorが変わるためCOM番号が変わる場合があります。
既存ペアリングが実際につながらなくなった場合を除き、BT_CLRや再ペアリングは不要です。
