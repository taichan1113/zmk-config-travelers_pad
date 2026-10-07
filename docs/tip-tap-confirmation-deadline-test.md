# Contact confirmation deadline regression

[English](#english) | [日本語](#日本語)

## English

The submitted trace proves that valid second contacts reached the driver but
were sometimes hidden by session stabilization: reports 80/81 show two contacts
then one, with no intermediate report to re-evaluate the 35 ms confirmation.
This is a frame-driven deadline defect, not evidence of failed BLE key delivery.

The function module now schedules the pending confirmation deadline. A delayed
work item only enqueues a private, non-sync wake event. The session callback
checks the current pending state and deadline, then emits a complete canonical
frame through the same arbiter path. It never emits keys or claims ownership
from the timer. No extra sensor reads are performed. The latest cached physical
contact state is used; unreported physical changes cannot be known by this timer.

Callback state is protected by a recursive mutex, and a receiving-frame flag
prevents confirmation in the middle of a physical frame. Release/reset cancels
pending confirmation. An already queued stale wake is ignored or waits for the
current candidate's own deadline. Queue pressure retries the wake without
blocking the workqueue. An unsynchronized marker is not a contact frame for
downstream recognizers. Actual scheduling may be later than 35 ms under load.

The 35 ms stabilization policy, Tip-Tap-only 250 ms contact limit, 250 ms anchor
hold and 50 ms key presses remain unchanged. The prior driver diagnostics remain
enabled; this revision changes only function logic and test/config documentation.
Long second contacts can still legitimately exceed the 250 ms limit; that is a
separate tuning question.

Flash `travelers-pad-tiptap-confirm-deadline-20261008.uf2` from Windows Downloads.
Use BLE for input, USB for serial capture:

1. Hold a stationary anchor for 0.5 seconds, then perform 10 short second-finger
   taps on each side while preserving the anchor. Record missed action times.
2. Repeat with second contacts of about 0.2 seconds. Check `contact confirmation
   deadline reached`, followed by normalized 1->2 and Tip-Tap upon release.
3. Check ordinary two-/three-finger taps, one-finger pointer/drag, two-finger
   scrolling/inertia, and four-finger swipe/management hold for regressions.

Very brief contacts below 35 ms remain deliberately filtered. Submit full logs
with approximate failure times; no main merge/push occurs before BLE acceptance.
Host tests cover quiet-sensor confirmation, 100 short noise bursts, early/stale
wakes, release/reset, partial frames, queue pressure and the existing feature
regressions. Real hardware timing and radio delivery still require testing.

## 日本語

提出ログでは、有効な2本目がドライバへ届いたのにsession安定化で除外されています。
report 80／81では2本→1本となり、その間に35ms確認を再評価するreportがありません。
これはフレーム依存の期限処理不備で、BLEキー配信失敗を示すものではありません。

機能モジュールで確認期限を予約するよう修正しました。遅延workは内部の非sync
wakeイベントだけを投入し、session callbackが現在の候補と期限を確認して、
従来のarbiter経路へ完全な接触frameを発行します。タイマー自身はキーを発火せず、
所有権も取得しません。追加のセンサー読み取りも行いません。
最後に報告された物理接触状態を使うため、未報告の物理変化までは判断できません。

callback状態は再帰可能なmutexで保護し、物理frame受信途中は確定を延期します。
解放／resetで確認待ちを取り消し、投入済みの古いwakeは無視するか現在の候補自身の
期限を待ちます。キュー満杯時はworkqueueをブロックせずwakeを再試行します。
内部markerは非syncなので、下流認識器の接触frameとしては扱いません。
負荷によって実際の処理は35msより遅れる場合があります。

35ms安定化、Tip-Tap専用250ms上限、anchor静止250ms、キー押下50msは維持します。
前回のドライバ診断も維持し、今回の修正対象は機能ロジックと試験／設定文書です。
2本目の長い接触は引き続き250ms上限で棄却され、これは別のチューニング事項です。

Windows Downloadsの `travelers-pad-tiptap-confirm-deadline-20261008.uf2` を書き込み、
入力はBLE、USBはシリアル記録に使います。

1. anchorを0.5秒静止し、そのまま左右へ2本目を短く各10回タップ。不発時刻を控える。
2. 2本目を約0.2秒接触させる操作でも反復。`contact confirmation deadline reached`
   と正規化1→2、その後の解放でTip-Tapが発火することを確認。
3. 通常の2・3本指タップ、ポインタ／ドラッグ、2本指スクロール／慣性、
   4本指スワイプ／管理ホールドに退行がないか簡単に確認。

35ms未満の極短接触は引き続き意図的に除外します。不発時刻とログ全文を提出して
ください。BLE実機合格前にmain統合・pushは行いません。
自動試験はセンサー無報告中の確定、100回の短いノイズ、早期／古いwake、解放／reset、
部分frame、キュー圧力と既存機能の回帰を確認しています。実機タイミングと
無線配信の確認は別途必要です。
