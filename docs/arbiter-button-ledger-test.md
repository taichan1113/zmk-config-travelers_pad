# Terminal button ledger test / Terminal button ledgerテスト

## Purpose / 目的

Verify that a drag button release is accepted only when the same trackpad
recognizer previously pressed that exact button under the same owner token.
The broker owns this held-button ledger; recognizer-local state alone cannot
release a host button.

drag buttonのreleaseが、同じtrackpad recognizerが同じowner tokenで正確にそのbuttonを先に
押していた場合だけ許可されることを確認する。brokerがheld-button ledgerを所有し、recognizer
局所stateだけではhost buttonをreleaseできない。

## Hardware procedure / 実機手順

1. For one, two, and three fingers separately, double-tap, hold the second
   touch, move enough to start drag, then release all fingers.

   1本、2本、3本指それぞれでdouble-tapし、2回目を保持してdrag開始距離まで動かし、全指を
   離す。
2. For every case, confirm the matching left/right/middle button is held only
   while dragging and is released immediately on lift.

   各操作で、対応する左／右／中buttonがdrag中だけ押下され、指を離した直後にreleaseされることを
   確認する。
3. After each drag, perform a normal one-finger move and tap. Confirm no button
   remains held and the normal click still occurs once.

   各dragの後、通常の1本指移動とtapを行う。buttonが残留せず、通常clickが1回発生することを
   確認する。

## Expected diagnostics / 期待する診断

Each drag has one authorized `tap-drag-button-down` and one accepted terminal
release record. No `duplicate held button denied`, `held-button ledger full`,
`terminal button release denied`, or stuck host button is allowed.

各dragは認可済み `tap-drag-button-down` 1回と、acceptされたterminal release記録1回を持つ。
`duplicate held button denied`、`held-button ledger full`、`terminal button release denied`、
またはhost buttonの残留は失敗とする。
