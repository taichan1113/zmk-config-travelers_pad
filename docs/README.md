# Test guide index / テスト手順一覧

Keep these documents as regression-test assets. They record reproducible
hardware checks for interactions that have previously failed or regressed.
The normal workflow uses the full integration test first; isolated tests are
used only when an integration failure needs to be localized.

これらの文書は回帰テスト資産として保持します。過去に不具合または回帰した相互作用を、
実機で再現確認するための手順です。通常は全入力結合テストから実施し、不具合が出た時だけ
原因の切り分けとして単体テストを実施します。

## Normal release check / 通常のリリース確認

| Document / 文書 | Use it when / 使用タイミング |
|---|---|
| [all-input-integration-test.md](all-input-integration-test.md) | Default check after changes to a shipped configuration, driver, router, or function module / 出荷対象の設定、ドライバ、ルーター、機能モジュール変更後の標準確認 |
| [usb-reconnect-reliability-test.md](usb-reconnect-reliability-test.md) | After startup, UF2, USB, or power-sequencing changes / 起動、UF2、USB、電源シーケンス変更後 |
| [architecture-and-tuning.md](architecture-and-tuning.md) | To decide ownership and tuning location before implementation / 実装前に責務と調整場所を確認する時 |
| [two-finger-scroll-tuning.md](two-finger-scroll-tuning.md) | To change only the user preference for 2F scroll direction / 2本指スクロール方向だけを変更する時 |

## Isolated regression checks / 単体回帰確認

| Area / 対象 | Document / 文書 |
|---|---|
| Pointer baseline / ポインタ基本動作 | [pointer-isolated-test.md](pointer-isolated-test.md) |
| Router hand-off / ルーター受け渡し | [router-pointer-isolated-test.md](router-pointer-isolated-test.md) |
| Tap and drag / タップ・ドラッグ | [tap-drag-isolated-test.md](tap-drag-isolated-test.md) |
| 3–5F gestures / 3〜5本指ジェスチャー | [multifinger-gesture-integration-test.md](multifinger-gesture-integration-test.md) |
| Edge circle continuation / エッジ円継続 | [edge-dial-circle-isolated-test.md](edge-dial-circle-isolated-test.md) |
| Edge × pointer / エッジ×ポインタ | [edge-pointer-arbitration-test.md](edge-pointer-arbitration-test.md) |
| Edge × tap/drag / エッジ×タップ・ドラッグ | [edge-tap-drag-arbitration-test.md](edge-tap-drag-arbitration-test.md) |
| Edge-hold continuation detection / エッジホールド継続の検出 | [edge-hold-continue-detection-test.md](edge-hold-continue-detection-test.md) |
| Edge-hold drag continuation / エッジホールド・ドラッグ継続 | [edge-hold-drag-continuation-test.md](edge-hold-drag-continuation-test.md) |
| Edge-hold 2F scroll continuation / エッジホールド・2本指スクロール継続 | [edge-hold-scroll-continuation-test.md](edge-hold-scroll-continuation-test.md) |
| 2F scroll inertia / 2本指スクロール慣性 | [two-finger-scroll-inertia-test.md](two-finger-scroll-inertia-test.md) |
| Optional grid adapter / 任意格子アダプタ | [grid-matrix-isolated-test.md](grid-matrix-isolated-test.md) |
| Earlier base integration / 旧基本結合確認 | [basic-input-integration-test.md](basic-input-integration-test.md) |

The grid document remains useful even though the grid module is disabled by
default: it is the acceptance procedure to use when that optional feature is
enabled later.

格子文書は現在の既定設定で格子モジュールが無効でも残します。将来この任意機能を有効化
する際の受入手順だからです。
