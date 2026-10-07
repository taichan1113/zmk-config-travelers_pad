# Tip-Tap sensor and normalized-contact comparison

[English](#english) | [日本語](#日本語)

## English

The last submitted failure at 01:06.352 was a recognized second contact lasting
182 ms, rejected by the 180 ms limit. The keyboard's Tip-Tap-only
`second-contact-timeout-ms` is now 250 ms; the anchor hold remains 250 ms and
mouse/gesture key presses remain 50 ms. No sensor recognition, sampling,
power-policy, or contact-stabilization settings are changed.

`CONFIG_IQS9150_CONTACT_DIAGNOSTICS=y` adds `contact sensor` logs to already-read
reports. No extra I2C transactions are performed. It shows sensor-reported count
(`raw`), decoded valid-slot count (`decoded`), valid/confidence/non-sentinel
coordinate masks for all seven slots, and coordinates of slots 0 and 1. Reports
are logged when these counts/masks change, plus at most once per second between
transitions while contacts remain present. Masks reveal sparse slots even though
only the first two coordinate pairs are printed. These are digital sensor reports,
not analog measurements. Diagnostics default off in the driver.

Compare these logs with the existing `Tip-Tap trace` after session normalization.
Different report/frame sequence domains must be correlated by time, not by equal
sequence numbers. A raw second contact without a valid decoded slot suggests
confidence/coordinate gating. A decoded second contact without a normalized
transition points to session filtering or frame delivery; errors and transition
timing must be examined before assigning a cause. Missing raw second contacts
cannot establish whether the cause is sensing, sampling, or report acquisition.
Diagnostics themselves may affect timing.

The session currently confirms a second contact added to an existing single
contact for 35 ms. Shorter raw contacts can therefore disappear before the
Tip-Tap recognizer sees them. This policy protects contact stability and is
unchanged for this comparison; the new logs will check whether it explains the
reported missing transitions.

Flash `travelers-pad-tiptap250-sensor-diagnostics-20261008.uf2` from Windows
Downloads. Use BLE for input, USB for serial capture only.

1. Keep one anchor stationary for 0.5 seconds, then perform 10 brief second-finger
   taps to either side, maintaining the anchor. Record failure times and count.
2. Repeat with deliberate second contacts of about 0.2 seconds, not long holds.
3. Briefly check ordinary two-finger taps/scrolling and one-finger drag.
4. Submit the full log and approximate failure times. After a failure keep the
   anchor alone for two seconds before retrying to separate attempts in the log.

Automated tests cover 182/250/251 ms recognition, driver diagnostic masks,
normalization regression, and 50 ms output pacing. BLE acceptance remains pending.
The driver diagnostic is on a local test branch; release west revisions and main
are unchanged. Local test builds pass both module paths explicitly.

## 日本語

前回の01:06.352の不発は、2本目を認識したものの接触時間182msが上限180msを
超えたためでした。キーボード設定のTip-Tap専用 `second-contact-timeout-ms`
を250msへ変更しました。anchorの静止時間250ms、マウス／キー押下50msは維持します。
センサー認識・sampling・省電力方針・接触安定化設定は変更していません。

`CONFIG_IQS9150_CONTACT_DIAGNOSTICS=y` により、読み取り済みreportから
`contact sensor` を記録します。追加のI2C通信は行いません。
センサー報告本数raw、有効slot数decoded、全7slotの有効／confidence／
非sentinel座標mask、slot0/1の座標を表示します。本数・maskの変化時と、
接触中の変化がない期間に最大1秒に1回記録します。座標の表示は2slotですが、
maskは飛び飛びのslotも示します。アナログ測定ではなくデジタルreportの診断です。
ドライバの標準設定は診断無効です。

session正規化後の `Tip-Tap trace` と時刻で照合します。
reportとframeは異なる連番なので、番号が等しいとは限りません。
rawが2本でもdecodedが1本ならconfidence／座標による除外を疑い、decodedが
2本でも正規化遷移がなければsessionフィルタやframe配信を調べます。
エラーと遷移時間を確認してから原因を判断します。rawにも2本目がない場合、
検出・sampling・report取得のどこが原因かは、それだけでは断定できません。
診断ログ自体がタイミングへ影響する可能性もあります。

現在のsession処理は、継続中の1本指へ追加された2本目を35ms確認します。
短い生接触はTip-Tap認識器へ届く前に除外される可能性があります。
接触安定性を保護する処理のため今回は変更せず、今回の遷移欠落の原因かどうかを
新しいログで確認します。

Windows Downloadsの `travelers-pad-tiptap250-sensor-diagnostics-20261008.uf2`
を書き込みます。入力はBLE、USBはシリアル記録用です。

1. anchorを約0.5秒静止し、左右へ2本目を短く10回タップ。anchorは維持し、
   不発時刻と操作回数を控えます。
2. 次は2本目を約0.2秒接触させる操作で反復します。長押しにはしません。
3. 通常の2本指タップ／スクロール、1本指ドラッグも簡単に確認します。
4. ログ全文と不発時刻を提出してください。不発後はanchorだけを2秒維持して
   再試行すると、ログ上で操作を分離しやすくなります。

自動試験では182／250／251msの判定、ドライバ診断mask、正規化の回帰、
50ms押下を確認しています。BLE実機合格は未判定です。
ドライバ診断はローカル試験ブランチに保存し、リリースwest revisionとmainは
変更しません。ローカル試験ビルドでは両モジュールのパスを明示します。
