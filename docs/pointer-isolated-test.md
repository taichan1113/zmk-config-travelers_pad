# IQS9150 pointer isolated hardware test / ポインタ単体実機試験

## Scope / 対象範囲

This UF2 enables only the IQS9150 driver and the pointer function module.
The pointer consumes raw IQS9150 frames directly: one finger produces relative
pointer movement, and two or more fingers produce scroll reports. Tap/drag,
the multi-finger router, grid/edge input, and gesture KSCAN are disabled.

このUF2ではIQS9150ドライバとポインタ機能モジュールだけを有効にする。
ポインタはIQS9150の生フレームを直接受け、1本指は相対ポインタ移動、2本指以上は
スクロールレポートを出力する。tap/drag、多指router、格子/辺入力、ジェスチャー
KSCANは無効である。

## Test sequence / 試験手順

1. Flash `travelers-pad-pointer-isolated.uf2`, reconnect USB, and open the
   serial log. Ignore the known QSPI JEDEC warning.
   `travelers-pad-pointer-isolated.uf2`を書き込み、USBを再接続してシリアルログを
   開く。既知のQSPI JEDEC警告は本試験の対象外である。
2. Move one finger slowly, then quickly, along horizontal, vertical, and
   diagonal paths. Confirm that the pointer follows without a jump to a screen
   edge. Repeat after fully releasing and touching again.
   1本指を水平・垂直・斜めに、低速と高速で動かす。画面端へのジャンプなしに
   ポインタが追従することを確認する。完全に離してから再接触しても繰り返す。
3. Put down two fingers and move them together up/down, then left/right.
   Confirm vertical/horizontal scrolling and confirm that the pointer does not
   move during the scroll.
   2本指を置き、上下、続いて左右に一緒に動かす。縦横スクロールが出ること、
   スクロール中にポインタが動かないことを確認する。
4. During a two-finger scroll, pause for one second and resume in the same
   direction five times. Confirm every restart scrolls and no restart jumps.
   2本指スクロール中に1秒停止して同方向へ再開する操作を5回行う。毎回スクロールが
   再開し、再開時のポインタジャンプがないことを確認する。
5. While two fingers remain down, briefly let one finger lift and re-touch.
   Confirm it does not emit pointer movement; full release ends the scroll
   session and a new one-finger touch restores pointer motion.
   2本指を置いたまま一方だけを短く離して再接触する。ポインタ移動を出さないことを
   確認する。両方を離すとスクロールセッションは終了し、新たな1本指接触ではポインタ
   移動に戻ることを確認する。

## Evidence / 判定ログ

For a failure, save approximately two seconds of serial output before and after
the event, and state the test step. Relevant HID lines are `Mouse movement set`
and `Mouse scroll set`. A motion value with an unexpectedly large magnitude, or
movement reports during step 3/4, is a failure.

失敗時は、該当操作の前後約2秒のシリアルログと手順番号を保存する。確認対象は
`Mouse movement set` と `Mouse scroll set` である。予期しない大きな移動値、または
手順3/4中の移動レポートは不合格とする。
