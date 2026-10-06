## English

# Edge-hold drag continuation test

This stage extends only an already-confirmed double-tap-and-hold drag. When
its centroid remains in an edge zone for 120 ms, it emits a virtual pointer
delta every 24 ms towards that edge. It does not alter button ownership, normal
drag tracking, edge dial, or two-finger scroll continuation.

## Procedure

1. Flash `travelers-pad-edge-hold-drag-continuation.uf2`. After startup, confirm
   `edge hold active: ... drag delay=120ms period=24ms delta=8; scroll output disabled`.

   `edge hold active: ... drag delay=120ms period=24ms delta=8; scroll output disabled`

2. In the pad centre, start a one-finger double-tap-and-hold drag. Move to the
   right edge and keep holding for about one second. After the short delay, the
   pointer must keep moving right while the left button stays held. The log must
   contain one `drag continuation eligible edge=right` followed by `started`.

3. Without releasing, return to the centre. Repeated movement must stop at once,
   while ordinary drag motion and the held button continue. Then release: the
   drop occurs once and no delayed movement may remain.

4. Repeat at left, top, and bottom. The continuation direction must be outward:
   left, up, and down respectively. Test a two- or three-finger drag once too;
   its corresponding right/middle button must remain held while continuation is
   active.

5. Perform ordinary pointer motion, edge dial, and two-finger scrolling into an
   edge. They must retain their prior behavior. Scroll may log `eligible`, but
   it must not repeat automatically in this stage.

## Pass criteria

- Edge-held confirmed drags continue smoothly in each outward direction.
- Leaving the edge or releasing stops continuation with no stale pointer motion.
- Other pointer, click, scroll, and edge-dial paths are unchanged.

## 日本語

# エッジホールド・ドラッグ継続試験

この段階では、確定済みのダブルタップ＋ホールド・ドラッグだけを拡張する。重心が
辺のゾーンに120 ms留まると、その辺の方向へ24 msごとに仮想ポインタ移動を出力する。
ボタンの所有、通常のドラッグ追跡、エッジダイヤル、2本指スクロール継続は変更しない。

## 手順

1. `travelers-pad-edge-hold-drag-continuation.uf2`を書き込む。起動後に
   を確認する。

2. パッド中央で1本指ダブルタップ＋ホールド・ドラッグを開始する。右端まで移動し、
   約1秒保持する。短い遅延後、左ボタンを保持したままポインタが右へ継続移動する。
   ログには`drag continuation eligible edge=right`と続く`started`が各1回出る。

3. 指を離さず中央へ戻す。繰返し移動は直ちに止まり、通常のドラッグ移動とボタン保持は
   続く。その後に離すとドロップは1回だけで、遅延した移動が残ってはならない。

4. 左、上、下でも繰り返す。継続方向は各々、左、上、下の外向きでなければならない。
   2本または3本指ドラッグも1回試す。継続中は対応する右／中ボタンの保持が維持される。

5. 通常のポインタ移動、エッジダイヤル、辺へ向かう2本指スクロールを行う。これらは
   従来どおりでなければならない。スクロールは`eligible`をログ出力してもよいが、
   この段階で自動繰返ししてはならない。

## 合格基準

- 辺で保持した確定ドラッグは、各外向き方向へ滑らかに継続する。
- 辺から離れる／指を離すと、残留ポインタ移動なしに継続が停止する。
- その他のポインタ、クリック、スクロール、エッジダイヤル経路は不変である。
