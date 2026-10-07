# Gesture key spacing and Tip-Tap diagnostics

[English](#english) | [日本語](#日本語)

## English

This candidate extends the multi-finger tap test with 50 ms discrete virtual-key
presses, controlled by `CONFIG_TRAVELERS_PAD_GESTURE_PRESS_MS=50`. Multi-finger
swipes/Grasp/Release/management hold, Tip-Tap and pinch/rotation use a serialized,
bounded 16-action transport queue. Release occurs at least 50 ms after the press
callback finishes; repeated actions are separated by at least 5 ms button-up.
No input-thread sleeps are added. Accepted actions are authorized before enqueue,
not treated as new delayed recognizer claims. Sensor cancellation drops pending
actions and releases a pressed action. A full queue reports an error rather than
growing indefinitely. Callback duration, workqueue scheduling and host processing
can lengthen the timing. BLE arrival times are not guaranteed by these intervals.

Edge dial retains its existing keyboard-owned behavior/macro pacing: its scroll
macros already provide their own hold duration, and lengthening them changes the
repeat rate. Mouse tap clicks remain 50 ms down / 5 ms up. Live drag duration,
pointer release protection, driver power policy and released west revisions are
unchanged. Four-finger hold recognition remains 1 second; only its accepted
virtual-key action is paced.

The submitted Tip-Tap log contains seven recognized Right actions, with virtual
key press/release processing. This proves recognition and dispatch, not host-side
success. Session 69 ends with peak=1, so the normalized input never reported a
second contact in that session. Sessions 65/68 instead end as two-finger fallback
taps rejected at 516/611 ms; these fallback logs alone do not identify why Tip-Tap
failed. New `Tip-Tap ignored` logs distinguish anchor age, changed contacts,
movement over slop, second-contact duration over 180 ms, horizontal separation
under 160, and competing ownership. Recognition thresholds remain unchanged.
The router's `route=swipe candidate` log is not proof of an emitted swipe key.

### BLE-only hardware check

Flash `travelers-pad-gesture-keys50-tiptap-diagnostics-20261008.uf2` from Windows
Downloads. Keep input on BLE; USB is power/serial logging only.

1. In a browser, hold an anchor still for about 0.5 seconds, then briefly tap a
   second finger well to its left/right. Keep the anchor and repeat 10 times.
   Check tab switching and capture both successful and failed attempts.
2. Check three-/four-finger swipe keys, five-finger actions, management enter/exit,
   and pinch keys. Logs should show paired `paced gesture ... pressed=1/0` lines
   at least 50 ms apart during normal operation. Reset cancellation is exempt.
3. Check ordinary two-/three-finger taps do not become Tip-Tap/swipe, and pointer,
   drag, scroll/inertia and edge operations remain unchanged.

Submit logs around each failure. A `Tip-Tap Right/Left` followed by `paced gesture`
and ZMK key processing means recognition succeeded; missing host action points to
transport/application handling instead. An explicit `Tip-Tap ignored` identifies
a recognition condition. If neither appears, the sensor may not have reported the
necessary contact transition; the absence alone does not prove a driver fault.
Host regressions and firmware build are complete; BLE hardware acceptance remains
pending. No main merge or push is performed.

## 日本語

この試験版は複数指タップ対策に、単発ジェスチャー仮想キーの50ms押下を追加します。
`CONFIG_TRAVELERS_PAD_GESTURE_PRESS_MS=50` で調整できます。
多指スワイプ・Grasp/Release・管理ホールド、Tip-Tap、ピンチ・回転は最大16件の
順序付き非同期送信キューを使います。押下callback終了から最低50ms後に解放し、
反復action間の解放は最低5msです。inputスレッドにsleepは追加していません。
受付時に出力を認可し、遅延処理で新しい認識claimは作りません。
センサー取消しでは未送信actionを破棄し、押下済みactionは解放します。
キュー満杯時はエラーを報告し、無制限には滞留させません。
callback処理・workqueue・ホスト処理で間隔が延びる場合があります。
これらの間隔はBLE到達時刻の保証ではありません。

エッジは従来のキーボード側behavior/macro経路を維持します。スクロールmacroは
独自の保持時間を持ち、追加の50ms保持は反復レートを変えてしまうためです。
マウスタップは押下50ms・解放5msのままです。ドラッグ保持時間、ポインタ解放保護、
ドライバ省電力方針、westのリリースrevisionは変更していません。
4本指ホールドの認識は1秒のままで、確定した仮想キーactionの押下だけを調整します。

提出されたTip-TapログにはRight認識が7回あり、仮想キーの押下・解放処理もあります。
これは認識・dispatchの成功であり、ホスト側の動作成功を保証するものではありません。
session 69はpeak=1で終わり、そのsessionでは正規化入力に2本目が報告されていません。
session 65/68は2本指fallbackタップとして516/611msで棄却されていますが、このログだけ
ではTip-Tap不成立の条件を特定できません。
追加の `Tip-Tap ignored` は、anchor時間不足、接触変更、slop超過、2本目の接触時間
180ms超過、左右間隔160未満、所有権競合を区別します。認識閾値は維持しています。
routerの `route=swipe candidate` は、実際のスワイプキー発火を示すログではありません。

### 必ずBLEで実機確認

Windows Downloadsの `travelers-pad-gesture-keys50-tiptap-diagnostics-20261008.uf2`
を書き込みます。入力はBLE、USBは給電・シリアルログのみです。

1. ブラウザでanchorを約0.5秒静止し、十分左右に離した位置へ2本目を短くタップ。
   anchorを維持して10回反復し、タブ切替と成功・不成立の両方のログを確認。
2. 3・4本指スワイプ、5本指操作、管理モード出入り、ピンチのキーを確認。
   通常時は `paced gesture ... pressed=1/0` が最低50msの差で対になること。
   resetによる安全な取消しは例外です。
3. 通常の2・3本指タップがTip-Tap／スワイプに化けず、ポインタ・ドラッグ・
   スクロール・慣性・エッジ操作が維持されていること。

不成立の前後ログを提出してください。`Tip-Tap Right/Left` の後に `paced gesture` と
ZMKキー処理があれば認識は成功し、ホスト動作が無い場合は送信・アプリ側を疑います。
`Tip-Tap ignored` があれば具体的な認識条件を確認できます。
どちらも無い場合は必要な指本数遷移が報告されていない可能性もありますが、
それだけでドライバ不具合とは断定できません。
ホスト回帰試験・ビルドは完了し、BLE実機合格は未判定です。main統合・pushはしません。
