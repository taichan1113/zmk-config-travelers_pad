## English

# Pinch/rotation integration test

This is the single hardware pass for the transform feature. It verifies all
four directional actions, repeated steps, ownership against scroll/tap, and
normal input recovery with one USB-logging UF2.

## Firmware and log capture

Use the UF2 built from `feat/pinch-rotate`:

```text
build/iqs9150_xiao/zephyr/zmk.uf2
```

1. Flash the UF2 and reconnect the application USB device.
2. Open its CDC serial port at 115200 baud and save the complete session.
   On Linux, first locate it with `ls -l /dev/serial/by-id/`; for example:

   ```sh
   tio -b 115200 /dev/serial/by-id/<device> | tee pinch-rotate.log
   ```

3. Open a browser page where `Ctrl+-` and `Ctrl+=` change zoom. Open a keyboard
   event tester as well if host confirmation of F15/F16 is required.

The default keymap is:

| Gesture| Log action| Host binding|
|---|---|---|
| Pinch in| `PINCH_IN` | `Ctrl+-` (zoom out) |
| Pinch out| `PINCH_OUT` | `Ctrl+=` (zoom in) |
| Counter-clockwise| `ROTATE_COUNTER_CLOCKWISE` | `F15` |
| Clockwise| `ROTATE_CLOCKWISE` | `F16` |

## One-pass procedure

Keep the same firmware and serial capture for every case.

| Order | Operation| Required result|
|---:|---|---|
| 1 | Move one finger, then make one stationary two-finger tap| Pointer and right-click remain normal. No transform action appears. |
| 2 | Move both fingers in parallel| Log shows `route=scroll proposed move=(...,...) dot=...` with a positive dot value, followed by `winner=scroll`; scrolling occurs and no transform action appears. |
| 3 | Pinch inward far enough to cross several steps| One `transform=PINCH proposed`, `winner=transform`, and one or more `action=PINCH_IN`; browser zoom decreases. No wheel or right-click occurs. |
| 4 | Start a new contact and pinch outward across several steps| `action=PINCH_OUT` appears one or more times; browser zoom increases. |
| 5 | Start a new contact and rotate counter-clockwise| `action=ROTATE_COUNTER_CLOCKWISE`; host receives F15 if observed. |
| 6 | Start a new contact and rotate clockwise| `action=ROTATE_CLOCKWISE`; host receives F16 if observed. |
| 7 | Pinch and rotate during one contact| Completion mode becomes `PINCH_ROTATE`; both applicable action names appear, with no scroll or right-click. |
| 8 | Release, then move one finger and scroll with two fingers| Pointer and scroll start normally; no action from the preceding transform is repeated. |

Each transform session must end with exactly one line matching
`2F transform=<mode> completed`. A long movement may produce several action
lines because each configured threshold crossing is one intentional step.

## Failure filter

The run fails if any of these occur:

- Pinch and rotation directions are reversed from the table.
- A transform produces wheel output or a two-finger right-click on release.
- An action appears before `winner=transform`, after `completed`, or in the next session.
- The log contains `transform-pinch ignored: KSCAN callback is not configured`,
  `arbiter output rejected producer=transform`, `proposal rejected`, or a queue-overflow message.
- A transform is reported as scroll, or ordinary parallel scrolling is reported as transform.

Recognition uses `distance-threshold` and `rotation-threshold`. Repeated actions
use `pinch-step-threshold` and `rotation-step-threshold` on
`&iqs9150_transform_gesture`. Tune only after preserving a failing log and
identify classification errors separately from excessive or insufficient repeats.

## 日本語

# ピンチ・回転結合試験

これはtransform機能の1回の実機確認手順である。1つのUSB logging UF2で4方向action、
継続step、scroll/tapとの所有権排他、通常入力への復帰をまとめて確認する。

## ファームとログ取得

```text
build/iqs9150_xiao/zephyr/zmk.uf2
```

|操作 |ログ |ホスト割当 |
|---|---|---|
|指間を狭める | `PINCH_IN` | `Ctrl+-` (zoom out) |
|指間を広げる | `PINCH_OUT` | `Ctrl+=` (zoom in) |
|反時計回り | `ROTATE_COUNTER_CLOCKWISE` | `F15` |
|時計回り | `ROTATE_CLOCKWISE` | `F16` |

## 1回の確認手順

| Order |操作 |合格条件 |
|---:|---|---|
| 1 |1本指移動後、静止した2本指タップ | Pointer and right-click remain normal. No transform action appears. |
| 2 |両方の指を同方向へ平行移動 | Log shows `route=scroll proposed move=(...,...) dot=...` with a positive dot value, followed by `winner=scroll`; scrolling occurs and no transform action appears. |
| 3 |指間を十分に狭める | One `transform=PINCH proposed`, `winner=transform`, and one or more `action=PINCH_IN`; browser zoom decreases. No wheel or right-click occurs. |
| 4 |新しい接触で指間を十分に広げる | `action=PINCH_OUT` appears one or more times; browser zoom increases. |
| 5 |新しい接触で反時計回り | `action=ROTATE_COUNTER_CLOCKWISE`; host receives F15 if observed. |
| 6 |新しい接触で時計回り | `action=ROTATE_CLOCKWISE`; host receives F16 if observed. |
| 7 |同じ接触中にピンチと回転 | Completion mode becomes `PINCH_ROTATE`; both applicable action names appear, with no scroll or right-click. |
| 8 |解放後に1本指移動と2本指scroll | Pointer and scroll start normally; no action from the preceding transform is repeated. |

各transform sessionは `2F transform=<mode> completed` に一致する行を1つだけ出す。
長い操作では設定閾値を越えるたびに1 stepとなるため、action行が複数出ても正常である。

## 異常判定

方向が表と逆、transform中のwheel／解放時右クリック、owner確定前・completed後・次sessionでの
action、callback未設定・output拒否・proposal拒否・queue overflowは不合格とする。

分類は`distance-threshold`と`rotation-threshold`、継続actionは
`pinch-step-threshold`と`rotation-step-threshold`を使う。調整前に失敗ログを保存し、
分類誤りと繰返し過多／不足を分けて判定する。
