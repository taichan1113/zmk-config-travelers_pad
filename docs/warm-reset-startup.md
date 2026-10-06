# Warm-reset startup / RESET後の起動

## English

The failed-reset USB log shows a configured USB endpoint, but all 40 product
probes return `0xEEEE`. At ten seconds, the driver is ERROR (`state=11`) with
zero contact reports. This is a sensor-startup failure, not evidence of a USB
HID transmission failure. The QSPI JEDEC mismatch is separate from these probes.

An MCU-only reset can leave the externally powered sensor in its previous Event
Mode. The supplied EV02 configuration selects the alternate Force Comms Method.
The [official datasheet, section 12.9.2](https://www.azoteq.com/images/stories/pdf/IQS9150_IQS9151_datasheet.pdf)
describes a communication request and a subsequent RDY window. The former probe
loop performed register reads regardless of RDY and stopped permanently after
approximately one second. The retained sensor state is consistent with the
reported difference between MCU RESET and removing power, but needs hardware
confirmation with the corrected sequence.

Startup now requests a window with `0xFF` when RDY is inactive, then reads the
product and configuration only with RDY active. Because STOP may close each
window, the configuration read and software-reset write can also request their
next window. An explicit sensor reset precedes ACK, configuration, and ATI.
Requests are bounded to 120 per waiting phase; failed communication remains an
explicit error rather than an unbounded loop. Normal event-mode idle sends no
requests. The previous runtime recovery and inertia cancellation fixes remain.

The production state-machine host test checks the warm-start request/RDY/product/
configuration/reset/ACK sequence, request limits, and silent runtime idle. It
uses a simulated bus and does not validate real sensor timing.

Hardware acceptance:

1. Flash with USB connected. Do not touch during ATI. Verify initialization and
   pointer movement without pressing RESET or removing USB.
2. With USB still connected, press RESET once, wait for ATI, and verify pointer
   and scroll. Repeat three times; removing USB must not be necessary.
3. Check battery-only BLE startup and subsequent USB reconnection. Repeat short
   scroll/glide cycles to confirm the inertia fix remains effective.

Success logs include a valid product `0x076a`, a reset request `0x0200`, Show Reset
observation, and `STEP1B complete`. If unsuccessful, capture the entire boot and
the ten-second `runtime health` line, including communication request messages.

## 日本語

RESET後のUSBログではUSB endpointは構成済みですが、製品番号の40回の読み出しが
すべて `0xEEEE` です。10秒時点でdriverはERROR（`state=11`）、接触レポート数は0です。
USB HID送信の停止ではなく、センサー初期化の失敗を示します。
QSPIのJEDEC不一致は、この製品番号読み出しとは別の問題です。

MCUだけのRESETでは、給電が続くセンサーに前回のEvent Mode設定が残る場合があります。
EV02設定は代替Force Comms Methodを選択しており、
[公式データシート12.9.2節](https://www.azoteq.com/images/stories/pdf/IQS9150_IQS9151_datasheet.pdf)
では通信要求を送った後にRDY窓が開く方式が説明されています。
従来のprobeはRDYに関係なくレジスタを読み、約1秒で恒久エラーに移行していました。
設定の保持はMCU RESETと電源の入れ直しの差に整合しますが、実機での確認が必要です。

起動時、RDY無効なら `0xFF` で通信窓を要求し、RDY有効時だけ製品番号・設定を読みます。
STOPで各窓が閉じる場合があるため、続く設定読み出し・ソフトウェアリセット書込みも
次の窓を要求できます。センサーを明示的にリセットしてからACK・設定・ATIを行います。
窓要求は待機段階ごとに最大120回とし、通信できない場合はエラーを明示します。
通常のEvent Mode無操作時には要求を送りません。先のruntime復旧と慣性修正も維持します。

実状態機械のホストテストで、窓要求→RDY→製品番号→設定→reset→ACK、要求回数制限、
通常の無操作時に要求しないことを確認します。バスは模擬しており、センサーの実タイミングは検証しません。

実機確認：

1. USB接続のまま書き込み、ATI中は触らず待ちます。
   RESETやUSB抜き差しをせず、初期化完了とポインタ移動を確認します。
2. USB接続のままRESETを1回押し、ATI完了後にポインタ・スクロールを確認します。
   3回繰り返し、USB抜き差しが不要であることを確認します。
3. バッテリーのみのBLE起動とUSB再接続を確認します。
   短いスクロールと慣性を繰り返し、先の慣性修正が有効であることも確認します。

成功時は製品番号 `0x076a`、リセット要求 `0x0200`、Show Reset確認、
`STEP1B complete` が出ます。失敗時は窓要求メッセージを含む起動全体と、
10秒時点の `runtime health` を保存してください。
