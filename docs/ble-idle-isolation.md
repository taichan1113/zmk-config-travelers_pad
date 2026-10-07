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

### USB-removal follow-up and zero-latency comparison

The user confirmed continuous touch during the first 15–20 seconds after USB
removal, followed by stutter until USB was restored. The retained samples at
32–47 seconds have unchanged IRQ/raw/REL counters despite those touches; at
52–87 seconds all three counters progress on battery power. The system-workqueue
heartbeat progresses and BLE remains connected throughout. This establishes an
initial sensor/RDY input gap and later host-visible stutter despite generated
REL events, not a single proven cause. History does not measure HID delivery or
supply voltage. Deep sleep is disabled.

`tests/ble-idle/ble-zero-latency.conf` changes only the preferred BLE peripheral
latency request from 30 to 0 relative to the history profile. Driver, recognizers,
connection interval preference and history collection are unchanged. This is an
A/B diagnostic, not a fix for absent RDY; latency 30 alone does not prove the
cause of stutter. The host may choose different parameters: check actual `lat=0`
in the history after connection. Increased battery consumption is possible.

With battery ON, unplug USB and immediately move one finger in the center for
30 seconds; separately note initial silence and subsequent stutter. Then idle
three minutes and retest movement/tap/scroll. Reattach USB without RESET and
save the full history. If actual latency is not zero, report that before drawing
conclusions; do not clear pairing as a first step.

### Input-flow history after the zero-latency test

The host accepted latency zero before USB removal. During continuous attempted
movement, samples at 22–27 seconds showed raw frames increasing 584→980 while
REL remained 819. At 32 seconds the cached count was two despite the intended
one-finger test. A count sampled once is not proof of the complete contact
sequence. At 42 seconds the cached count was zero. The reported stop cannot be
attributed solely to preferred latency 30. Raw frame count does not prove
changing coordinates, so no recognition or power fix is justified yet.

The diagnostic adds a `flow` line with the same sampling timestamp as `history`:
`raw-move` counts changed Finger-1 coordinates on consecutive one-finger frames;
`changes` counts contact-count transitions; `pos` is the latest raw coordinate;
`routed`/`routed-move` observe the configured pointer input device;
`routed-fingers` is its cached count; `markers` counts routed reset-marker frames
(normal routing can emit these without sensor reset); `xy` and `wheel` separate
nonzero relative pointer and scroll events. Counters are cumulative, not rates.
Fields are separate atomic reads, not a transactionally consistent frame capture.
Observation callbacks do not perform I2C, claim ownership or change routing.
BLE latency zero and the five-second history interval are retained.

Repeat continuous one-finger movement in the center for 30 seconds after USB
removal. If stopped, keep moving that finger for another ten seconds before
reattaching USB without RESET. Report roughly how many seconds after unplugging
the stop occurred, whether the finger stayed down, and save both line types.
Do not start the three-minute idle test until this immediate failure is isolated.

### Transient contact-count investigation

The input-flow test showed raw coordinates continuing to change while routed
frames paused: at 27.362 seconds raw=1335/raw-move=1332, but routed=1219,
routed-fingers=0 and markers=1. Contact-count changes rose from 1 to 3 between
22 and 27 seconds. This proves raw movement continues through at least part of
the routed suppression, but the five-second samples do not identify the exact
intermediate count or owner. The user reports faster motion can trigger a stop
and lifting/recontact restores motion.

Code inspection finds that unowned ROUTER_PASS_ONE becomes ROUTER_AWAIT_TWO on
any reported count >=2; a subsequent count of one does not restore PASS_ONE.
The router intentionally suppresses pointer until full release or a committed
route. This is a plausible match, not yet proof that a phantom second contact
caused the failure. Broadly resuming pointer after every multi-to-one transition
could break scroll, pinch and Tip-Tap isolation and is not applied.

The next diagnostic adds a 96-entry contact-transition RAM ring. It records each
raw count change/reset frame with uptime, before/after count, both legacy
coordinates, second-contact validity, VBUS and cumulative raw/XY observations.
`raw` is counted before this sync frame, and `xy` may lag asynchronous routing.
The ring is copied under a short spinlock before logging, then printed as
`contacts begin` / `contact` / `contacts end` before `history end`. It does not
register an arbiter observer or alter decisions. Only the most recent 96 changes
are retained, independently of the five-second history duration.

With one finger staying down, move slowly for five seconds after unplugging,
then faster until a stop. Keep moving ten seconds after stopping, then lift once
and retouch to check recovery. Reattach USB without RESET and save the full dump.
Note whether the finger stayed down until the deliberate recovery lift.

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

### USB取り外し直後の追加確認とlatency=0比較

USB取り外し直後15〜20秒も指を動かし、その後もUSB再接続までカクついたとの確認です。
起動32〜47秒の履歴は操作中にもIRQ/raw/RELが増えず、52〜87秒には3つとも増えています。
全期間でsystem workqueueは進み、BLE接続も維持しています。
直後のセンサー/RDY入力の空白と、その後のREL生成中のカクつきは確認できますが、
単一の原因とは確定できません。HID通知の到達・実電圧は履歴にありません。deep sleepは無効です。

`tests/ble-idle/ble-zero-latency.conf` は履歴版に対してBLE peripheral latencyの希望値だけを
30から0に変更します。driver・認識器・接続間隔の希望値・履歴採取は変更しません。
原因比較用であり、RDYが来ない問題の対策ではありません。latency=30だけでカクつきの
原因とは断定しません。PC側の決定もあるため、履歴の実際の `lat=0` を確認します。
電池消費が増える可能性があります。

バッテリーONでUSBを外し、直後から中央で1本指を30秒動かします。
最初の無反応時間と、その後のカクつきを分けて記録してください。
次に3分放置後、移動・タップ・スクロールを確認し、RESETせずUSBを再接続して履歴全体を保存します。
実際のlatが0でなければ先にその旨を報告し、最初からBT_CLRはしません。

### latency=0確認後の入力経路履歴

USB取り外し前に実際のlatency=0が反映されています。連続移動操作中の起動22〜27秒では
rawが584→980に増え、RELは819のままでした。32秒の指本数は2で、1本指を意図した
操作との違いもあります。ただし5秒ごとの本数だけで接触の全経過は判断できません。
42秒には本数0になっています。latency=30だけでは停止を説明できず、rawフレーム数
だけでは座標変化を証明できないため、現時点で認識や電源の修正を断定しません。

診断版は `history` と同じ採取時刻の `flow` 行を追加します。
`raw-move` は連続する1本指フレーム間のFinger 1座標変化回数、`changes` は指本数変化回数、
`pos` は最新の生座標です。`routed`/`routed-move` は設定されたポインタ入力デバイスの
転送フレーム数と1本指座標変化回数、`routed-fingers` はその最新本数です。
`markers` は経路切替等のreset marker付きフレーム数で、センサーリセット回数ではありません。
`xy` と `wheel` は非ゼロのポインタ移動・スクロールイベントを区別します。
回数は累積値です。各値は別々のatomic読出しで、完全に同時のフレームキャプチャではありません。
観測callbackはI2C・所有権取得・経路変更をしません。latency=0と5秒間隔採取は維持します。

USBを外した直後から中央で1本指を30秒動かし、停止しても同じ指をさらに10秒動かしてから、
RESETせずUSBを戻してください。取り外し何秒後に止まったか、指を離さず動かし続けたかも
記録し、history/flowの両方を保存します。直後の停止を切り分けるまで3分放置テストは保留します。

### 一時的な指本数変化の調査

入力経路試験では、起動27.362秒にraw=1335/raw-move=1332まで生座標変化が継続する一方、
routed=1219、routed-fingers=0、markers=1で転送が抑制されています。
22〜27秒で指本数変化は1→3回に増えました。転送抑制の少なくとも一部で生座標が
動いていることは確認できますが、5秒間隔では途中の具体的な本数・ownerは分かりません。
利用者は速い移動で止まりやすく、離して再接触すると復帰すると報告しています。

コードではownerなしのROUTER_PASS_ONEが本数2以上の報告でROUTER_AWAIT_TWOになり、
その後本数1に戻ってもPASS_ONEへ戻りません。全接触解除またはroute確定までポインタを
抑制する仕様です。症状と整合しますが、架空の2点目が原因との確証はまだありません。
多指→1本を全てポインタへ戻す変更はscroll/pinch/Tip-Tapの分離を壊す恐れがあるため実施しません。

次の診断版は96件の指本数変化RAMリングを追加します。生frameの本数変化/resetごとに、
起動経過時刻・変化前後の本数・2点のlegacy座標・2点目有効フラグ・VBUS・raw/XY累積回数を保存します。
rawはそのsync frameの計上前、XYは非同期経路の処理により遅れる場合があります。
短いspinlock内でcopyしてからログ出力し、`history end` の前に
`contacts begin` / `contact` / `contacts end` を出します。arbiter observerを登録せず裁定も変えません。
直近96回分だけを保持し、5秒間隔履歴とは保存期間が独立しています。

USB取り外し後、1本指を置いたまま5秒ゆっくり動かし、次に速く動かして停止を再現します。
停止後も10秒動かし、最後に一度だけ離して再接触し復帰を確認します。
RESETせずUSBを戻し履歴全体を保存してください。復帰確認で意図的に離すまで、指を置いたままだったかも記録します。
