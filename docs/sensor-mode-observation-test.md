# Passive sensor mode / ALP observation

## English

Driver/pad branch: `debug/sensor-mode-observation`. Features are unchanged.
UF2: `C:\Users\takec\Downloads\travelers-pad-sensor-mode-observation-20261007.uf2`.
This diagnostic deliberately restores `power-mode-limit = "none"` (LP2 allowed)
to investigate the original failure, keeping BLE-only HID/latency-zero/history.
Fallback: the accepted `travelers-pad-sensor-idle-only-20261007.uf2`.

The optional driver `power-diagnostics` reads two setup blocks before ATI,
inside normal RDY windows, without modifying settings. Logs report five sampling
periods, five timeouts, reference interval, Config/Other Settings, ALP setup and
Tx enable, thresholds, debounce and LP1/LP2 filters. Runtime diagnostics use
Info Flags already present in the normal 72-byte contact report. A spinlock
protects the cached snapshot; the diagnostic accessor performs no I2C, wake,
reset, or host input. No new recognizer or event is added to the contact pipeline.

History gains `sensor` rows with `observed-at`, `age-ms`, `last-mode`, raw
`info`, ALP prox/error and eight cumulative mode report counters. Mode codes:
0 Active, 1 Idle-Touch, 2 Idle, 3 LP1, 4 LP2; 5–7 reserved. Counter values count
reports, not time in mode. All-EEEE invalid observations are excluded. An
`observed-at=0` / `age-ms=-1` means no observation; last-mode then has no meaning.
Reports containing Show Reset retain that flag so a reset is visible.

**Passive observation cannot prove a silent mode.** Event Mode may emit no
report for a power transition. The last observed mode can be stale throughout
idle, and the first wake report may already be Active. Observing mode 4 confirms
LP2 at that report; no mode-4 report does not prove LP2 was never entered.
USB reconnection can change conditions, so pre-reconnection timestamps matter.
The setup logs are readback configuration, not proof of runtime mode residency.

## Investigation findings

Bundled settings: ALP Setup=0xD3FFFFFF (ALP enabled), Other Settings=0x80A8:
LP1 Auto-Prox disabled, LP2 Auto-Prox enabled with 64 cycles. ALP output and
Auto-Prox thresholds are both 8; debounce 2/2; LP1 filters 70/4 and LP2 filters
180/100. LP1 sample period is 50ms; LP2 is 100ms. The latest successful LP1-only
test therefore did not test the same Auto-Prox path as unrestricted LP2.

This makes LP2 Auto-Prox a specific next comparison candidate, not a proven
cause. Runtime ALP ATI flags, calibration/reference, electrode topology and
battery supply remain candidates. Do not tune thresholds or force ALP ATI
simultaneously yet. A subsequent LP2-Auto-Prox-off comparison can isolate that
setting if the failure reproduces. True idle residency/wake latency might need
a separately labeled intrusive test or external electrical observation; passive
history alone cannot measure the instant the user first touched the surface.

## Hardware checks

1. Capture boot `power setup:` lines. Confirm other=0x80a8, alp-enabled=1,
   lp1-autoprox=0, lp2-autoprox=1 and timeout order 10/60/5/40, active-idle=1500.
2. Unplug USB and confirm pointer/taps/scroll. Leave untouched for 5 minutes.
3. Before USB or RESET, move one finger for 30 seconds, then tap and scroll.
   Note delay, approximate first attempted touch time, and failure/recovery.
4. Reconnect USB and capture **all** history including `sensor` rows. If it fails,
   reconnect without resetting first so the RAM history remains available.

Refs: [Azoteq official datasheet](https://www.azoteq.com/images/stories/pdf/IQS9150_IQS9151_datasheet.pdf), sections 5.6.3, 6, and appendices A.4/A.14/A.15.

## 日本語

driver/pad：`debug/sensor-mode-observation`。機能モジュールは変更しません。
UF2：`C:\Users\takec\Downloads\travelers-pad-sensor-mode-observation-20261007.uf2`。
元の停止問題を調べるため **`power-mode-limit = "none"` に戻し、LP2を許可**します。
BLE専用HID、latency=0、historyは維持。戻し先は合格したIdle維持版
`travelers-pad-sensor-idle-only-20261007.uf2` です。

任意設定 `power-diagnostics` はATI前のRDY通信窓で2つの設定ブロックを読み出します。
sampling、timeout、reference周期、Config/Other Settings、ALP setup/Tx、閾値、
debounce、LP1/LP2 filterを表示します。設定は変えません。
runtimeでは通常72バイト接触report内のInfo Flagsだけを保存。spinlock付きRAM取得APIは
I2C・復帰・RESET・ホスト入力を行いません。接触経路に認識器やイベントを追加しません。

historyに `sensor` 行が追加されます。採取時刻 `observed-at`、古さ `age-ms`、
最終採取mode `last-mode`、Info Flags、ALP検出/ATI error、mode別report累計です。
0=Active、1=Idle-Touch、2=Idle、3=LP1、4=LP2、5〜7=予約。累計は滞在時間ではありません。
全EEEEは観測から除外。`observed-at=0 / age-ms=-1` は未観測で、mode値は無効です。
Show Resetを含むreportはフラグを保持してRESETの発生を見えるようにします。

**受動観測では無通信中の現在modeを証明できません。** Event Modeでは省電力遷移の
reportがない可能性があり、最後のmodeは古くなります。復帰後の最初のreportは既にActive
かもしれません。mode=4の観測はその採取時点のLP2を示しますが、4がないからLP2未移行
とは判断しません。USB再接続後は条件が変わるため、再接続前の採取時刻を見ます。

### 調査結果

同梱値はALP Setup=0xD3FFFFFFでALP有効、Other Settings=0x80A8。
**LP1 Auto-Prox無効、LP2 Auto-Prox有効（64 cycles）** です。
ALP通常/Auto-Prox閾値は8/8、debounce=2/2、LP1 filter=70/4、LP2 filter=180/100。
LP1 sampling=50ms、LP2=100ms。合格したLP1版ではLP2と同じAuto-Prox経路を試していません。

LP2 Auto-Proxは次の比較対象として具体化しましたが、原因確定ではありません。
ALP ATI状態、校正/reference、電極構成、電池給電も候補です。今回は閾値変更やALP ATIの
強制実行を混ぜません。再現したら次にLP2 Auto-Proxだけ無効にする比較を検討できます。
無通信中の実modeや正確な復帰遅延には、別枠の介入診断や外部電気的観測が必要になる
可能性があります。受動historyだけでは実際に指を置いた瞬間は分かりません。

### 実機確認

1. 起動の `power setup:` 全行を保存。other=0x80a8、alp-enabled=1、lp1-autoprox=0、
   lp2-autoprox=1、timeout=10/60/5/40、active-idle-ms=1500を確認。
2. USBを外してポインタ・タップ・スクロール確認後、5分間無接触で放置。
3. USB/RESET前に1本指移動30秒、タップ・スクロール確認。ラグや停止の有無、指を置いた
   およその時刻も記録。
4. USBを戻して `sensor` を含むhistory全文を保存。停止しても先にRESETせず、RAM履歴を残す。

根拠：[Azoteq公式データシート](https://www.azoteq.com/images/stories/pdf/IQS9150_IQS9151_datasheet.pdf) 5.6.3節、6章、付録A.4/A.14/A.15。
