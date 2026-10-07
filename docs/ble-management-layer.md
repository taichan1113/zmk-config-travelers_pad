# BLE management by four-finger gestures

## English

The grid-based Stage 1 hardware check passed, but cell locations were difficult
to discover. Management now uses four-finger gestures only. Grid input/listener
and its composite mapping are disabled. No driver or feature-module change was
needed for this revision; the existing hold recognizer and standard ZMK
behaviors are reused. Driver v0.2.0 and the accepted ALP-off policy remain.

Enter: hold four fingers still for 1000 ms; management activates while fingers
remain down. Release when the LED becomes cyan. Normal layer is off, matching
zmk-config-mimitabu layer 0/1 colors. Continued holding cannot toggle again.
Exit: a new four-finger hold for 1000 ms; release after the layer LED turns off.
The action is emitted once after live-frame arbiter resolution; the contact
session remains owned until full release, preventing entry fingers from swiping
on the new layer. Selection and clearing stay
in management until explicitly exited. Mouse output remains gated; tap/drag is
suspended in layer 1. Base-layer gestures are unchanged.

| Four-finger action in management | Binding |
| --- | --- |
| Left swipe | BT_NXT (next profile, wraps) |
| Right swipe | BT_PRV (previous profile, wraps) |
| Up swipe | BT_SEL 0 (first profile) |
| Single down swipe | None |
| Two separate down swipes | Tap dance: BT_CLR for the currently selected profile |
| Stationary hold 1s | Exit to layer 0 while still touching |

The double-down tap dance uses tapping-term-ms=1500. Lift all fingers between
swipes; the second gesture event must arrive within the tap-dance window.
This setting measures virtual key events, not initial physical touch times.
A single down event times out to &none. An intervening virtual key interrupts
the dance. BT_CLR is not BT_CLR_ALL: only the current profile is cleared.
Clearing a paired profile requires re-pairing and may require forgetting its
old device entry in the host OS. The LED provides standard BLE state, not a
numeric profile indication. USB remains power/serial; HID is BLE-only.

Hardware check (not yet accepted):
1. Enter and exit twice. Short four-finger taps must not enter; normal pointer,
   taps/drag/scroll must return after exit. No host mouse events in management.
2. Enter, swipe left/right/up. Verify profile/status changes using host Bluetooth
   settings/LED/logs; up returns to the first profile. Switching does not exit.
3. Choose a disposable profile. Swipe down once, wait at least two seconds:
   its pairing must remain. Then swipe down twice quickly with full finger
   release between strokes: only that selected profile should be cleared.
   Do not perform this destructive check on a pairing you need to preserve.
4. Hold/release to exit. Verify ordinary input and five-minute idle resume.

The gesture-management revision passed all user hardware checks. This new
immediate-hold revision is pending: verify LED changes before finger release,
continued holding for 3s does not toggle back, and release adds no extra action.
UF2: C:\Users\takec\Downloads\travelers-pad-ble-management-hold1s-20261007.uf2.
Host tests: bash scripts/test-ble-management.sh plus arbiter/dependency checks.
All main branches stay unchanged until hardware acceptance.

## 日本語

格子版第1段階は実機合格しましたが、区画位置が分かりづらいため管理操作を4本指
ジェスチャーへ変更。格子input/listenerとcomposite mapを無効化しました。
この変更ではdriver・機能モジュールの追加改修は不要。既存ホールドとZMK標準behaviorを
使用し、driver v0.2.0・合格済みALP無効対策を維持します。

入口：4本指を1000 ms静止。置いたまま管理に入り、LEDがシアンになったら離す。
通常レイヤーは消灯で、zmk-config-mimitabuのレイヤー0/1の色に統一。
保持継続では再切替しません。終了は一度全指を離してから新しく4本指を1秒静止し、
レイヤー表示が消えたら離します。生frameのarbiter裁定後に一度だけ発火。
全指解放まで同一sessionを所有し、切替後の置いた指が別のスワイプになることを防ぎます。
接続先選択・消去では自動終了しません。
管理中のマウス出力を抑止し、レイヤー1でtap/dragを休止。通常レイヤーの割当は不変。

| 管理中の4本指操作 | 割当 |
| --- | --- |
| 左スワイプ | BT_NXT：次の接続先（末尾から先頭へ循環） |
| 右スワイプ | BT_PRV：前の接続先（先頭から末尾へ循環） |
| 上スワイプ | BT_SEL 0：接続先1 |
| 下スワイプ1回 | 何もしない |
| 下スワイプ2回 | タップダンスで現在の接続先をBT_CLR |
| 1秒静止ホールド | 指を置いたまま通常レイヤーへ終了 |

タップダンスのtapping-term-msは1500。毎回全指を離し、判定時間内に2回目の
ジェスチャーeventを送ります。時間基準は仮想キーeventで、物理的に指を置いた時刻では
ありません。単発は時間切れで&none。他の仮想キー操作が間に入るとダンスを中断。
BT_CLR_ALLではなく、選択中の接続先だけを消去します。
消去後は再ペアリングが必要で、PC側で古い登録の削除が必要な場合もあります。
LEDは標準のBLE状態を示し、接続先番号の表示ではありません。USBは給電・シリアル、
HIDはBLEのみです。

実機確認（新版は未確認）：
1. 入口・終了を2回確認。短い4本指タップでは入らないこと。終了後に通常操作が戻り、
   管理中にホストへマウス操作が漏れないこと。
2. 管理に入り左/右/上スワイプ。PCのBluetooth状態・LED・ログで切替確認。
   上で接続先1へ戻ること。切替後も管理中であること。
3. 消してよい接続先を選択。下1回→2秒以上待つ：登録維持。
   全指を毎回離して下2回を続ける：選択中だけ消去。
   保存したいペアリングではこの破壊的試験をしないでください。
4. ホールドして離して終了。通常操作と5分放置後の復帰を確認。

ジェスチャー管理版は全操作実機合格。今回の即時ホールド版は未確認。
指を離す前にLEDが変わり、3秒保持しても戻らず、離しても追加操作が出ないことを確認。
UF2：C:\Users\takec\Downloads\travelers-pad-ble-management-hold1s-20261007.uf2。
ホスト確認：bash scripts/test-ble-management.shとarbiter/依存回帰確認。
実機合格までは各mainを変更しません。
