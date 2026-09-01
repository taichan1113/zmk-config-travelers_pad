# IQS9150 grid matrix isolated hardware test / 格子matrix単体実機試験

## Scope / 対象範囲

This UF2 enables only the private one-finger IQS9150 input stream and
`zmk-input-matrix`. Pointer, mouse tap/drag, scroll, and multi-finger gesture
output are disabled. A contact can therefore produce only a virtual grid key.

このUF2は、専用の1本指IQS9150入力ストリームと `zmk-input-matrix` だけを有効にする。
ポインタ、マウスタップ/ドラッグ、スクロール、多指ジェスチャー出力は無効である。従って
接触による出力は仮想格子キーだけとなる。

## Test bindings / 試験用割当

Open Notepad. The 3x3 region is numbered left-to-right, top-to-bottom.
メモ帳を開く。3x3領域は左上から右下へ1--9の順である。

| Input / 入力 | Expected characters / 期待文字 |
| --- | --- |
| Tap / タップ | `1`--`9` |
| Flick up / 上フリック | `Q`--`O` |
| Flick down / 下フリック | `A`--`L` |
| Flick left / 左フリック | `Z`--`.` |
| Flick right / 右フリック | `F1`--`F9` (verify with a key-event viewer) |

## Test sequence / 試験手順

1. Flash `travelers-pad-grid-matrix-isolated.uf2`, reconnect USB, and wait for
   `STEP1B complete: Event Mode active`.
   `travelers-pad-grid-matrix-isolated.uf2`を書き込み、USB再接続後、
   `STEP1B complete: Event Mode active` を待つ。
2. Tap the centre and four corners. Confirm `5`, then `1`, `3`, `7`, `9` in
   Notepad. No pointer movement or mouse click may occur.
   中央と四隅をタップし、メモ帳に順に `5`、`1`、`3`、`7`、`9` が入力されることを
   確認する。ポインタ移動やマウスクリックは発生してはならない。
3. From the centre cell, make a deliberate up, down, left, then right flick.
   Confirm `T`, `G`, `B`, then `F5` (the latter in a key-event viewer).
   中央セルから、上、下、左、右へ明確にフリックする。順に `T`、`G`、`B`、`F5`
   （最後はキーイベントビューア）を確認する。
4. Repeat the five centre operations ten times. Confirm that every touch
   produces exactly one key event, with no mouse HID output.
   中央で上記5操作を10回繰り返す。各接触がキーイベント一回だけを出し、マウスHID出力が
   無いことを確認する。

## Expected log / 期待ログ

`kscan_matrix: Reporting KSCAN event: Row … Column … Pressed 1`, followed by
the matching release, appears once per operation. No non-zero `Mouse movement
set`, `Mouse scroll set`, or `Mouse buttons set` line may appear.

各操作につき `kscan_matrix: Reporting KSCAN event: Row … Column … Pressed 1` と
対応する解放が一回出る。非ゼロの `Mouse movement set`、`Mouse scroll set`、
`Mouse buttons set` は出てはならない。
