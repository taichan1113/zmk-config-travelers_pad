## English

# Test guide index

Keep these documents as regression-test assets. They record reproducible
hardware checks for interactions that have previously failed or regressed.
The normal workflow uses the full integration test first; isolated tests are
used only when an integration failure needs to be localized.

## Normal release check

| Document| Use it when|
|---|---|
| [`scripts/verify-contact-arbiter-architecture.sh`](../scripts/verify-contact-arbiter-architecture.sh) | Run before every release build; checks the single-owner source invariants|
| [`scripts/verify-contact-feature-dependencies.sh`](../scripts/verify-contact-feature-dependencies.sh) | Before making a feature-limited diagnostic UF2; checks supported module dependencies|
| [feature-dependency-matrix.md](feature-dependency-matrix.md) | To select a supported isolated feature profile|
| [all-input-integration-test.md](all-input-integration-test.md) | Default hardware check after changes to a shipped configuration, driver, router, or function module|
| [usb-reconnect-reliability-test.md](usb-reconnect-reliability-test.md) | After startup, UF2, USB, or power-sequencing changes|
| [startup-ati-recovery-test.md](startup-ati-recovery-test.md) | After IQS9150 initialization or ATI changes|
| [idle-resume-recovery.md](idle-resume-recovery.md) | Diagnose idle/resume failures and validate reset/RDY recovery |
| [inertia-cancel-recovery.md](inertia-cancel-recovery.md) | Validate repeated scroll/glide interruption and recovery |
| [architecture-and-tuning.md](architecture-and-tuning.md) | To decide ownership and tuning location before implementation|
| [reusable-modules.md](reusable-modules.md) | Import the driver/basic functions into another keyboard, with build fixtures and regression checks|
| [two-finger-interaction-flow.md](two-finger-interaction-flow.md) | IC-independent tap/drag/scroll/Tip-Tap flow and porting contract|
| [contact-arbiter-solution-review-prompt.md](contact-arbiter-solution-review-prompt.md) | Hand-off to an independent architecture review after a verified milestone|
| [two-finger-scroll-tuning.md](two-finger-scroll-tuning.md) | To change only the user preference for 2F scroll direction|

## Isolated regression checks

These are diagnostic recipes for separately built, feature-limited UF2 files.
They are not the default release candidate and are used only to localize an
integration failure. The full integration test remains the release gate.

| Area| Document|
|---|---|
| Pointer baseline| [pointer-isolated-test.md](pointer-isolated-test.md) |
| Router hand-off| [router-pointer-isolated-test.md](router-pointer-isolated-test.md) |
| Pinch/rotation × scroll/tap / pinch・rotation × scroll/tap | [transform-gesture-arbitration-test.md](transform-gesture-arbitration-test.md) |
| Tap and drag| [tap-drag-isolated-test.md](tap-drag-isolated-test.md) |
| 3–5F gestures| [multifinger-gesture-integration-test.md](multifinger-gesture-integration-test.md) |
| Edge circle continuation| [edge-dial-circle-isolated-test.md](edge-dial-circle-isolated-test.md) |
| Edge × pointer| [edge-pointer-arbitration-test.md](edge-pointer-arbitration-test.md) |
| Edge × tap/drag| [edge-tap-drag-arbitration-test.md](edge-tap-drag-arbitration-test.md) |
| Archived edge-hold detection-only stage| [edge-hold-continue-detection-test.md](edge-hold-continue-detection-test.md) |
| Edge-hold drag continuation| [edge-hold-drag-continuation-test.md](edge-hold-drag-continuation-test.md) |
| Edge-hold 2F scroll continuation| [edge-hold-scroll-continuation-test.md](edge-hold-scroll-continuation-test.md) |
| 2F scroll inertia| [two-finger-scroll-inertia-test.md](two-finger-scroll-inertia-test.md) |
| Arbiter lifecycle and leases / Arbiter lifecycle・lease | [arbiter-lifecycle-lease-test.md](arbiter-lifecycle-lease-test.md) |
| Arbiter safety hardening| [arbiter-safety-hardening-test.md](arbiter-safety-hardening-test.md) |
| Arbiter output broker / Arbiter output broker | [arbiter-output-broker-test.md](arbiter-output-broker-test.md) |
| Arbiter complete-frame routing| [arbiter-complete-frame-routing-test.md](arbiter-complete-frame-routing-test.md) |
| Primary decision and observer boundary| [arbiter-observer-boundary-test.md](arbiter-observer-boundary-test.md) |
| Terminal tap/double-click grant / Terminal tap・double-click grant | [arbiter-terminal-action-test.md](arbiter-terminal-action-test.md) |
| Primary virtual-KSCAN broker| [arbiter-kscan-output-broker-test.md](arbiter-kscan-output-broker-test.md) |
| Terminal button ledger / Terminal button ledger | [arbiter-button-ledger-test.md](arbiter-button-ledger-test.md) |
| Optional grid adapter| [grid-matrix-isolated-test.md](grid-matrix-isolated-test.md) |
| Earlier base integration| [basic-input-integration-test.md](basic-input-integration-test.md) |

The grid document remains useful even though the grid module is disabled by
default: it is the acceptance procedure to use when that optional feature is
enabled later.

## Release evidence

A release candidate passes only when the structural script, clean firmware
build, and full input integration test all pass. Keep the serial log only for
failed operations or unexpected warnings; the integration document lists the
required evidence patterns.

## 日本語

# テスト手順一覧

これらの文書は回帰テスト資産として保持します。過去に不具合または回帰した相互作用を、
実機で再現確認するための手順です。通常は全入力結合テストから実施し、不具合が出た時だけ
原因の切り分けとして単体テストを実施します。

## 通常のリリース確認

|文書 |使用タイミング |
|---|---|
| [`scripts/verify-contact-arbiter-architecture.sh`](../scripts/verify-contact-arbiter-architecture.sh) |リリースビルド前に実行し、単一所有権のソース不変条件を検査 |
| [`scripts/verify-contact-feature-dependencies.sh`](../scripts/verify-contact-feature-dependencies.sh) |機能限定の診断UF2作成前に実行し、対応するmodule依存を検査 |
| [feature-dependency-matrix.md](feature-dependency-matrix.md) |対応する単体機能profileを選ぶ時 |
| [all-input-integration-test.md](all-input-integration-test.md) |出荷対象の設定、ドライバ、ルーター、機能モジュール変更後の標準実機確認 |
| [usb-reconnect-reliability-test.md](usb-reconnect-reliability-test.md) |起動、UF2、USB、電源シーケンス変更後 |
| [startup-ati-recovery-test.md](startup-ati-recovery-test.md) |IQS9150初期化またはATI変更後 |
| [idle-resume-recovery.md](idle-resume-recovery.md) | 放置後の停止の原因とreset/RDY復旧を検証する時 |
| [inertia-cancel-recovery.md](inertia-cancel-recovery.md) | 慣性の連続操作・中断・再開を検証する時 |
| [architecture-and-tuning.md](architecture-and-tuning.md) |実装前に責務と調整場所を確認する時 |
| [reusable-modules.md](reusable-modules.md) |別キーボードへdriver・基本機能を取り込む手順、build例、回帰確認 |
| [two-finger-interaction-flow.md](two-finger-interaction-flow.md) |別IC向けtap・drag・scroll・Tip-Tap処理と移植契約 |
| [contact-arbiter-solution-review-prompt.md](contact-arbiter-solution-review-prompt.md) |実機確認済み節目の後、独立したアーキテクチャレビューへ引き継ぐ時 |
| [two-finger-scroll-tuning.md](two-finger-scroll-tuning.md) |2本指スクロール方向だけを変更する時 |

## 単体回帰確認

これらは機能を限定して別途ビルドしたUF2向けの診断手順である。通常のリリース候補ではなく、
結合試験の失敗を切り分ける時だけ使用する。リリース判定は全入力結合試験を正とする。

|対象 |文書 |
|---|---|
|ポインタ基本動作 | [pointer-isolated-test.md](pointer-isolated-test.md) |
|ルーター受け渡し | [router-pointer-isolated-test.md](router-pointer-isolated-test.md) |
| Pinch/rotation × scroll/tap / pinch・rotation × scroll/tap | [transform-gesture-arbitration-test.md](transform-gesture-arbitration-test.md) |
|タップ・ドラッグ | [tap-drag-isolated-test.md](tap-drag-isolated-test.md) |
|3〜5本指ジェスチャー | [multifinger-gesture-integration-test.md](multifinger-gesture-integration-test.md) |
|エッジ円継続 | [edge-dial-circle-isolated-test.md](edge-dial-circle-isolated-test.md) |
|エッジ×ポインタ | [edge-pointer-arbitration-test.md](edge-pointer-arbitration-test.md) |
|エッジ×タップ・ドラッグ | [edge-tap-drag-arbitration-test.md](edge-tap-drag-arbitration-test.md) |
|旧エッジホールド検出専用段階 | [edge-hold-continue-detection-test.md](edge-hold-continue-detection-test.md) |
|エッジホールド・ドラッグ継続 | [edge-hold-drag-continuation-test.md](edge-hold-drag-continuation-test.md) |
|エッジホールド・2本指スクロール継続 | [edge-hold-scroll-continuation-test.md](edge-hold-scroll-continuation-test.md) |
|2本指スクロール慣性 | [two-finger-scroll-inertia-test.md](two-finger-scroll-inertia-test.md) |
| Arbiter lifecycle and leases / Arbiter lifecycle・lease | [arbiter-lifecycle-lease-test.md](arbiter-lifecycle-lease-test.md) |
|Arbiter安全性強化 | [arbiter-safety-hardening-test.md](arbiter-safety-hardening-test.md) |
| Arbiter output broker / Arbiter output broker | [arbiter-output-broker-test.md](arbiter-output-broker-test.md) |
|Arbiter完全frame routing | [arbiter-complete-frame-routing-test.md](arbiter-complete-frame-routing-test.md) |
|Primary裁定とobserver境界 | [arbiter-observer-boundary-test.md](arbiter-observer-boundary-test.md) |
| Terminal tap/double-click grant / Terminal tap・double-click grant | [arbiter-terminal-action-test.md](arbiter-terminal-action-test.md) |
|Primary仮想KSCAN broker | [arbiter-kscan-output-broker-test.md](arbiter-kscan-output-broker-test.md) |
| Terminal button ledger / Terminal button ledger | [arbiter-button-ledger-test.md](arbiter-button-ledger-test.md) |
|任意格子アダプタ | [grid-matrix-isolated-test.md](grid-matrix-isolated-test.md) |
|旧基本結合確認 | [basic-input-integration-test.md](basic-input-integration-test.md) |

格子文書は現在の既定設定で格子モジュールが無効でも残します。将来この任意機能を有効化
する際の受入手順だからです。

## リリース判定

リリース候補は、構造script、クリーンなファームビルド、全入力結合試験のすべてに合格して
初めて合格とする。シリアルログは失敗操作または想定外warningがある場合に保存する。必要な
ログの判定パターンは全入力結合試験に記載する。
