## English

# IQS9150 pointer isolated hardware test

## Scope

This UF2 enables only the IQS9150 driver and the pointer function module.
The pointer consumes raw IQS9150 frames directly: one finger produces relative
pointer movement, and two or more fingers produce scroll reports. Tap/drag,
the multi-finger router, grid/edge input, and gesture KSCAN are disabled.

## Test sequence

1. Flash `travelers-pad-pointer-isolated.uf2`, reconnect USB, and open the
   serial log. Ignore the known QSPI JEDEC warning.
2. Move one finger slowly, then quickly, along horizontal, vertical, and
   diagonal paths. Confirm that the pointer follows without a jump to a screen
   edge. Repeat after fully releasing and touching again.
3. Put down two fingers and move them together up/down, then left/right.
   Confirm vertical/horizontal scrolling and confirm that the pointer does not
   move during the scroll.
4. During a two-finger scroll, pause for one second and resume in the same
   direction five times. Confirm every restart scrolls and no restart jumps.
5. While two fingers remain down, briefly let one finger lift and re-touch.
   Confirm it does not emit pointer movement; full release ends the scroll
   session and a new one-finger touch restores pointer motion.

The first meaningful movement locks the scroll axis for the current two-finger
contact: lift both fingers before changing from vertical to horizontal or vice
versa. Slow movement is accumulated, so it may take several small reports before
the first wheel event; it must not be lost.

## Evidence

For a failure, save approximately two seconds of serial output before and after
the event, and state the test step. Relevant HID lines are `Mouse movement set`
and `Mouse scroll set`. A motion value with an unexpectedly large magnitude,
wheel reports on the locked opposite axis, or movement reports during step 3/4
is a failure. Record every `discarded implausible scroll delta` line with the
test step; it must not be emitted during ordinary scrolling.

## 日本語

# ポインタ単体実機試験

## 対象範囲

このUF2ではIQS9150ドライバとポインタ機能モジュールだけを有効にする。
ポインタはIQS9150の生フレームを直接受け、1本指は相対ポインタ移動、2本指以上は
スクロールレポートを出力する。tap/drag、多指router、格子/辺入力、ジェスチャー
KSCANは無効である。

## 試験手順

1. `travelers-pad-pointer-isolated.uf2`を書き込み、USBを再接続してシリアルログを
   開く。既知のQSPI JEDEC警告は本試験の対象外である。
2. 1本指を水平・垂直・斜めに、低速と高速で動かす。画面端へのジャンプなしに
   ポインタが追従することを確認する。完全に離してから再接触しても繰り返す。
3. 2本指を置き、上下、続いて左右に一緒に動かす。縦横スクロールが出ること、
   スクロール中にポインタが動かないことを確認する。
4. 2本指スクロール中に1秒停止して同方向へ再開する操作を5回行う。毎回スクロールが
   再開し、再開時のポインタジャンプがないことを確認する。
5. 2本指を置いたまま一方だけを短く離して再接触する。ポインタ移動を出さないことを
   確認する。両方を離すとスクロールセッションは終了し、新たな1本指接触ではポインタ
   移動に戻ることを確認する。

最初の有意な移動で、その2本指接触中のスクロール軸を固定する。縦横を切り替える前に
両指を離すこと。低速移動は蓄積するため最初のホイールイベントまで数レポートかかるが、
移動量は失われない。

## 判定ログ

失敗時は、該当操作の前後約2秒のシリアルログと手順番号を保存する。確認対象は
`Mouse movement set` と `Mouse scroll set` である。予期しない大きな移動値、固定した
反対軸のホイール出力、または手順3/4中の移動レポートは不合格とする。
`discarded implausible scroll delta` は発生したすべてについて手順番号を記録する。
通常のスクロール中には発生してはならない。
