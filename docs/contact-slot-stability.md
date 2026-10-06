[English](#english) | [日本語](#日本語)

# Sparse contacts and transient second contacts

## English

### Evidence and scope

During continuous one-finger motion on battery/BLE, a raw two-contact report at
20.355 seconds returned to one at 20.367 seconds. Pointer XY events stayed at
1240 until full release/recontact at 32.598/33.097 seconds. Later two-contact
bursts lasted 12–26 milliseconds. Some count-one frames had absent slot-zero
coordinates while slot one held a position. BLE remained connected at latency
zero. This reproduces an input interpretation failure; it does not establish
the cause of the earlier long-idle/no-RDY symptom or of the sensor's extra-contact
detection. No voltage, finger-split tuning, I2C polling, or recovery policy changes
are included.

The [Azoteq datasheet, section 7.2.6](https://www.azoteq.com/images/stories/pdf/IQS9150_IQS9151_datasheet.pdf)
specifies that tracked fingers keep their memory-map locations. Active contact
count must not be used as an upper bound on slot indices.

### Driver changes

All seven physical slots are decoded independently of their index. Nonempty,
confident coordinates are counted as normalized contacts; hardware zero count
invalidates all slots. Slot indices/tracking IDs remain physical, not compacted.
The legacy representative stays on its previous active slot; on loss, another
valid slot is selected. An internal MSC reference-change marker signals a
motion rebase, not a sensor reset or ownership cancellation.

### Feature changes

The session manager buffers one complete slot frame before publishing it.
Adding a second contact during an already active one-contact stream requires
the same two-slot set to persist for 35 milliseconds. Short additions are hidden
from every recognizer, including tap/drag, Tip-Tap, pinch and scroll. During
confirmation the surviving established slot continues as the one-contact frame.
Initial multi-contact touches, three-or-more contacts, full release and reset
are immediate. Once a two-contact stream is accepted, existing routing and
ownership rules still apply: this is not a general multi-to-one pointer fallback.

Configure at the keyboard level:

```dts
&iqs9150_session {
    extra-contact-confirm-ms = <35>; /* 0 disables confirmation */
};
```

Consequently an added second contact lasting less than 35 milliseconds is not a
Tip-Tap; a sustained second contact is observed after confirmation. The first
pointer contact has no new waiting period. Router motion and tap/drag rebase on
representative changes without releasing a held button or changing its owner.

### Software and hardware checks

`scripts/test-contact-stability.sh DRIVER FEATURES` compiles production decoder
helpers and the production session manager against host input/arbiter stubs.
It checks sparse slot six with count one, confidence/absent-coordinate rejection,
sticky representative selection, 100 repeated 26ms bursts, 35ms confirmation,
initial multi-touch, 3F, release, reset, reference changes and disabled filtering.
Driver recovery, inertia cancellation, architecture and dependency regressions
also pass. These checks do not simulate Windows BLE delivery or analog sensing.

The validation build uses local development worktrees
`/tmp/iqs9150-warm-start-fix` (`fix/sparse-contact-slots`) and
`/tmp/trackpad-inertia-cancel-fix` (`fix/transient-contact-count`) through a
`ZMK_EXTRA_MODULES` override. Manifest revisions and remote branches are unchanged;
`west update` alone will not select this unaccepted candidate.

1. With battery ON, remove USB. Move one finger slowly then quickly for 30 seconds
   without lifting. It must not remain stuck or jump to the screen edge.
2. Check ordinary tap/double-tap/drag, actual two-finger scrolling and pinch,
   and anchored Tip-Tap. Accepted scroll must not leak ordinary pointer movement
   when a contact is removed. A deliberately sub-35ms added contact is suppressed.
3. If a stop occurs, keep moving ten seconds, then lift/recontact once. Attach
   USB without RESET and save `history begin` through `history end`.

Only after these pass, resume the separate battery/BLE idle test for 3–5 minutes,
then 30 minutes. If stopped, try pointer/tap/scroll for ten seconds and capture
RAM history by attaching USB without RESET. No idle fix is claimed yet.

## 日本語

### 根拠と対象範囲

バッテリー/BLEの1本指連続移動中、20.355秒に2点報告、20.367秒に1点へ戻る
12msの変化が発生しました。XY回数は1240で停止し、32.598秒の解除・33.097秒の再接触で復帰します。
後続にも12〜26msの2点報告があり、本数1でもslot 0は無効座標、slot 1に位置が残る例があります。
BLEはlatency=0で接続を維持しています。今回の入力解釈上の問題を再現できましたが、
以前の長時間放置時のRDY停止や、センサーが余分な接触を報告する原因まで同一とは確定しません。
電圧・Finger Split・I2Cポーリング・復旧方針は変更していません。

[Azoteq公式datasheet §7.2.6](https://www.azoteq.com/images/stories/pdf/IQS9150_IQS9151_datasheet.pdf)
では、追跡中の指がメモリマップ内の同じ位置に残ると規定しています。
接触数をslot indexの上限にはできません。

### ドライバー変更

物理7スロットをindexによらず確認し、有効座標・confidenceのある接触を正規化本数とします。
ハードウェア本数0なら全slotを無効とします。slot番号/tracking IDは詰め直さず物理位置を維持します。
legacy代表指は以前のslotが有効なら継続し、消えたら別の有効slotへ移ります。
交代は内部MSC markerで通知し、移動基準だけ更新します。センサーresetや所有権取消ではありません。

### 機能モジュール変更

session managerが完全なslot frameを一旦保持してから配信します。
継続中の1本指へ2本目が加わる場合、同じ2スロット構成が35ms続いたら認識器へ渡します。
短い追加接触はtap/drag・Tip-Tap・pinch・scrollを含む全認識器から隠し、
確認中は残っている既存slotで1本指操作を継続します。
最初から多指で触れる場合、3本以上、全解除、resetは即時です。
2本指として受理された後は従来のrouting/所有権を維持します。多指→1本を常にポインタへ戻す変更ではありません。

キーボード側で設定できます：

```dts
&iqs9150_session {
    extra-contact-confirm-ms = <35>; /* 0なら確認を無効化 */
};
```

追加した2本目を35ms未満で離す操作はTip-Tap等として認識しません。
2本目を維持した場合は確認後に渡します。最初のポインタ接触に新たな待ち時間はありません。
代表指交代時はrouterとtap/dragの移動基準を更新し、押下中buttonやownerは維持します。

### ソフトウェア検証・実機確認

`scripts/test-contact-stability.sh DRIVER FEATURES` は実decoder helperと実session managerを
host input/arbiter stubへ組み合わせます。接触数1でslot 6のみ有効な場合、confidence・無効座標、
代表指維持、26msの追加接触100回、35ms確認、初回多指・3F・解除・reset・代表交代・確認無効を検証します。
driver復旧・慣性キャンセル・アーキテクチャ・依存関係の回帰試験も通過しています。
WindowsへのBLE到達やアナログセンシング自体は模擬していません。

検証ビルドは `/tmp/iqs9150-warm-start-fix`（`fix/sparse-contact-slots`）、
`/tmp/trackpad-inertia-cancel-fix`（`fix/transient-contact-count`）のローカル開発worktreeを
`ZMK_EXTRA_MODULES` で指定しています。manifest revisionやremote branchは変更しておらず、
`west update` だけでは未合格の今回の候補へ切り替わりません。

1. バッテリーONでUSBを外し、1本指を離さず遅く→速く30秒動かします。
   停止し続けたり、画面端へ飛んだりしないことを確認します。
2. tap/double-tap/drag・実際の2本指scroll/pinch・anchor付きTip-Tapを確認します。
   確定済みscrollで1本を離しても普通のポインタ移動へ漏れないことも確認します。
   意図的に35ms未満で追加指を離す場合は抑制対象です。
3. 停止したら10秒動かしてから一度離して再接触し、RESETせずUSB接続して
   `history begin`〜`history end` 全体を保存します。

合格後に別件のバッテリー/BLE放置試験を3〜5分、続いて30分で再開します。
停止時は移動・tap・scrollを10秒試し、RESETせずUSBを戻して履歴を採取します。
長時間放置の修正完了はまだ主張しません。
