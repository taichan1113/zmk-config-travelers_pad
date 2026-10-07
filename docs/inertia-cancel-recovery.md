# Inertia cancellation recovery / 慣性キャンセル後の復帰

## English

### Evidence and cause

The supplied USB log starts inertia normally in sessions 9–11. Sessions 12–16
still emit ordinary wheel movement, but every terminal frame logs
`scroll inertia skipped: stale/no motion`. Lease creation continues normally.

The inertia worker sets `emitting` before broker output. If that output loses
its post-session lease, its `-EACCES` branch returns without reaching the normal
`emitting = false` epilogue. Previously, state clearing also left this flag set.
Subsequent canonical wheel samples were then ignored indefinitely, even after a
new contact cleared velocity history. This is a confirmed code defect; the
specific lease-denial event is not visible in the supplied INFO-level log.

### Fix and regression

State clearing now resets the transient emission guard. The existing lease gate,
new-contact interruption and synchronous work cancellation are unchanged.
Lease-denial output paths now log their cancellation reason at INFO level.
No speed, sensitivity, decay or scroll-direction parameters were changed.

`bash scripts/test-inertia-cancel.sh` compiles the production inertia handlers
with fake work/clock/broker interfaces. The old source fails the cancellation
assertion; the fixed source passes 256 cancel/restart cycles covering both axes
and time-based/legacy inertia. It also checks a non-permission output error and
a missing lease. It does not prove electrical timing or real concurrent scheduling.
The structural regression script runs this test alongside driver recovery tests.

### Pointer evidence and limits

The same log shows ordinary pointer movement in sessions 4–7. Driver health at
40 and 50 seconds reports RUNNING (`state=8`), inactive RDY, and zero report
errors/rescues; pointer output resumes at 51 seconds. Sessions 9–16 then belong
to two-finger scrolling, where suppression of pointer output is intentional.
There is no long-idle pointer freeze or reset-recovery loop in this log.
Therefore this change fixes inertia, not a newly established pointer-freeze cause.
The previous reset/RDY recovery fix remains included in the diagnostic firmware.

### Hardware checks

1. Repeat short vertical and horizontal two-finger scroll strokes 20–30 times.
   Retouch before each preceding glide ends. Every moving stroke should be able
   to seed a fresh glide after release, without RESET.
2. Interrupt a glide with a stationary touch. The glide should stop; the next
   two-finger moving stroke should start inertia normally. Stationary taps should
   not start inertia.
3. Leave untouched for 5–10 minutes or the original reproduction interval, then
   try pointer and scroll without RESET. This separately validates idle recovery.

If pointer motion stops, preserve health lines and contact/arbiter logs while
attempting one-finger movement in the central non-edge region. Record whether
scroll and taps still work. If inertia fails, preserve the final working glide,
the next contact, and the first `skipped` message. The diagnostic image omits the
new BLE management layer and retains the existing bindings and tuning.

## 日本語

### ログと原因

USBログではsession 9〜11で慣性が正常に開始しています。session 12〜16でも
通常のホイール移動は出力されますが、指を離すたびに
`scroll inertia skipped: stale/no motion` が出ます。lease作成は続いています。

慣性workerはbroker出力前に `emitting` を立てます。その出力がセッション後の
権限を失うと、`-EACCES` の経路で通常末尾の `emitting = false` を通らずreturnします。
従来は状態クリアもこのフラグを解除せず、次の接触で速度履歴を消した後も、
通常ホイール値の観測を無期限に無視する状態になっていました。
コード上の不具合は確認済みですが、実際の権限拒否の瞬間は今回のINFOログには出ていません。

### 修正と回帰テスト

状態クリア時に一時的な出力中フラグも解除します。既存のleaseによる出力制限、
再接触による慣性停止、同期workキャンセルは維持します。
出力時の権限拒否はINFOで理由を表示します。
速度・感度・減衰・反転などの調整値は変更しません。

`bash scripts/test-inertia-cancel.sh` は実際の慣性handlerを、work・時刻・brokerの
スタブと組み合わせてコンパイルします。修正前ソースはキャンセル後の検査に失敗し、
修正後は縦横と時間基準・旧方式を含む256回のキャンセル→再開に合格します。
権限拒否以外の出力エラー、lease取得失敗も確認します。
電気的タイミングや実際の並行スケジューリングを保証するテストではありません。
構造回帰スクリプトからも、ドライバ復旧テストと合わせて実行します。

### ポインタについて分かること・分からないこと

同じログではsession 4〜7で通常ポインタが動いています。40秒・50秒のhealthは
RUNNING（`state=8`）、RDY無効、レポートエラー・救済回数0で、51秒からポインタ出力が再開します。
session 9〜16は2本指スクロールに所有権があり、ポインタ出力の抑止は仕様どおりです。
長時間放置後の停止や、リセット復旧の無限待ちを示すログは含まれていません。
したがって今回の修正は慣性に対するもので、新たなポインタ停止原因を確定したものではありません。
先のreset/RDY復旧修正は検証用ファームに引き続き含めます。

### 実機確認

1. 短い縦・横の2本指スクロールを各20〜30回繰り返します。
   前の慣性が終わる前に触り直し、毎回の移動後に指を離しても慣性を再開できることを確認します。
2. 慣性中に静止した指を置いて停止させ、次の2本指移動で再び慣性が始まることを確認します。
   静止タップだけでは慣性が始まらないことも確認します。
3. 5〜10分、または以前の再現時間まで放置し、RESETせずポインタとスクロールを確認します。
   これは慣性とは別に、放置復帰を検証する項目です。

ポインタが止まったら、エッジを避けた中央で1本指移動を試しながら、healthと
接触・arbiterログを保存してください。スクロールやタップが反応するかも記録してください。
慣性が止まった場合は、最後の正常な慣性から次の接触、最初の `skipped` までを残します。
今回の検証版は新しいBLE接続管理を含まず、既存のキー割当と調整値を維持します。
