## English

# Edge-start-only integration check

## Scope

Only edge-dial acquisition changes. The first contact frame of a session must
contain one finger on an enabled edge. Interior, multi-finger, unavailable-coordinate,
and disabled-edge starts cannot acquire an edge later in the same session.
Temporary finger-count changes do not reset eligibility; reacquisition is restricted
to the original edge. All-finger release or cancellation clears the decision.

Edge widths, circle geometry, behaviors, tap/drag, two-finger scroll, edge-hold
continuations, and inertia settings are unchanged.

## Firmware

UF2: `C:\Users\takec\Downloads\travelers-pad-edge-start-only-20261005.uf2`
Build

## Hardware checks

1. Start one finger well inside the pad, then move to each edge without lifting.
   The pointer must continue moving; no brightness, volume, or edge-wheel action.
2. Lift, then start on an edge and trace it. Verify the existing action, and try
   circular continuation once. No pointer leakage at edge acquisition.
3. Start inside, briefly add/remove a second finger, then reach an edge with the
   original finger still down. It must not acquire the edge. Lift all fingers,
   then an edge start must work again.
4. Quick regressions: 1F tap/double-tap/drag; 2F scroll and inertia; drag/scroll
   edge-hold continuation. These remain the existing operations, independent
   of edge-dial acquisition.

For checks 1 and 3, `edge dial acquired` or edge-dial action logs must not appear
for the interior-start session. Check 2 should emit acquisition/action logs.

## Next step

Hardware checks 1, 2 and 4 passed. In check 3, pointer suppression after a second
contact persists until all fingers lift by existing router policy; no edge takeover
was reported. This stage was accepted and merged into main (2026-10-05).
（2026-10-05）。

## 日本語

# 接触開始時限定エッジの結合確認

## 変更範囲

変更はedge dial取得条件のみ。sessionの最初の接触frameが、有効辺上の1本指である
場合だけ取得可能とする。内側、多指、無効座標、無効辺からの開始は、そのsession中に
辺へ移動しても取得しない。一時的な指本数変化では判定をやり直さず、再取得は最初の辺に
限定する。全指解放または取消しで判定を解消する。

辺幅・円形領域・Behavior・tap/drag・2Fスクロール・エッジ保持・慣性の設定は変更しない。

## ファーム

ビルド先: `build/edge-start-only/zephyr/zmk.uf2`

## 実機確認

1. パッド内側から1本指で移動し、離さず4辺へ到達する。ポインタが継続し、輝度・音量・
   エッジスクロールが発火しないこと。境界付近ではなく明確に内側から開始する。
2. 指を離して辺からなぞり始め、従来の割当が発火すること。円継続も1回確認し、
   辺取得時にポインタが動かないこと。
3. 内側開始で2本目を一時的に追加・解放し、1本目を離さず辺へ移動する。辺操作に
   切り替わらないこと。全指解放後は辺開始が再び使えること。
4. 簡単な回帰確認：1Fタップ・ダブルタップ・ドラッグ、2Fスクロール・慣性、
   ドラッグ／スクロールの端保持。これらが従来どおり動くこと。

操作1・3の内側開始sessionでは`edge dial acquired`と辺操作ログが出ないこと。
操作2では取得・操作ログが出ること。シリアルログと操作体感を合わせて判定する。

## 次の段階

実機項目1・2・4は合格。項目3の2本目追加後に全指解放までポインタが戻らない動作は
既存router仕様であり、辺への横取りは報告されていない。本段階は承認されmainへ統合済み
