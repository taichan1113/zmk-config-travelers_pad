## English

# IQS9150 multi-finger gesture integration hardware test

## Scope

This UF2 enables the basic pointer, scroll, tap/drag path and the composite
KSCAN path for 3--5-finger gestures. The `zmk-input-matrix` grid and edge
sources remain disabled.

## Active bindings

| Gesture| Binding|
| --- | --- |
| 3F left / right / up / down | Browser Back / Forward / Page Up / Page Down |
| 4F left / right / up / down | `Win+Left` / `Win+Right` / `Win+Up` / `Win+Down` |
| 5F Grasp / Release | `Win+M` / `Win+Shift+M` |

`Win+Shift+M` restores windows minimized by `Win+M`; Windows has no standard

## Test sequence

1. Flash `travelers-pad-multifinger-gesture-integration.uf2`, reconnect USB,
   and wait for `STEP1B complete: Event Mode active`.
2. In a browser, perform a 3F left and right swipe. Confirm Back and Forward.
   Then perform 3F up/down swipes and confirm Page Up/Page Down.
3. Perform all four 4F swipe directions with a resizable foreground window.
   Confirm Windows Snap Left/Right and maximize/restore behavior for Up/Down.
4. Perform 5F Grasp, then 5F Release. Confirm all windows minimize, then the
   windows minimized by the preceding shortcut are restored.
5. For each gesture, verify there is no pointer movement, scrolling, or
   mouse-click output. Also verify a stationary 3F tap is still a middle click
   and a 3F double-tap-hold remains a middle-button drag.

## Expected log

- 3F / 4F: `multifinger claimed`, then `swipe candidate direction=` and one
  virtual KSCAN press/release.
- 5F: `5F Grasp gesture complete` or `5F Release gesture complete` and one
  virtual KSCAN press/release.
- No `Mouse movement set`, non-zero `Mouse scroll set`, or `Mouse buttons set`
  lines belong to the gesture interval.

## 日本語

# 多指ジェスチャー結合実機試験

## 対象範囲

このUF2は基本のポインタ、スクロール、タップ/ドラッグ経路に加え、3--5本指
ジェスチャー用composite KSCANを有効にする。`zmk-input-matrix` の格子・辺入力は
無効のままとする。

## 有効な割当

|操作 |割当 |
| --- | --- |
| 3F left / right / up / down | Browser Back / Forward / Page Up / Page Down |
| 4F left / right / up / down | `Win+Left` / `Win+Right` / `Win+Up` / `Win+Down` |
| 5F Grasp / Release | `Win+M` / `Win+Shift+M` |

shortcut that maximizes every window. `Win+Shift+M` は `Win+M` で最小化した
ウィンドウを復元する。全ウィンドウを最大化する標準Windowsショートカットはない。

## 試験手順

1. `travelers-pad-multifinger-gesture-integration.uf2`を書き込み、USB再接続後、
   `STEP1B complete: Event Mode active` を待つ。
2. ブラウザで3本指の左/右スワイプを行い、戻る/進むを確認する。続いて3本指の
   上/下スワイプでPage Up/Page Downを確認する。
3. サイズ変更可能な前面ウィンドウで4本指の全方向スワイプを行う。左右のスナップ、
   上下の最大化/復元動作を確認する。
4. 5本指Grasp、続けて5本指Releaseを行い、全ウィンドウの最小化と、その直前に
   最小化されたウィンドウの復元を確認する。
5. 各ジェスチャーで、ポインタ移動、スクロール、マウスクリックが混入しないことを
   確認する。さらに静止した3本指タップが中クリックのまま、3本指
   ダブルタップ＋ホールドが中ボタンドラッグのままであることを確認する。

## 期待ログ

- 3本/4本指: `multifinger claimed`、次に `swipe candidate direction=`、仮想KSCANの
  一回の押下/解放。
- 5本指: `5F Grasp gesture complete` または `5F Release gesture complete`、仮想KSCANの
  一回の押下/解放。
- ジェスチャー区間に `Mouse movement set`、非ゼロの `Mouse scroll set`、
  `Mouse buttons set` が出ない。
