# Idle resume recovery / 放置後の復帰

## English

### Diagnosis

The reported `runtime recovery waiting for Show Reset` loop places the driver in
reset verification, not normal event-mode inactivity. The previous implementation
sent System Control `0x0002`. According to the
[IQS9150/IQS9151 datasheet](https://www.azoteq.com/images/stories/pdf/IQS9150_IQS9151_datasheet.pdf),
Appendix A.12, software reset is bit 9 (`0x0200`, transmitted as `00 02`).
Bit 1 belongs to Mode Select. Waiting indefinitely for reset acknowledgement
after sending a mode command explains why contact reporting could not resume.
The supplied snippet does not identify the original event that started recovery.

### Changes

- Send the documented software reset command.
- Limit Show Reset verification to two seconds per request, at most three requests.
  Failed verification is reported explicitly rather than an unlimited warning loop.
- Use the logical active value `1` from `gpio_pin_get_dt()` with `GPIO_ACTIVE_LOW`.
  Perform window-bound transactions only while RDY is asserted.
- Correct Config Settings termination detection to low-byte bit 6 (Appendix A.13).
  The EV02 value `0x461E` leaves it clear, so STOP ends the communication window.
  High-byte bit 6 is Switch Event Enable, not communication termination.
- Check RDY level every 250 ms, requeueing work if RDY is asserted with no work
  pending. Inactive event-mode silence does not initiate I2C or reset.
- Include termination failures in the existing consecutive report failure recovery.
- Enable ten-second runtime health logs for this diagnostic firmware.

The verification deadline bounds waiting after a reset request. It does not
guarantee recovery from a physically stuck bus or absent RDY window. A failed
driver enters its explicit error state. The level check is configurable through
`rdy-watch-ms`; zero disables it. Health logs use `runtime-diagnostics`.

### Test scope

This firmware keeps the pre-management keymap and features module (`v0.1.0`).
The new four-finger hold management layer and RGB widget are intentionally absent.
USB and BLE remain enabled. No hardware wiring or gesture thresholds change.

Run `bash scripts/test-iqs9150-recovery.sh` after selecting the fixed driver.
This host test compiles the production state machine against GPIO/I2C/work stubs.
It checks reset bytes, active-low gating, success and timeout paths, inactive idle,
stranded RDY recovery, termination flags, and preservation of pending work.
It does not simulate real electrical timing, BLE, or sensor calibration.

### Hardware checks

1. Flash the provided UF2. Keep fingers off until initialization/ATI completes.
   Check pointer, tap/drag and two-finger scroll.
2. Leave the pad untouched for 1, 5 and 10 minutes (or longer than the observed
   failure interval). Resume pointer and scroll without RESET or USB reconnection.
3. Repeat under battery-only BLE, then reconnect USB and confirm continued operation.

`runtime health` every ten seconds is intentional. During inactive event-mode
idle, unchanged report counters and increasing `age` are normal. After touching,
`reports` should increase again. Level rescue warnings indicate requeued work;
they do not by themselves demonstrate a successful recovery.
If reset recovery occurs, look for `software reset 0x0200 requested`,
`Show Reset observed`, and initialization/ATI completion.
If stopping recurs, retain the preceding recovery reason and health lines,
not only the last warning. Do not count this issue as hardware-verified until
the idle/resume checks pass.

## 日本語

### 原因

報告された `runtime recovery waiting for Show Reset` の連続出力は、正常な
Event Modeの無通信ではなく、ドライバがリセット確認状態で停止していることを示します。
従来はSystem Controlへ `0x0002` を送っていました。しかし、
[IQS9150/IQS9151公式データシート](https://www.azoteq.com/images/stories/pdf/IQS9150_IQS9151_datasheet.pdf)
の付録A.12では、ソフトウェアリセットはbit 9、すなわち `0x0200`
（送信バイト列 `00 02`）です。bit 1はMode Selectの一部です。
モード指定を送った後にリセット通知を無期限に待っていたため、接触レポートが再開できませんでした。
ただし、復旧処理を開始させた最初の事象は、今回の抜粋だけでは特定できていません。

### 修正内容

- 公式仕様のソフトウェアリセットコマンドへ訂正。
- リセット要求後のShow Reset確認を1回2秒、最大3回に制限。
  無期限の警告ループではなく、復旧失敗を明示。
- `GPIO_ACTIVE_LOW` 指定時の `gpio_pin_get_dt()` の論理有効値を `1` として扱い、
  通信窓が必要なアクセスをRDY有効時に限定。
- Config Settingsの通信終了方式を下位バイトのbit 6で判定（付録A.13）。
  EV02の `0x461E` ではこのbitは0なのでSTOPで通信窓が終了します。
  上位バイトのbit 6はSwitch Event Enableであり、通信終了指定ではありません。
- 250ms間隔でRDYのレベルだけを確認し、RDY有効なのにworkが未予約の場合に再予約。
  無操作時間だけを理由にI2C通信やリセットを強制しません。
- 通信終了処理の連続失敗も、既存のレポート失敗復旧の対象へ追加。
- 今回の検証版では10秒間隔のruntime healthログを有効化。

確認期限は、リセット要求後の待機に上限を設けるものです。
物理的なバス固着やRDY窓が来ない障害まで復旧を保証するものではありません。
確認失敗時には明示的なエラー状態になります。
レベル監視は `rdy-watch-ms` で設定でき、0で無効化できます。
healthログは `runtime-diagnostics` で切り替えます。

### 検証範囲

接続管理追加前のキーマップと機能モジュール `v0.1.0` を使用しています。
新しい4本指ホールド管理レイヤーとRGB widgetは含めません。
USB/BLEは従来どおり有効です。配線・ジェスチャー閾値は変更しません。

修正版ドライバを選択後、`bash scripts/test-iqs9150-recovery.sh` でホストテストを実行します。
実際のドライバ状態機械をGPIO/I2C/workのスタブと組み合わせ、リセット送信値、
active-low判定、復旧成功・時間切れ、無操作時、RDY取り残し、通信終了bit、
予約済みworkの維持を確認します。電気的タイミング、BLE、センサー校正は再現しません。

### 実機確認

1. UF2を書き込み、初期化・ATI完了まで触らず待ちます。
   ポインタ、タップ・ドラッグ、2本指スクロールを確認します。
2. 1分・5分・10分（または以前の再現時間以上）放置し、RESETやUSB抜き差しをせず、
   ポインタとスクロールを再開できることを確認します。
3. バッテリーのみのBLEでも繰り返し、USB再接続後も操作が続くことを確認します。

10秒間隔の `runtime health` は今回意図した出力です。Event Modeで無操作なら、
レポート数が増えず `age` が増えるのは正常です。触ると `reports` が再び増えるはずです。
level rescue警告はworkの再予約を示しますが、それだけで復旧成功とは判断しません。
復旧が発生した場合は、`software reset 0x0200 requested`、`Show Reset observed`、
初期化・ATI完了まで確認してください。
再発したら、最後の警告だけでなく、その前の復旧開始理由とhealthログも残してください。
放置・復帰の実機確認が通るまでは、本件を実機検証済みとはしません。
