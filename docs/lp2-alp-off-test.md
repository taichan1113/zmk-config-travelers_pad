# LP2 / ALP-off comparison and Auto-Prox correction

## English

Branch: driver/pad `debug/lp2-alp-off-startup-fixed`. Features unchanged. LP2 remains allowed.
Primary UF2: `C:\Users\takec\Downloads\travelers-pad-lp2-alp-off-startup-fix-20261007.uf2`.
Fallback: accepted `travelers-pad-sensor-idle-only-20261007.uf2`.

Hardware result: two battery/BLE idle-resume trials passed (about 5m27s and
6m23s); first contact was one finger before USB reconnect; errors/rescues zero.
Production-candidate acceptance is separate: see
[production regression](alp-off-production-regression.md).

Latest evidence: the contact-independent final verification fix completed at
2.708s on hardware. However, Auto-Prox-off alone failed idle resume: the first
post-idle sensor contact was four fingers, after unsuccessful one-finger input.
Do not treat Auto-Prox-off as a permanent fix. This ALP-off build retains that
startup scheduling fix and restores baseline Auto-Prox. Diagnostic hardware acceptance passed.

**Correction:** the prior Auto-Prox-off firmware verified 0x8088 before ATI but
then wrote four bytes at 0x11BE when enabling Event Mode, restoring Other Settings
to 0x80A8. Its failed hardware run is not evidence that disabling Auto-Prox fails.
Event Mode now writes only the two Config Settings bytes. Diagnostic power setup
is also read back after Event Mode before entering RUNNING. Tests cover reversion
and reject it. No runtime polling/wake/reset is added.

The ALP-off primary clears only ALP Setup bit31 (0x11C5 bit7): 0xD3FFFFFF becomes
0x53FFFFFF. LP1/LP2 use trackpad sensing per datasheet A.15. Auto-Prox is baseline
(Other Settings=0x80A8, LP1 off/LP2 on). All timeouts, sampling, electrodes,
thresholds, filters, automatic reference management and BLE/feature settings
are unchanged. Before ATI and after Event Mode, verify full ALP Setup and original
Other Settings. A mismatch stops startup. Cached passive observations still do
not prove current mode during Event Mode silence.

1. Flash **the ALP-off UF2** and capture boot setup logs. Confirm alp=0x53ffffff,
   alp-enabled=0, other=0x80a8, lp2-autoprox=1, timeouts=10/60/5/40 and the final
   `final power setup verified after Event Mode enable` line.
2. Disconnect USB and check pointer/taps/scroll; note first-response delay and
   how long initial stutter lasts. Stop operating once basic input is stable.
3. Leave untouched 5 minutes, then try one-finger pointer for 30s, taps and
   scroll before USB/RESET. If one finger fails, record that failure before
   testing four fingers and whether that restores subsequent one-finger input.
4. Reconnect USB and capture full history with sensor rows. State whether USB alone
   restored input. Do not reset first if it stops.

An optional corrected Auto-Prox-off UF2 is also supplied:
`travelers-pad-lp2-autoprox-off-verified-20261007.uf2`. This keeps ALP=0xD3FFFFFF
and checks Other Settings=0x8088 again **after** Event Mode. It uses the same code
with a different overlay. Test it separately using the same steps if needed;
do not confuse it with the old unverified Auto-Prox-off artifact. Success/failure
of either comparison needs correct final setup readback before interpretation.

Reference: [Azoteq datasheet](https://www.azoteq.com/images/stories/pdf/IQS9150_IQS9151_datasheet.pdf), Appendix A.15.

## 日本語

driver/pad：`debug/lp2-alp-off-startup-fixed`。機能モジュールは変更しません。LP2は許可します。
今回試すUF2：`C:\Users\takec\Downloads\travelers-pad-lp2-alp-off-startup-fix-20261007.uf2`。
戻し先：合格済み `travelers-pad-sensor-idle-only-20261007.uf2`。

実機結果：バッテリーBLEで約5分27秒・6分23秒放置後、USB再接続前に1本指で復帰。
2回合格、エラー・救済0。本番候補の合否は別途
[最終回帰確認](alp-off-production-regression.md)で確認します。

最新結果：無接触での最終設定確認は実機2.708秒で完了し、診断待ち修正は合格。
一方、Auto-Prox停止だけでは放置後の1本指入力に反応せず、最初の生接触は4本指でした。
Auto-Prox停止は恒久対策として合格扱いにしません。今回のALP無効版は起動時の予約修正を
維持し、Auto-Proxを基準値へ戻します。この診断版の実機確認は合格です。

**前回試験の訂正：** Auto-Prox無効版はATI前に0x8088を確認したものの、Event Modeを
有効化するときに0x11BEから4バイト書き、Other Settingsを0x80A8へ戻していました。
前回停止した結果は、Auto-Proxを無効化しても失敗する証拠にはできません。
今回はEvent ModeでConfig Settingsの2バイトだけを書き、診断設定をEvent Mode有効化後にも
読み戻してからRUNNINGに進みます。巻き戻りを拒否する回帰テストも追加しました。
runtimeポーリング・強制復帰・定期RESETは追加しません。

今回のALP無効版は0x11C5 bit7だけを0にし、ALP Setupを0xD3FFFFFF→0x53FFFFFFにします。
公式A.15に従いLP1/LP2のsensingをtrackpadに切り替えます。
Auto-Proxは基準値（Other=0x80A8、LP1無効/LP2有効）へ戻します。
timeout、sampling、電極選択、閾値、filter、自動reference、BLE、既存機能は維持。
ATI前とEvent Mode有効化後にALP Setup全word・元のOther Settingsを確認し、不一致で停止。
受動診断の採取値は無通信中に古くなるため、現在modeの証明にはなりません。

1. **ALP無効版UF2**を書き込み、起動ログ保存。alp=0x53ffffff、alp-enabled=0、
   other=0x80a8、lp2-autoprox=1、timeout=10/60/5/40、および最後の
   `final power setup verified after Event Mode enable` を確認。
2. USBを外して基本操作確認。初動の待ち時間とカクツキが落ち着くまでの時間を記録。
   安定したら操作をやめる。
3. 5分無接触後、USB/RESET前に1本指移動30秒・タップ・スクロール確認。
   1本指が反応しない場合は失敗として記録し、その後4本指で復帰するか、続いて1本指が
   動くかを確認。4本指で復帰しても合格にはしません。
4. USBを戻してsensor行を含むhistory全文を保存。USBだけで復帰したかも共有。
   停止しても先にRESETしないでください。

前回試験用の修正版も別名で用意：`travelers-pad-lp2-autoprox-off-verified-20261007.uf2`。
こちらはALP=0xD3FFFFFF、Other=0x8088を**Event Mode後にも**確認します。
同じコードでoverlayだけ異なります。必要なら同じ手順で別試験に使用してください。
旧Auto-Prox無効版とは区別し、各試験の最終readbackを確認してから結果を判断します。

根拠：[Azoteq公式データシート](https://www.azoteq.com/images/stories/pdf/IQS9150_IQS9151_datasheet.pdf) 付録A.15。
