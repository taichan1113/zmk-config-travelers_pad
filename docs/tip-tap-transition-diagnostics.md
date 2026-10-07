# Tip-Tap transition diagnostics

[English](#english) | [日本語](#日本語)

## English

This test enables `CONFIG_TRAVELERS_PAD_TIP_TAP_DIAGNOSTICS=y` without changing
recognition thresholds or 50 ms discrete key presses. It observes normalized
input, not analog sensing or raw pre-normalization sensor frames. Diagnostics
default to disabled in the module. No driver changes are included.

`Tip-Tap trace` logs each contact-count transition and an active-contact snapshot
once per second between transitions. It includes frame sequence, the previous
attempt number, candidate state, ownership, and the two coordinate slots.
An entry into two contacts increments `attempt`; the subsequent candidate or
unarmed line carries that new attempt number. `candidate ... armed=1` means a
candidate was created. `armed=0` means no second valid slot was found (the second
slot fields then may be stale). `unarmed` identifies a missing prior anchor;
`interrupted` identifies a candidate ending other than through 2->1.
Existing ignored logs explain age, movement, identity, duration, separation,
and ownership rejection. Serial logging can affect timing; disable it after
diagnosis. The trace is not printed for every steady-state sensor frame.

Flash `travelers-pad-tiptap-transition-diagnostics-20261008.uf2` from Windows
Downloads. Use BLE for input and USB only for power/serial capture:

1. In a browser, hold one stationary anchor for about 0.5 seconds; tap the second
   finger briefly, well to the left or right. Keep the anchor down for 10 taps.
2. Note the approximate time of each missing action. After a failure, keep only
   the anchor down for two seconds before retrying, making the trace readable.
3. Submit the complete log, the intended tap count, and the failure times. Also
   briefly check ordinary two-finger tap/scroll and one-finger drag.

Interpretation: trace staying at one contact suggests the required normalized
second-contact transition was absent, but does not establish a sensor fault.
A 1->2 transition without a candidate points to an unarmed condition. A candidate
followed by an ignored/interrupted message identifies the rejection path.
Recognition followed by paced key press/release shifts investigation to output
and host processing; it does not prove host receipt.

## 日本語

この試験版は `CONFIG_TRAVELERS_PAD_TIP_TAP_DIAGNOSTICS=y` を有効化します。
認識閾値と単発キーの50ms押下は変更しません。観測対象は正規化入力で、
アナログ検出や正規化前のセンサー生データではありません。モジュールの標準は
診断無効で、ドライバは変更していません。

`Tip-Tap trace` は指本数の変化時と、接触継続中の1秒ごとの状態を記録します。
frame連番、直前のattempt番号、候補・所有状態、2つの座標slotを含みます。
2本への遷移時にattemptを増やし、その後のcandidate／unarmedログは新しい番号を
使います。`candidate ... armed=1` は候補成立、`armed=0` は有効な2本目slotが
見つからなかった状態です（この場合、secondの項目は前回値の場合があります）。
`unarmed` は先行anchorがない状態、`interrupted` は候補が2→1以外の遷移で
終了したことを示します。既存のignoredログは時間・移動・接触ID・指間隔・
所有権による棄却を示します。ログ自体がタイミングへ影響する可能性があるため、
調査後は無効化します。静止中の全センサーフレームを出力する方式ではありません。

Windows Downloadsの `travelers-pad-tiptap-transition-diagnostics-20261008.uf2`
を書き込みます。入力はBLE、USBは給電とシリアル記録のみです。

1. ブラウザで片指を約0.5秒静止し、十分左右に離した位置へ2本目を短くタップ。
   anchorを維持して10回繰り返します。
2. 不発のおよその時刻を控え、不発後はanchorだけを2秒維持してから再試行します。
3. ログ全文・操作回数・不発時刻を提出してください。通常の2本指タップ／
   スクロール、1本指ドラッグも簡単に確認します。

traceが1本のままなら必要な2本目の正規化遷移が届かなかった可能性がありますが、
センサー不具合とは断定できません。1→2で候補がなければunarmed条件を確認し、
候補の後にignored／interruptedがあれば具体的な棄却経路を確認します。
認識とpacedキー押下・解放が揃っていれば出力／ホスト処理を調べます。
キー出力ログだけでホスト受信成功を保証するものではありません。
