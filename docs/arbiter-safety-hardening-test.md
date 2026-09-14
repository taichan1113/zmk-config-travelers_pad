# Arbiter safety hardening test / Arbiter安全性強化テスト

Use this procedure after flashing a build that includes lifecycle-epoch,
auxiliary-lease, work-generation, broker retry, or arbiter-lock changes. It
does not replace the full integration test. It is the evidence for reset and
delayed-work failure paths.

この手順は lifecycle epoch、auxiliary lease、work generation、broker retry、または
arbiter lockを変更したビルドを書き込んだ後に使用する。全入力結合試験を置き換えるものではなく、
resetとdelayable workの異常系に対する証跡である。

## Preparation / 準備

1. Build the USB-logging target and manually copy its `zmk.uf2` to the XIAO
   UF2 drive. Do not reset the board while copying.
2. Open the USB serial console at the configuration's normal logging rate and
   start capture before touching the pad. Save one file per case, for example
   `arbiter-safety-YYYYMMDD-caseN.log`.
3. Filter or retain lines containing `arbiter`, `edge-hold`, `scroll inertia`,
   `tap/drag`, `broker`, `output report failed`, and `session`.

1. USB logging付きtargetをbuildし、`zmk.uf2`をXIAOのUF2 driveへ手動でコピーする。
   コピー中にboardをresetしない。
2. 通常のlogging rateでUSB serial consoleを開き、padに触れる前からcaptureを開始する。
   ケースごとに`arbiter-safety-YYYYMMDD-caseN.log`のような別fileで保存する。
3. `arbiter`、`edge-hold`、`scroll inertia`、`tap/drag`、`broker`、
   `output report failed`、`session`を含む行を保存またはfilterする。

## Case 1: reset during inertia / inertia中のreset

1. Make an axis-locked 2F scroll and release both fingers so that inertia
   begins. Confirm `arbiter post-session lease` and `scroll inertia lease
   acquired` in the log.
2. While wheel continuation is visible, cause the IQS9150 runtime recovery
   path or reset the sensor by the established diagnostic procedure.
3. Keep the pad untouched for at least two inertia periods, then begin a new
   1F pointer contact and a fresh 2F scroll.

Expected evidence:

- `arbiter global cancel epoch=... reason=sensor-reset-event`
- `scroll inertia cancelled by sensor reset`
- no `scroll-inertia-wheel` or `scroll-inertia-hwheel` output after the global
  cancel and before the new contact
- the new contact starts a new session and works normally

## Case 2: reset after release, before delayed tap / release後の遅延tap前reset

1. Perform a short 1F tap and leave the pad untouched; the first click is in
   the double-tap waiting interval.
2. Before that interval ends, trigger IQS9150 recovery/reset.
3. Wait longer than `double-tap-max-gap-ms`, then make one fresh tap.

Expected evidence:

- the global cancel log appears before the old timer could emit a click
- no old `single tap confirmed` or click action appears after reset
- only the fresh tap can later produce a click

## Case 3: stale edge-hold work / 古いedge-hold work

Run this case twice, once for drag and once for axis-locked 2F scroll.

1. Reach the relevant forward edge and wait until `continuation started` is
   logged.
2. Trigger sensor recovery/reset while continuation is armed or repeating.
3. Immediately make another operation of the same kind, including reaching an
   edge again.

Expected evidence:

- the reset produces `arbiter global cancel epoch=...`
- the old continuation produces no motion or wheel output after that line
- any new continuation is preceded by a new `continuation started` line and
  follows only the new contact

## Case 4: button release and recovery / button releaseと回復

1. Start a double-tap drag, move enough to log `arbiter owner active`, then
   release the contact.
2. Repeat while forcing a busy host connection if available; watch for
   `output report failed` or `retried button-up`.
3. Confirm the host never keeps the drag button down after release, reset, or
   the following touch.

The retry path is only fully verified when a test fixture injects a successful
button-down followed by a failed button-up enqueue. If no such fixture exists,
record this case as hardware smoke coverage, not fault-injection coverage.

retry経路を完全に検証するには、button-down成功後にbutton-up enqueueだけを失敗させる
test fixtureが必要である。fixtureが無い場合、このケースはhardware smoke coverageとして
記録し、fault-injection coverageとは扱わない。

## Case 5: work-cancellation boundary / work cancel境界

Run the following three variants while retaining the complete serial capture.

1. Make a 1F tap, then trigger sensor recovery as close as possible to the
   single-tap timeout. Repeat enough times to cover both sides of the timeout.
2. Start drag or scroll edge continuation and reset while its first delayed
   tick is due. Immediately start a fresh operation of the same type.
3. Start inertia and touch the pad again while an inertia tick is due.

Expected evidence:

- no click, continuation report, or inertia report from the pre-cancel
  operation appears after `arbiter global cancel` or the new-contact boundary
- a new operation produces output only after its own normal owner/lease log
- the firmware remains responsive; waiting for an old work handler must not
  deadlock the input stream

各variantでserial capture全体を保存する。resetまたは新規contactの境界後に旧操作の
click・continuation・inertia出力が一つでもあれば失敗である。旧work handler待機により
input streamが停止しないことも確認する。

## Case 6: queued routed-pointer frame / queue済みrouted pointer frame

1. Move a 1F pointer continuously, then trigger sensor recovery/reset while
   movement is still being reported. Repeat with an axis-locked 2F scroll.
2. Immediately begin a fresh 1F contact, then a fresh 2F scroll. Repeat the
   sequence while moving rapidly enough to leave input events queued.
3. Retain lines containing `routed output denied`, `arbiter global cancel`,
   `router cancelled`, and the first session/owner line of the fresh contact.

Expected evidence:

- no pointer or wheel movement attributable to the old operation occurs after
  the reset boundary; a queued stale report may instead log `routed output
  denied frame=...`
- the new 1F pointer and new 2F scroll work normally after their own frame
  permit is issued
- changing from 1F pointer to committed 2F scroll never leaks a 1F REL report
  into the scroll session

1F pointerまたはaxis lock済み2F scrollの出力中にresetを行い、直後に新しい1F/2F操作を
開始する。高速移動でqueueを残す条件も繰り返す。reset境界後に旧操作のpointer/wheelが
出てはならず、旧frameは必要に応じて`routed output denied frame=...`として破棄される。
新しい操作の出力が正常で、1F→2F切替時に古い1F RELが漏れないことを確認する。

## Pass criteria / 合格条件

Each case must preserve the captured log, operation time, USB/BLE state, and
whether reconnect or board reset was used. Any output from a pre-reset timer or
lease after `arbiter global cancel` is a failure. A failed button-up that is not
followed by a successful `retried button-up` before the next completed input
operation is also a failure.
