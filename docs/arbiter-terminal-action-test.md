## English

# Terminal tap / double-click grant test

## Purpose

Verify that a delayed tap or stationary double-click cannot bypass the resolved
contact owner. The arbiter issues a one-shot terminal-action grant only after a
normal release has been completely resolved; the output broker consumes that
grant exactly once before it sends the mouse click.

## Hardware procedure

1. Make one stationary one-finger tap. Confirm exactly one left click, with no
   duplicate click after the double-tap interval.

2. Make stationary two- and three-finger taps. Confirm one right click and one
   middle click respectively, again without a duplicate.

3. Double-tap one finger and release the second tap without moving. Confirm the
   host receives a double-click, not a drag.

4. Double-tap one finger, keep the second touch down, then move. Confirm drag
   begins with one press and finishes with one release; it must not also emit a
   terminal click.

## Expected diagnostics

For a terminal click, the log may contain one `terminal action granted` and one
matching `terminal action consumed` record. A stationary double-click is two
mouse pulses but still one grant consumption. The drag path need not consume a
terminal grant.

Treat `terminal action denied`, `tap click report failed`, `arbiter output
rejected`, a duplicate host click, or a stuck button as a failure. A terminal
frame owned by Tip-Tap, gesture, scroll, or edge dial must instead log a normal
`tap suppressed: resolved terminal owner=...` line and must not request a
terminal-action grant.

## 日本語

# Terminal tap・double-click grant テスト

## 目的

遅延tapまたは静止double-clickが裁定済みcontact ownerを迂回しないことを確認する。arbiterは
通常releaseが完全に裁定された後だけ一回限りのterminal-action grantを発行し、output brokerは
マウスclick送信前にそのgrantを一度だけ消費する。

## 実機手順

1. 1本指で静止tapを1回行う。double-tap判定時間の経過後にも重複せず、左clickが1回だけ送られることを確認する。

2. 2本指、3本指で静止tapを行う。それぞれ右click、中clickが1回だけ送られ、重複しないことを確認する。

3. 1本指でdouble-tapし、2回目を動かさずに離す。hostがdragではなくdouble-clickを受け取ることを確認する。

4. 1本指でdouble-tapし、2回目を保持してから動かす。dragが1回のpressと1回のreleaseで始終し、
   terminal clickが追加送信されないことを確認する。

## 期待する診断

terminal clickでは、`terminal action granted` 1回と対応する `terminal action consumed` 1回が
記録され得る。静止double-clickはマウスpulseが2回でもgrantの消費は1回である。drag経路は
terminal grantを消費しなくてよい。

`terminal action denied`、`tap click report failed`、`arbiter output rejected`、host上の重複click、
または押下状態が残るbuttonは失敗とする。Tip-Tap、gesture、scroll、edge dialがterminal frameを
所有する場合は、代わりに通常の `tap suppressed: resolved terminal owner=...` が出て、
terminal-action grantを要求しないこと。
