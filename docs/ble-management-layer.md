# BLE management layer

## English

### Stage 1: select a host and exit

Place four fingers near the center, keep each contact within 60 sensor units
on each axis for 1500 ms, then lift all fingers. The hold claims the existing
multi-finger arbiter owner. Its keyboard action is dispatched only on full
release, preventing the entry fingers from selecting a grid cell. A contact
count change or tracking-ID replacement before confirmation cancels the hold.
Existing swipes and Grasp/Release bindings remain unchanged.

The management layer uses this two-row, three-column layout:

| Left | Center | Right |
| --- | --- | --- |
| Host 1 | Host 2 | Host 3 |
| Host 4 | Host 5 | Exit |

A stationary one-finger tap selects the corresponding zero-based ZMK profile
and returns to the normal layer. Grid forwarding starts after the existing
80 ms one-finger settle interval. Management uses the same surface coordinates
as the pointer (X: 2000, Y: 1213).

The RGB LED widget shows the management layer in magenta. BLE status remains
the widget's standard connected/advertising/disconnected indication; this does
not display the profile number. USB output behavior is unchanged. When checking
BLE host switching, disconnect USB after writing the firmware, or otherwise
ensure BLE is the active host output.

Host mouse output is gated during management. Tap/drag recognition is suspended
on layer 1 and its delayed click state is cleared. The grid processor receives
only its private stream, and only on layer 1. Other virtual-key actions use
`&none` on that layer. Recognition of other functions is not globally stopped.
The keyboard-local output gate does not mutate raw frames or arbiter ownership,
and preserves releases for previously delivered button presses.

There is no automatic management timeout in this stage. The exit cell or a
second four-finger hold returns to normal. Restart also restores the base layer.

### BT_CLR is not enabled yet

The intended final lower-right action is short press: exit; long press: clear
the currently selected host. Stage 1 deliberately contains no BT_CLR binding:
holding the exit cell does not erase pairing. After grid and transitions pass
hardware checks, stage 2 will enable held grid keys and a guarded 3-second
hold/short-tap binding. No pairing or settings reset is part of stage 1.

### Hardware checks

1. Four-finger hold for roughly two seconds, lift, and confirm the management
   LED color. A short four-finger touch or an ordinary swipe must not enter.
2. Tap the lower-right cell, lift, and verify ordinary pointer, tap, drag,
   two-finger scroll, and inertia resume. Repeat entry and exit several times.
   No host click or motion should result from management cell operations.
3. Select each desired profile, pair it if empty, and check pointer delivery
   over BLE. Unused profiles may remain unpaired; that is expected.
4. Confirm normal multi-finger, Tip-Tap, edge, and pinch actions remain usable
   outside management. Check transitions after inertia as well.

Useful logs: `4F hold confirmed; action waits for all contacts to lift`,
`4F hold action dispatched after release`, and the `zip_matrix` gesture logs.

### Development and verification

The driver is unchanged. Optional four-finger hold and the generic tap/drag
`disabled-layer` setting live on the feature module's development branch,
selected by the keyboard west manifest. Existing module main and release tags
are unchanged. The LED project uses the requested `v0.3-branch` revision.

Host tests using extracted production C with platform stubs cover hold timing,
per-contact movement and identity, count changes, reset, competing owners,
one-shot release output, preserved swipe recognition, default-disabled behavior,
and host/grid output gating across layer transitions. These tests do not model
sensor noise or asynchronous Bluetooth delivery. The integrated firmware build
passes, with 31 bindings per layer and the original 24 bindings preserved.

## 日本語

### 第1段階：接続先選択と終了

パッド中央付近に4本指を置き、各指のX/Y変位をセンサー座標60以内に保って
1500 ms静止し、全指を離してください。既存の多指arbiter ownerを取得し、
全指解放後にキーマップ操作を発火します。入口の指が格子を選ぶことを防ぐためです。
成立前の本数変更やtracking-IDの入れ替えでホールド候補を取り消します。
既存のスワイプとGrasp/Releaseの割り当ては維持しています。

管理レイヤーの2行×3列の配置：

| 左 | 中央 | 右 |
| --- | --- | --- |
| 接続先1 | 接続先2 | 接続先3 |
| 接続先4 | 接続先5 | 終了 |

1本指を静止タップして離すと、対応するZMKプロファイルを選び、通常レイヤーへ
戻ります。番号は内部では0始まりです。格子転送には既存の1本指確定時間80 msが
あるため、極端に短い接触は対象外です。座標範囲はポインタと同じX:2000、Y:1213です。

RGB LED widgetで管理レイヤーを紫色に表示します。BLE状態表示はwidget標準の
接続済み・ペアリング待ち・未接続表示で、接続先番号の表示ではありません。
USB出力の設定は変更していません。BLE切替の検証時は書き込み後にUSBを外すか、
BLEが出力先になっていることを確認してください。

管理中はホストへのマウス出力を抑止します。tap/dragはレイヤー1で認識を休止し、
遅延クリック状態も破棄します。格子processorは専用ストリームだけをレイヤー1で
処理し、他の仮想キーはそのレイヤーで`&none`にします。他機能の認識全体を止める
構成ではありません。キーボード固有の出力ゲートは生frameやarbiter所有権を変更せず、
送信済みボタン押下の解放を維持します。

この段階では自動終了タイムアウトはありません。終了区画、またはもう一度の
4本指ホールドで通常レイヤーへ戻れます。再起動でも基本レイヤーへ戻ります。

### BT_CLRはまだ有効にしていません

最終仕様は右下の短押しで終了、長押しで選択中の接続先消去です。
第1段階には意図的にBT_CLRを割り当てていません。右下を保持してもペアリングは
消えません。格子とレイヤー遷移の実機確認後、第2段階で格子の保持キー出力と、
3秒長押し／短押しの保護付き割り当てを有効にします。第1段階では接続情報・
settingsのリセットも行いません。

### 実機確認

1. 4本指を約2秒静止して離し、管理レイヤーのLED色を確認します。
   短い4本指接触や通常のスワイプでは入らないことも確認します。
2. 右下をタップして離し、通常のポインタ・タップ・ドラッグ・2本指スクロール・
   慣性へ戻ることを確認します。数回繰り返し、管理区画の操作でホストへの
   クリック・移動が出ないことを確認します。
3. 必要な接続先を選び、空ならペアリングし、BLEでポインタを確認します。
   未使用の接続先が未ペアリングのままでも正常です。
4. 管理外で多指・Tip-Tap・エッジ・ピンチの既存操作を確認します。
   慣性の後に管理へ入る遷移も確認してください。

目印のログは`4F hold confirmed; action waits for all contacts to lift`、
`4F hold action dispatched after release`、および`zip_matrix`のジェスチャーです。

### 開発状態と検証

ドライバは変更していません。任意の4本指ホールドと汎用tap/dragの`disabled-layer`
設定は機能モジュールの開発ブランチにあり、キーボードのwest manifestで選択します。
既存moduleのmainとリリースタグは変更していません。LEDは指定の`v0.3-branch`です。

実コードを抽出しプラットフォーム部分をスタブ化したホストテストで、成立時間、
各指の移動・同一性、本数変化、リセット、競合owner、解放後の一度だけの出力、
既存スワイプ、既定無効、レイヤー遷移時のホスト／格子出力ゲートを確認しました。
センサーノイズや非同期BLE送信はテスト対象外です。結合ファームのビルドは成功し、
各レイヤー31 bindingと、従来24 bindingの維持も確認しています。
