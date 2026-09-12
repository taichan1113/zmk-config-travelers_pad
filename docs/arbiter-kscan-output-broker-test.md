# Primary virtual-KSCAN broker test / Primary仮想KSCAN brokerテスト

## Purpose / 目的

Verify that all primary virtual-KSCAN actions use the contact output broker.
The broker authorizes the exact arbiter token and emits one inseparable
press/release pair for edge dial, Tip-Tap, 3/4-finger swipe, and 5-finger
Grasp/Release.

すべてのprimary仮想KSCAN actionがcontact output brokerを通ることを確認する。brokerは
正確なarbiter tokenを認可し、edge dial、Tip-Tap、3/4本指swipe、5本指Grasp/Releaseについて
不可分なpress/release対を1回送信する。

## Hardware procedure / 実機手順

1. Perform one configured edge-dial detent. Confirm only its mapped action is
   emitted and ordinary pointer movement does not leak.

   有効化したedge dialを1 detent分だけ操作する。対応する割当だけが発火し、通常ポインタの
   移動が漏れないことを確認する。
2. Hold an anchor finger and perform one left and one right Tip-Tap. Confirm
   each assigned keymap action occurs exactly once.

   anchor指を保持し、左・右それぞれTip-Tapを1回行う。各keymap actionが1回だけ発火することを
   確認する。
3. Perform one 3F swipe and one 4F swipe. Confirm their assigned actions occur
   once and no mouse-button or wheel action occurs.

   3本指・4本指swipeを各1回行う。割当actionが1回だけ発火し、マウスbuttonやwheel actionが
   混在しないことを確認する。
4. Perform one 5F Grasp and one 5F Release. Confirm each configured action
   occurs once.

   5本指Grasp・Releaseを各1回行う。各設定actionが1回だけ発火することを確認する。

## Expected diagnostics / 期待する診断

The action logs (`edge dial`, `Tip-Tap`, `3F/4F swipe`, or `5F`) must correspond
to one host action. No `arbiter output rejected`, `KSCAN callback is not
configured`, duplicate key action, or incompatible pointer/scroll/button output
is allowed.

操作log（`edge dial`、`Tip-Tap`、`3F/4F swipe`、`5F`）はhost action 1回に対応する。
`arbiter output rejected`、`KSCAN callback is not configured`、重複key action、または
非互換なpointer/scroll/button出力は失敗とする。
