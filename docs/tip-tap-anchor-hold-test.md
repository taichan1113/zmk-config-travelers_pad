# Tip-Tap anchor modifier hold

[English](#english) | [日本語](#日本語)

## English

The optional `anchor-hold;` property on the Tip-Tap node holds virtual row 0,
column 2 from the first accepted action until anchor loss, reset, cancellation,
or interruption by more than two contacts. Without the property the original
two-column discrete Tip-Tap behavior is unchanged. The sensor driver is unchanged.

The keyboard maps column 2 independently from left/right keys. In this test the
composite map appends `RC(19, 2)` (leaving existing positions unchanged), binding
`&kp LALT`. Left/right bind `&kp LS(TAB)` / `&kp TAB`. The management-layer entry
is `&trans` so an owed release still resolves to the same modifier binding.
To choose CTRL, use `&kp LCTRL` at the appended position. To use ordinary Tip-Tap,
remove `anchor-hold;` and restore the desired left/right combinations.

The broker authorizes the hold start on the Tip-Tap owner token and serializes
modifier press before discrete key press. Repeats preserve the hold. Normal end
waits for accepted queued key actions to finish (50 ms presses plus release gaps)
before modifier release. This can delay final Alt-Tab selection briefly or longer
after a burst; it is not an instantaneous physical-release guarantee. Cancellation
drops pending pulses and repays active key and modifier releases. Output capacity
is bounded, and only one anchored hold is supported at a time. A new hold while
the previous one drains is rejected rather than sharing its lifecycle.

A modifier held by this virtual key also affects other host key input while
active. This is ordinary modifier behavior, not isolation to TAB alone. Host
window-switcher behavior may vary. Diagnostics, click durations, contact
confirmation and other manually edited bindings are otherwise unchanged.

Flash `travelers-pad-tiptap-alt-anchor-hold-20261009.uf2` from Windows Downloads.
Test on BLE:

1. Open three windows. Hold one stationary anchor for 0.5 seconds and tap a second
   finger on the right. The Alt-Tab switcher should remain open with the anchor.
2. Repeat right taps to move forward; left taps should move backward. SHIFT must
   not remain held after a left action. Release the anchor to confirm selection.
3. Repeat with anchor release immediately after the second-finger tap. Check no
   TAB/ALT remains pressed. Reset once during an active hold and verify normal
   typing is not affected afterward.
4. Briefly check ordinary taps/drag, scrolling/inertia and four-finger management
   hold. Test the property-absent mode before release if switching back to normal
   Tip-Tap. Submit any stuck-modifier or missed action logs.

Host tests cover hold disabled, first/repeated actions, release/reset, token denial,
modifier/key ordering and cancellation before/after press. BLE acceptance remains
pending. Work is on config/features branches `feat/tip-tap-anchor-hold`; main,
west release tags, remote repositories and travelers-system are not changed.

## 日本語

Tip-Tap nodeの任意設定 `anchor-hold;` により、最初の認可済みactionからanchor喪失・
reset・取消し・3本以上への遷移まで、仮想行0列2を保持します。
指定しなければ従来の左右2列の単発Tip-Tap動作です。ドライバは変更しません。

キーボード側で列2を左右キーと独立に割り当てます。この試験ではcomposite map末尾に
`RC(19, 2)` を追加し、既存positionを維持したまま `&kp LALT` を割り当てています。
左右は `&kp LS(TAB)`／`&kp TAB` です。管理レイヤーでは `&trans` とし、保持中に
レイヤーが変わっても対応するmodifier bindingを解放できるようにします。
CTRL保持なら末尾を `&kp LCTRL` に変更します。保持なしは `anchor-hold;` を削除し、
左右に希望する従来の組合せを割り当てます。

brokerがTip-Tap所有tokenで保持開始を認可し、modifier押下→単発キー押下の順を
直列化します。反復中は保持を維持します。通常終了では受付済みキーactionの
50ms押下と解放間隔を完了してからmodifierを解放するため、Alt-Tab確定が少し、
連続入力が滞留した場合はより長く遅れる可能性があります。物理解放と完全同時の
解放保証ではありません。取消しでは未送信pulseを破棄し、押下済みキーとmodifierを
解放します。出力キューは有限で、同時のanchor保持は1件までです。
前回保持の解放待ち中の新規保持は、ライフサイクルを混ぜず棄却します。

保持modifierはTAB以外のホスト入力にも作用します。通常のmodifier動作であり、
TAB専用に隔離するものではありません。ウィンドウ切替UIはOSにより異なります。
診断設定・クリック時間・接触確認・その他の手動bindingは維持しています。

Windows Downloadsの `travelers-pad-tiptap-alt-anchor-hold-20261009.uf2` を書き込み、
BLEで確認します。

1. ウィンドウを3つ開く。anchorを0.5秒静止し、右へ2本目をタップ。
   anchorを維持している間、Alt-Tabの選択画面が残ること。
2. 右を反復して順方向、左を反復して逆方向へ選択。左操作後にSHIFTが残らず、
   anchorを離すと選択が確定すること。
3. 2本目のタップ直後にanchorも離す操作を反復し、TAB／ALTが残らないこと。
   保持中にRESETを1回押し、復帰後の通常入力にALTが残らないこと。
4. 通常タップ／ドラッグ、スクロール／慣性、4本指管理ホールドを簡単に確認。
   保持なし運用へ戻す場合はpropertyを外した版もリリース前に確認します。
   modifier残留や不発があればログを提出してください。

host試験は保持無効、初回／反復、解放／reset、token拒否、modifierとキーの順序、
押下前後の取消しを確認しています。BLE実機合格は未判定です。
config／featuresの `feat/tip-tap-anchor-hold` ブランチで作業し、main・westの
リリースタグ・リモート・travelers-systemは変更しません。
