# Post-Event diagnostic startup verification

## English

The corrected Auto-Prox-off firmware passed the user's idle-resume test: cached
history shows about seven minutes without contacts followed by input while USB
was still disconnected. Runtime errors and level rescues were zero. This does
not prove LP2 residency or the exact wake latency. ALP-off remains untested.

The boot log nevertheless stayed in state 13 (final power setup readback) at
10 and 20 seconds. After enabling Event Mode the driver changed state but did
not schedule work; a silent sensor supplied no RDY interrupt until contact.

The fix schedules the final verification immediately and starts a fresh bounded
communication-window retry budget. Registers are read only when RDY is active.
No periodic runtime I2C, sensor reset, power policy or features were added or
changed. Other Settings stays 0x8088, ALP stays 0xD3FFFFFF, LP2 remains allowed.
Regression cases cover scheduling without contact/IRQ, retry budget reset,
bounded failure, and the unchanged non-diagnostic path. Hardware is pending.

UF2: `C:\Users\takec\Downloads\travelers-pad-lp2-autoprox-off-startup-fix-20261007.uf2`.
Local driver and Pad branch: `fix/post-event-diagnostic-startup`.

1. Flash, leave the pad untouched, and capture boot logs for 15 seconds. Confirm
   `final power setup verified after Event Mode enable` and `STEP1B complete`
   appear without touching. Record elapsed boot time. State 13 must not persist.
2. Disconnect USB and check pointer/taps/scroll. Record first-response latency.
3. Leave untouched five minutes, then test movement for 30 seconds, taps and
   two-finger scroll before reconnecting USB or pressing RESET. Record latency
   separately from the initial boot time.
4. Reconnect USB and submit the full history plus boot log. If input stops, do
   not reset before collecting history. The final readback must show Config
   0x471E, Other 0x8088 and ALP 0xD3FFFFFF. Quiet Event Mode mode counters cannot
   prove the current mode.

## 日本語

Auto-Prox停止修正版の放置試験は実機合格。history上、約7分の無接触後にUSB未接続の
まま入力が再開し、通信エラー・レベル救済は0でした。ただしLP2滞在や正確な復帰時間の
証明ではありません。ALP無効版は未検証です。

起動ログでは10秒・20秒時点でもstate=13（最終設定確認待ち）でした。
Event Mode有効化後に状態だけを変更し、workを予約していなかったため、無通信になると
次の接触によるRDY割り込みまで確認が進まない経路がありました。

修正では最終確認を直ちに予約し、有限回の通信窓要求カウンタを新しく開始します。
レジスタ読み取りはRDY有効時のみ。通常稼働中のI2Cポーリング・センサーRESETの追加は
ありません。省電力設定・機能は変更せず、Other=0x8088、ALP=0xD3FFFFFF、LP2許可を維持。
接触・割り込みなしの予約、再試行カウンタ初期化、有限回の失敗、診断無効時の不変を
回帰テストで確認します。今回の修正版の実機確認は未完了です。

UF2：`C:\Users\takec\Downloads\travelers-pad-lp2-autoprox-off-startup-fix-20261007.uf2`。
driver/Padのローカルブランチ：`fix/post-event-diagnostic-startup`。

1. 書き込み後、パッドに触れず15秒間起動ログ取得。
   `final power setup verified after Event Mode enable` と `STEP1B complete` が
   無接触で出ることを確認。起動からの秒数も記録。state=13が継続しないこと。
2. USBを外し、ポインタ・タップ・スクロール確認。最初の反応までの時間を記録。
3. 5分無接触後、USB/RESETを使わず移動30秒・タップ・2本指スクロール確認。
   復帰時間を起動時間と分けて記録。
4. USBを戻し、history全文と起動ログを提出。停止時は採取前にRESETしない。
   最終読み戻しでConfig=0x471E、Other=0x8088、ALP=0xD3FFFFFFを確認。
   Event Mode無通信中のmodeカウンタは現在のmodeの証明にはなりません。
