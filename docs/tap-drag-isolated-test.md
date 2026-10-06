## English

# Tap/Drag Isolated Hardware Test

This branch intentionally enables only the IQS9150 driver, the tap/drag
function, and its HID input listener. Pointer movement, two-finger scroll,
the multi-finger router, grid/edge input, and three-to-five-finger gesture
KSCAN are disabled.

## Expected boot evidence

After `STEP1B complete`, the serial log must contain:

`tap/drag active: tap=180ms move=75 settle=120ms double-gap=250ms`

## Test sequence

1. Do five one-finger stationary taps. Each must log `1F tap released` followed
   250 ms later by `1F single tap confirmed` and send a left click.
2. Do ten two-finger stationary taps: five firm taps, then five light taps.
   Each must log `2F tap released` followed by `2F single tap confirmed` and
   send a right click.
3. Repeat five two-finger taps after moving with one finger and immediately
   placing the second finger. This checks the 1F-to-2F transition.
4. Do five three-finger stationary taps. Each must send a middle click.
5. Do a one-finger double-tap, hold, and move. Verify left-button drag starts
   without a large cursor jump.

## Failure record

Copy the serial segment from the preceding `2F frame` (or `1F` contact) through
the next `tap released`, `tap rejected`, or click line. `tap rejected` records
whether duration or movement caused the rejection.

## 日本語

# タップ・ドラッグ単体実機試験

このブランチでは、IQS9150ドライバ、タップ/ドラッグ機能、HID input listenerだけを
有効にする。ポインタ移動、2本指スクロール、多指ルーター、格子/辺入力、3〜5本指
ジェスチャーKSCANは無効である。

## 起動時の確認

`STEP1B complete` の後に、上記の `tap/drag active` 行が出ることを確認する。

## 試験手順

1. 1本指の静止タップを5回行う。各回で `1F tap released` の250 ms後に
1. `1F single tap confirmed` が出て、左クリックになること。
2. 2本指の静止タップを10回行う。前半5回は通常の強さ、後半5回は軽く触る。
2. 各回で `2F tap released` の後に `2F single tap confirmed` が出て、右クリックに
   なること。
3. 1本指でポインタを移動した直後に2本指タップを5回行う。1F→2F遷移を確認する。
4. 3本指の静止タップを5回行う。各回で中クリックになること。
5. 1本指ダブルタップ→ホールド→移動を行う。画面端へ飛ばず、左ボタンのドラッグが
5. 開始すること。

## 失敗時の記録

失敗時は、直前の `2F frame`（または1本指接触）から、次の `tap released`、
`tap rejected`、クリック行までをコピーする。`tap rejected` は時間または移動のどちらで
棄却されたかを記録する。
