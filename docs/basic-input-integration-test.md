# IQS9150 basic input integration hardware test / 基本入力結合実機試験

## Scope / 対象範囲

This UF2 enables `IQS9150 → session → contact arbiter → router/pointer` and
the session-frame `tap_drag` function with its HID listener. It validates the
single ownership path between pointer movement, two-finger scrolling,
one/two/three-finger taps, and double-tap-hold drag. Grid/edge input and
gesture KSCAN are disabled.

このUF2は `IQS9150 → session → contact arbiter → router/pointer` と、session frameを
監視する `tap_drag` 機能およびHID listenerを有効にする。単一所有権経路におけるポインタ移動、
2本指スクロール、1/2/3本指タップ、ダブルタップ＋ホールドドラッグ間の所有権を検証する。
格子/辺入力とジェスチャーKSCANは無効である。

## Test sequence / 試験手順

1. Flash `travelers-pad-basic-input-integration.uf2`, reconnect USB, and wait
   for `STEP1B complete: Event Mode active` in the serial log.
   `travelers-pad-basic-input-integration.uf2`を書き込み、USB再接続後、シリアルログの
   `STEP1B complete: Event Mode active` を待つ。
2. Move one finger, then make a stationary one-finger tap. Confirm ordinary
   pointer movement and exactly one left click, with no scroll.
   1本指を動かした後、静止した1本指タップを行う。通常のポインタ移動と左クリック1回、
   スクロールなしを確認する。
3. Make five stationary two-finger taps and five stationary three-finger taps.
   Confirm right clicks and middle clicks respectively. No pointer movement or
   scroll may be emitted.
   静止した2本指タップを5回、3本指タップを5回行う。順に右クリック、中クリックとなり、
   ポインタ移動・スクロールが出ないことを確認する。
4. Scroll vertically and horizontally with two fingers. Confirm scrolling only:
   no right click at release and no pointer movement. Lift both fingers before
   changing axis.
   2本指で縦・横スクロールを行う。リリース時の右クリックとポインタ移動がなく、
   スクロールだけが出ることを確認する。軸を変える前に両指を離す。
5. Make a one-finger double tap, hold the second contact, and move. Confirm the
   left button is held before the first meaningful drag delta; on release it is
   released exactly once. Repeat with two fingers (right-button drag) and three
   fingers (middle-button drag).
   1本指ダブルタップの2回目をホールドして動かす。最初の有意なドラッグ差分より先に
   左ボタンが押下され、リリース時に一度だけ解放されることを確認する。2本指（右）と
   3本指（中）でも繰り返す。
6. Start a one-finger move and immediately add a second finger, then perform a
   stationary two-finger tap. Confirm it is a right click, not a continued
   one-finger pointer move or left-button drag.
   1本指移動を開始して直ちに2本目を加え、静止した2本指タップを行う。継続する1本指
   ポインタ移動や左ドラッグではなく、右クリックになることを確認する。

## Evidence / 判定ログ

For a failure, save approximately two seconds of serial output before and after
the operation. Include `route=`, `tap released` / `tap rejected`,
`double-tap drag`, `Mouse buttons set`, `Mouse movement set`, and `Mouse scroll
set` lines.

失敗時は操作前後およそ2秒のシリアルログを保存する。`route=`、`tap released` /
`tap rejected`、`double-tap drag`、`Mouse buttons set`、`Mouse movement set`、
`Mouse scroll set` を含める。
