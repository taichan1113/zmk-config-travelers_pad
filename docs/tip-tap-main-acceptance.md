# Tip-Tap deadline fix acceptance

[English](#english) | [日本語](#日本語)

## English

The user accepted the deadline fix on BLE. The submitted log records 15 Left and
12 Right Tip-Taps; confirmation wakes produce complete normalized frames followed
by ordinary recognizer proposals and broker-authorized 50 ms key pulses.
No new runtime frame-forwarding, ownership, or output errors were found. The known
QSPI identification mismatch and one startup RDY level rescue are unrelated
observations, not new failures introduced by this fix.

The timer does not read the sensor, emit HID actions, claim ownership, or bypass
the arbiter. Session callbacks serialize contact state, defer partial frames,
check the current deadline, and ignore revoked/stale confirmations. The private
wake is non-sync; consumers receive the same canonical contact-frame path as
physical reports. These preserve the ownership architecture.

Residual risks are not zero: stabilization is shared by all multi-finger
recognizers; short contacts that previously disappeared now reach them correctly.
Scheduling/queue load may delay confirmation. Cached physical state cannot reveal
unreported releases. Host stubs validate the logic but do not exhaust all real
thread interleavings or BLE host behavior. Continue monitoring ordinary taps,
drag, scroll/inertia, pinch and management gestures during normal use.

Regression checks passed for contact deadlines (including early/stale wakes,
release/reset, partial frames and queue pressure), sparse contacts, Tip-Tap
thresholds, 50 ms click/key pacing, multi-finger release, management hold, pointer
release protection, inertia cancellation, driver recovery and structural feature
dependencies. Detailed sensor and Tip-Tap traces are disabled in normal config;
diagnostic code remains optional, default off. The 35 ms confirmation, Tip-Tap
250 ms second-contact limit and 50 ms presses are retained.

Local integration uses driver tag `v0.2.1` and feature tag `v0.4.0`. Neither tags
nor main branches are pushed as part of this integration. Publish the two module
tags/branches before publishing the config manifest for another PC or CI.
The feature main contains the previously accepted click pacing, pointer protection,
multi-finger discrimination and Tip-Tap improvements as well as this deadline fix.
Ubuntu double-click semantics and intermittent BLE latency remain separately
monitored issues, not declared resolved by this acceptance.

## 日本語

期限処理修正はユーザーのBLE実機検証で合格です。提出ログには左15回・右12回の
Tip-Tap認識があり、確認wakeから完全な正規化frame、通常の認識proposal、
broker認可済み50msキー押下へ進んでいます。新たなframe転送・所有権・出力エラーは
見つかりませんでした。既知のQSPI識別不一致と起動時1回のRDY level rescueは、
今回の修正で新たに発生した不具合ではありません。

タイマーはセンサー読み取り、HID発火、所有権取得、arbiter迂回を行いません。
session callbackで状態を直列化し、部分frame受信中は延期、現在の期限を確認し、
取消済み／古い確認を無効にします。内部wakeは非syncで、下流には物理reportと
同じ正規接触frame経路で配信します。所有権アーキテクチャは維持されています。

残余リスクはゼロではありません。安定化は多指認識器で共有するため、以前消えて
いた短い接触が正しく届くようになります。負荷で確定が遅れる場合や、未報告の
物理解放をキャッシュ状態から判断できない制約があります。host stub試験は
全実スレッド競合やBLEホスト挙動を網羅しません。通常タップ、ドラッグ、
スクロール／慣性、ピンチ、管理操作は日常運用でも観察を継続します。

接触期限（早期／古いwake、解放／reset、部分frame、キュー圧力）、疎な接触slot、
Tip-Tap閾値、50msクリック／キー押下、多指解放、管理ホールド、ポインタ解放保護、
慣性取消し、ドライバ復旧、構造／依存関係の回帰試験が合格しました。
通常configでは詳細センサー／Tip-Tap診断を無効化し、診断コードは選択式・標準無効で
残します。35ms確認、Tip-Tapの2本目250ms上限、50ms押下は維持します。

ローカル統合はdriver `v0.2.1`、features `v0.4.0` のタグを使用します。
今回はmain／タグをpushしません。別PCやCIにconfig manifestを公開する前に、
両モジュールのタグとmainを先にpushする必要があります。
features mainには今回の期限修正だけでなく、既に確認した押下間隔・ポインタ保護・
複数指判定・Tip-Tap改善も含みます。Ubuntuのダブルクリック解釈と断続的なBLE遅延は
別途監視中であり、今回の合格で解決済みとは扱いません。
