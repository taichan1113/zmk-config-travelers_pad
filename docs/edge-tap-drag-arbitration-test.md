# Edge dial and tap/drag arbitration test / エッジダイヤルとタップ/ドラッグ排他試験

## Scope / 対象

This test enables the pointer, router, edge dial, and tap/drag function together.
The edge dial keeps its letter bindings (`D/U/R/I/T/Y/B/N`) so its output is
distinct from mouse buttons.

この試験ではポインタ、router、エッジダイヤル、tap/drag機能を同時に有効化する。
エッジダイヤルはマウスボタンと区別できるよう、従来どおり
`D/U/R/I/T/Y/B/N`を出力する。

## Procedure / 手順

1. Flash `travelers-pad-edge-tap-drag-arbitration.uf2` and wait for
   `STEP1B complete: Event Mode active`.
   `travelers-pad-edge-tap-drag-arbitration.uf2`を書き込み、
   `STEP1B complete: Event Mode active`を待つ。
2. In the centre, perform one-, two-, and three-finger stationary taps. Confirm
   left, right, and middle click respectively.
   中央で1本、2本、3本指の静止タップを行い、順に左、右、中クリックとなることを確認する。
3. In the centre, double-tap, hold the second contact, then move. Confirm the
   matching button is held before the drag movement, and released on lift.
   中央でダブルタップ後に2回目をホールドして動かす。移動前に対応ボタンが押され、
   指を離すと解放されることを確認する。
4. Trace each enabled edge and, where applicable, continue into its local circle.
   Confirm only its letter repeats: no `Mouse buttons set` log and no drag.
   有効辺をなぞり、該当する局所円へ継続する。対応文字だけが反復し、
   `Mouse buttons set`ログやドラッグがないことを確認する。
5. Immediately after lifting an edge contact, repeat a centre one-finger tap.
   Confirm exactly one left click.
   エッジ接触を離した直後に、中央で1本指タップを行う。左クリックが1回だけ出ることを確認する。

## Evidence / 判定ログ

Normal tap/drag evidence includes `tap released`, `single tap confirmed`,
`double-tap drag started`, and `drag dropped`. Edge operation must show
`edge dial mode=` and must not show any of those tap/drag lines for that same
contact.

通常のタップ/ドラッグでは`tap released`、`single tap confirmed`、
`double-tap drag started`、`drag dropped`を確認する。エッジ操作では
`edge dial mode=`が出て、同一接触に対するこれらのtap/dragログは出ないことを確認する。
