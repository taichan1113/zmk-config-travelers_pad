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

```
  1 | 2 | 3
 ---+---+---
  4 | 5 | 6
 ---+---+---
  7 | 8 | 9
```

For a tap, put a finger down, keep it still for about 0.15--0.25 seconds, then
lift it. Do not slide while touching. The test waits 80 ms before accepting a
grid contact and uses a 200-count flick threshold. For a flick, start at the
centre, pause briefly after touching, and travel straight by roughly one
quarter of the pad before lifting.

タップは、指を置いてから約0.15〜0.25秒静止し、そのまま離す。接触中に滑らせない。
この試験では、格子接触の確定を80ms待ち、フリックしきい値を200カウントとしている。
フリックは中央から始め、接触後に少し静止してから、パッド幅または高さのおよそ1/4を
まっすぐ動かして離す。

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
   Notepad. The matching log rows are `Row 1, Column 1`, then `(0,0)`,
   `(0,2)`, `(2,0)`, `(2,2)`. No pointer movement or mouse click may occur.
   中央と四隅をタップし、メモ帳に順に `5`、`1`、`3`、`7`、`9` が入力されることを
   確認する。ポインタ移動やマウスクリックは発生してはならない。
3. From the centre cell, make a deliberate up, down, left, then right flick.
   Confirm `T`, `G`, `B`, then `F5` (the latter in a key-event viewer). The
   matching log rows are `(4,1)`, `(7,1)`, `(10,1)`, `(13,1)`.
   中央セルから、上、下、左、右へ明確にフリックする。順に `T`、`G`、`B`、`F5`
   （最後はキーイベントビューア）を確認する。
4. Repeat the five centre operations ten times. Confirm that every touch
   produces exactly one key event, with no mouse HID output.
   中央で上記5操作を10回繰り返す。各接触がキーイベント一回だけを出し、マウスHID出力が
   無いことを確認する。

## Expected log / 期待ログ

`kscan_matrix: Reporting KSCAN event: Row … Column … Pressed 1`, followed by
the matching release, appears once per operation. Rows 0--2 are taps, 3--5
are up flicks, 6--8 are down flicks, 9--11 are left flicks, and 12--14 are
right flicks. No non-zero `Mouse movement set`, `Mouse scroll set`, or `Mouse
buttons set` line may appear.

各操作につき `kscan_matrix: Reporting KSCAN event: Row … Column … Pressed 1` と
対応する解放が一回出る。行0〜2がタップ、3〜5が上、6〜8が下、9〜11が左、12〜14が右
フリックである。非ゼロの `Mouse movement set`、`Mouse scroll set`、`Mouse buttons set`
は出てはならない。
