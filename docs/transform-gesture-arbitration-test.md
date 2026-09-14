# Transform gesture arbitration test / Transform gesture裁定試験

Use this procedure after changing `transform_gesture`, the router, or arbiter
priority. It verifies that two-finger pinch/rotation owns a session before
pointer scrolling or terminal tap output can inherit it.

`transform_gesture`、router、またはarbiter priorityを変更した後に実行する。2本指
pinch/rotation がsessionを所有し、pointer scrollやterminal tapが同じsessionを継承
しないことを確認する。

## Setup / 準備

1. Flash the full `iqs9150_xiao` UF2.
2. Capture the serial log at INFO level.
3. Open a host application where ordinary 2F scrolling and a right-click are
   observable. Transform mode currently records arbitration diagnostics only.

## Cases / 手順

| Case / 操作 | Required result / 合格条件 |
|---|---|
| 2F parallel movement | `2F route=scroll proposed` is followed by an arbiter `owner=scroll` decision; normal scroll occurs. |
| 2F pinch | Log contains `2F transform=PINCH proposed` and a transform owner decision. No wheel event or right click occurs on release. |
| 2F rotation | Log contains `2F transform=ROTATE proposed` and a transform owner decision. No wheel event or right click occurs on release. |
| Simultaneous pinch + rotation | Log contains `2F transform=PINCH_ROTATE proposed`. No scroll or terminal tap follows. |
| Reset during transform | Reset/recovery clears the session. The next touch starts normally and no old transform completion is logged. |

The thresholds are device-tuning values, so the exact physical movement is not
the pass criterion. The owner decision and absence of incompatible output are.

thresholdはdevice tuning値であるため、物理的な移動量そのものは合否基準ではない。
arbiterのowner決定と、非互換な出力が出ないことを合格基準とする。
