## English

# Archived: Edge-hold continuation detection test

> Historical, detection-only procedure. The release firmware now emits active
> drag and two-finger-scroll continuation. Use
> [all-input-integration-test.md](all-input-integration-test.md) for current
> release validation; retain this document only when intentionally restoring a
> detection-only profile.
>

This stage verifies only the activation boundary. It sends no additional mouse
movement or wheel reports. The expected new logs begin with
`travelers_pad_edge_hold`.

## Procedure

1. Flash the detection UF2 and wait for `STEP1B complete`. Confirm the startup
   line `edge hold detector active: ... output disabled`.

2. Move one finger from the centre to every edge and hold it briefly. No
   `continuation eligible` line may appear. Start a normal edge dial from an
   enabled edge too; it must retain its existing behavior and must not produce
   an edge-hold line.

3. Start a one-finger double-tap-and-hold drag in the centre. While the button
   remains held, drag its contact into an edge zone and keep holding for one
   second. Exactly one `drag continuation eligible edge=...` line is expected.
   Move back to the centre: one matching `drag continuation disarmed` line is
   expected. Releasing the finger must end the drag normally. No extra pointer
   movement is expected in this detection-only stage.

4. Begin a two-finger scroll from the centre, then move the representative
   first finger into an edge zone while both fingers remain down. One
   `scroll continuation eligible edge=...` line is expected only after
   `2F route=scroll committed`. Move away from the edge and expect the matching
   `scroll continuation disarmed` line. Lifting either/both fingers must
   disarm it; there is no repeated scroll yet.

5. Repeat with a 2F stationary tap, pinch/rotation, and 3F swipe. None may
   produce either `drag continuation eligible` or `scroll continuation eligible`.

## Pass criteria

- Only confirmed drag and router-committed 2F scroll can arm the detector.
- Entering/leaving a boundary generates one transition log each.
- Existing pointer, drag, scroll, edge dial, tap, and multi-finger behavior do
  not change.

## 日本語

# 旧: エッジホールド継続・検出試験

> これは過去の検出専用手順である。現在のリリースファームはドラッグと2本指スクロールの
> 継続出力を有効にする。通常のリリース確認は
> [all-input-integration-test.md](all-input-integration-test.md) を使い、意図的に検出専用
> profileへ戻す場合だけ本書を参照する。

この段階では有効化境界だけを検証する。追加のマウス移動またはホイールレポートは
送信しない。新しい期待ログの先頭は `travelers_pad_edge_hold` である。

## 手順

1. 検出用UF2を書き込み、`STEP1B complete` を待つ。起動時に
   `edge hold detector active: ... output disabled` が出ることを確認する。

2. 中央から1本指を各辺へ移動して短く保持する。`continuation eligible` は出ない。
   有効辺から通常のエッジダイヤルも開始し、従来どおり動作し、edge-hold行が出ない。

3. 中央で1本指ダブルタップ＋ホールドを開始し、ボタンを保持したまま辺へ移動して
   1秒保持する。`drag continuation eligible edge=...` が1回だけ出る。中央へ戻すと
   対応する `drag continuation disarmed` が1回出る。指を離すと通常どおりドロップし、
   検出専用段階なので追加ポインタ移動は出ない。

4. 中央で2本指スクロールを開始し、両指を離さず代表の第1指を辺へ移動する。
   `2F route=scroll committed` の後に限り、`scroll continuation eligible edge=...`
   が1回出る。辺から離すと対応する `scroll continuation disarmed` が出る。
   いずれかまたは両方の指を離すと解除される。この段階では繰返しスクロールはない。

5. 2本指静止タップ、ピンチ／回転、3本指スワイプでも繰り返す。いずれも
   `drag continuation eligible` と `scroll continuation eligible` を出してはならない。

## 合格基準

- 確定ドラッグとrouter確定済み2本指スクロールだけが検出対象となる。
- 辺への進入・離脱ごとに遷移ログが1回だけ出る。
- 既存のポインタ、ドラッグ、スクロール、エッジダイヤル、タップ、多指動作が変化しない。
