# Terminal click spacing test

[English](#english) | [日本語](#日本語)

## English

Both config and features use `test/terminal-click-spacing`. The driver is unchanged.
The released west revisions are intentionally unchanged; this test uses a local
features checkout override. Running `west update` does not select this test code.

Set `CONFIG_TRAVELERS_PAD_CLICK_PRESS_MS=5` and
`CONFIG_TRAVELERS_PAD_CLICK_GAP_MS=5` in the shield configuration. Each accepts
5–100 ms. Build with `bash scripts/build-click-spacing.sh [features-checkout]`.
Run `bash scripts/test-click-spacing.sh [features-checkout]` for host regression.

Terminal tap clicks are serialized asynchronously in a bounded eight-action
queue. A double-click emits down, 5 ms, up, 5 ms, down, 5 ms, up. These are minimum
input enqueue intervals, not guaranteed BLE delivery intervals. Workqueue/input
congestion can lengthen them. No input-thread sleeps are introduced.
Live drag presses and motion retain the existing path. A same-button drag cancels
pending tap pulses and releases their exact ledger entry before taking over;
this safety cancellation may shorten a pulse. Sensor cancellation drops queued
clicks and releases held buttons. Failed releases retain the existing retry ledger.
Accepted terminal grants are consumed once; delayed transactions do not replay
arbiter grants. This test does not add a BLE disconnect hook; pending transactions
finish locally as before, so verify disconnect/reconnect behavior on hardware.

### Hardware check

Use the same BLE host, application, and pointer speed as the failing baseline.
Do not re-pair unless connection itself fails; doing so changes another variable.

1. Perform 20 stationary three-finger taps on a browser link. Confirm middle-click
   opens one background tab per recognized tap (subject to browser settings).
2. On Ubuntu, perform 20 one-finger double taps over a word and 20 over a normal
   application's titlebar. Confirm word selection and maximize/restore.
3. Check one- and two-finger clicks, and double-tap movement dragging for all three
   button mappings. Confirm no premature double click or stuck/released drag button.
4. Check ordinary pointer, two-finger scroll/inertia, and power-cycle/reconnect.

Capture tap/drag recognition logs plus `terminal click queued` and `paced click`
lines. If recognition logs are absent, pacing is not the cause. A queued click
should show alternating values 1/0, with at least 5 ms between normal pulse events.
These logs show successful input enqueue, not successful host receipt. If needed,
compare 10 or 20 ms for both settings after the 5 ms trial, without other changes.
Hardware acceptance is pending.

## 日本語

configとfeaturesはともに `test/terminal-click-spacing` ブランチです。
ドライバは変更していません。westのリリースrevisionは意図的に維持し、試験では
ローカルfeatures checkoutを優先指定します。`west update` だけでは試験コードに
切り替わりません。

shield設定の `CONFIG_TRAVELERS_PAD_CLICK_PRESS_MS=5` と
`CONFIG_TRAVELERS_PAD_CLICK_GAP_MS=5` が、それぞれ押下時間と解放間隔です。
どちらも5～100msで設定できます。
`bash scripts/build-click-spacing.sh [features-checkout]` でビルドし、
`bash scripts/test-click-spacing.sh [features-checkout]` でホスト回帰試験を実行します。

タップclickは最大8件のキューで非同期に順番に送ります。ダブルクリックは
押下→5ms→解放→5ms→押下→5ms→解放です。これはinputへの投入間隔の下限であり、
BLEでホストに届く間隔の保証ではありません。混雑すると間隔は延びます。
inputスレッドでsleepする実装は追加していません。
ドラッグの押下・移動経路は維持します。同じボタンのドラッグが始まる場合は、
残りのタップclickを取り消し、正確な台帳項目を解放してから引き継ぎます。
この安全な取消しではclickの押下時間が短くなる場合があります。
センサー取消し時は未送信clickを破棄し、押下済みボタンを解放します。
解放送信に失敗した場合は既存の再試行台帳を維持します。
terminal grantは受付時に一度だけ消費し、遅延処理で再利用しません。
BLE切断専用hookは追加していません。未完了処理は従来同様ローカルで完了するため、
切断・再接続時の動作も実機で確認してください。

### 実機確認

不具合が起きたときと同じBLE接続先・アプリ・ポインタ速度で確認してください。
接続自体に問題がない限り再ペアリングせず、比較条件を揃えます。

1. ブラウザのリンク上で、静止した3本指タップを20回。
   認識されたタップごとに中クリックで背景タブが1つ開くか確認します。
   ブラウザ設定に依存する場合があります。
2. Ubuntuで単語上の1本指ダブルタップと、通常アプリのタイトルバー上の
   ダブルタップを各20回。単語選択と最大化・復元を確認します。
3. 1・2本指clickと、1・2・3本指のダブルタップからのドラッグを確認。
   意図しないダブルクリック、押しっぱなし、途中解放がないことを確認します。
4. 通常ポインタ、2本指スクロール・慣性、電源OFF→ONからの再接続を確認します。

認識ログと `terminal click queued`、`paced click` を提出してください。
認識ログが無い場合は、送信間隔とは別の問題です。通常のclickではvalue=1/0が
交互になり、送信投入時刻の差が最低5msになります。ログはinputへの投入成功を
示すもので、ホスト受信成功を保証しません。必要なら5msの結果を確認後、
他条件を変えず両設定を10msまたは20msで比較します。実機合格は未判定です。
