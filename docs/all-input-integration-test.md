# Full input integration test / 全入力結合試験

## Scope / 対象

This is the release-level hardware regression for the normal pointer path,
two-finger scroll and inertia, one-to-three-finger tap/drag, Tip-Tap,
three-to-five-finger gesture KSCAN, edge dial, and edge-hold continuation.
The optional grid module remains disabled because no user keymap selects it.

この試験は、通常ポインタ、2本指スクロールと慣性、1〜3本指タップ/ドラッグ、Tip-Tap、
3〜5本指ジェスチャーKSCAN、エッジダイヤル、エッジホールド継続を対象とするリリースレベルの
実機回帰試験である。任意格子モジュールは利用者用キーマップが未選択のため、無効のままとする。

## Automated preflight / 自動事前確認

Run the structural check from the repository root before building. It verifies
the single raw subscriber, absence of retired shared-claim APIs, primary token
gates, and observer-only auxiliary paths.

ビルド前にリポジトリルートで構造検査を実行する。唯一の生入力購読者、廃止済み
shared-claim APIが無いこと、primary token gate、observerだけの補助経路を検査する。

```sh
./scripts/verify-contact-arbiter-architecture.sh
```

Then build the release UF2 as documented in the project README. A successful
build and this script are necessary but do not replace the physical checks
below.

続いてプロジェクトREADME記載の手順でリリースUF2をビルドする。ビルド成功と本scriptは
必要条件だが、以下の実機確認を置き換えない。

## Essential checks / 重要確認項目

1. After flashing, confirm the IQS9150 reaches `STEP1B complete: Event Mode
   active`. Move one finger and make 1F/2F/3F stationary taps. Confirm pointer,
   left click, right click, and middle click respectively, without scroll.
   書込み後に `STEP1B complete: Event Mode active` を確認する。1本指ポインタ、
   1/2/3本指の静止タップを行い、順に左/右/中クリックとなり、スクロールが混入しないことを
   確認する。
2. For each of 1F, 2F, and 3F, double tap, keep the second contact down, then
   move. Confirm button-down starts only after meaningful movement, movement
   begins from the current position, and one release is sent when lifted.
   1/2/3本指それぞれで、ダブルタップ後に2回目を保持して動かす。有意な移動後だけ
   ボタン押下となり、現在位置から移動し、離した時に一回だけ解放されることを確認する。
3. Scroll vertically and horizontally with two fingers, including a pause and
   restart. No pointer movement or right click may leak. Lift to confirm a
   short decaying inertia motion, then touch again to stop it immediately.
   2本指で縦・横スクロールし、途中停止・再開も行う。ポインタ移動や右クリックが混入
   しないことを確認する。離して短い減衰慣性を確認し、再接触ですぐ停止することを確認する。
4. During a confirmed drag, reach the matching edge and hold; then repeat with
   a committed axis-locked scroll where either forward finger reaches the
   matching edge. Confirm continuation only in the current direction, and
   immediate stop on release.
   確定ドラッグで対応する辺へ到達して保持し、続いて軸ロック済みスクロール中に進行方向の
   辺へいずれかの指を到達させる。現在方向だけ継続し、解放で即時停止することを確認する。
5. Trace all four edges and their local circular continuation. Confirm only the
   configured repeated host action: left brightness, right vertical scroll, top
   volume, and bottom horizontal scroll. After lift, a central 1F tap must still
   produce exactly one left click.
   4辺と局所円継続をなぞる。左は輝度、右は縦スクロール、上は音量、下は横スクロールだけが
   反復することを確認する。離した直後の中央1本指タップは左クリック一回でなければならない。
6. Test the active virtual-key features: Tip-Tap left/right (`F13`/`F14` in the
   supplied keymap), 3F and 4F all directions, and 5F Grasp/Release. Confirm
   the configured host action and no pointer, scroll, or click leakage.
   有効な仮想キー機能を試す。Tip-Tap左右（付属keymapでは `F13`/`F14`）、3本/4本指の
   全方向、5本指Grasp/Releaseを行う。設定済み動作だけが起こり、ポインタ、スクロール、
   クリックが混入しないことを確認する。

7. Perform pinch-in/out and clockwise/counter-clockwise rotation, then release
   and resume ordinary pointer/scroll. Confirm only the transform actions occur,
   including repeated steps, with no wheel or terminal right-click leakage.
   ピンチ内/外、時計/反時計回りを行い、離して通常pointer/scrollへ戻る。
   継続stepを含むtransform actionだけが出て、wheel・解放時右clickが混入しないことを確認する。
   Detailed procedure / 詳細: [transform-gesture-arbitration-test.md](transform-gesture-arbitration-test.md).

## Expected ownership / 期待する所有権

| Operation / 操作 | Expected owner and output / 期待するownerと出力 |
| --- | --- |
| 1F pointer / 1本指ポインタ | Router pointer path → pointer HID / routerポインタ経路 → pointer HID |
| 1–3F tap / 1〜3本指タップ | Terminal action grant → mouse HID / terminal action grant → mouse HID |
| 1–3F drag / 1〜3本指ドラッグ | `drag` token → mouse HID / `drag` token → mouse HID |
| 2F axis-locked scroll / 2本指軸ロックスクロール | `scroll` token → pointer wheel → listener processors / `scroll` token → pointer wheel → listener processor |
| 3–5F gesture / 3〜5本指ジェスチャー | `gesture` token → KSCAN → keymap / `gesture` token → KSCAN → keymap |
| Edge dial / エッジダイヤル | `edge-dial` token → KSCAN → keymap / `edge-dial` token → KSCAN → keymap |
| Tip-Tap / Tip-Tap | `tip-tap` token → KSCAN → keymap / `tip-tap` token → KSCAN → keymap |
| Pinch/rotation / ピンチ・回転 | `transform` token → KSCAN → keymap / `transform` token → KSCAN → keymap |
| Edge hold / エッジホールド | Observer output delegated by live `drag` or `scroll` token / liveな `drag` または `scroll` tokenからobserver出力を委譲 |

## Evidence and failure rule / 判定ログと失敗基準

Save a serial log covering every failed operation. The expected ownership lines
are `arbiter session=... winner=drag|scroll|gesture|edge-dial|tip-tap|transform`. A test
fails if it contains `arbiter output rejected`, `arbiter frame queue overflow`,
`proposal ... rejected`, an unexpected `session=... cancelled`, or a contact
report failure. The known QSPI JEDEC warning is outside this trackpad test.

失敗した操作ごとにシリアルログを保存する。期待する所有権ログは
`arbiter session=... winner=drag|scroll|gesture|edge-dial|tip-tap|transform` である。
`arbiter output rejected`、`arbiter frame queue overflow`、`proposal ... rejected`、
意図しない `session=... cancelled`、接触レポート失敗があれば不合格とする。既知のQSPI
JEDEC warningは本トラックパッド試験の対象外である。
