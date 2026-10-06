## English

# Primary decision and observer boundary test

## Scope

The arbiter now resolves a frame when every enabled **primary** recognizer has
observed it. `scroll_inertia` and `edge_hold_continue` remain registered frame
observers so they can read the immutable lifecycle snapshot, but they do not
delay owner selection or router contact forwarding.

An observer handler runs only after both conditions hold: the five-primary
decision has resolved, and that observer has consumed the same raw frame. This
lets edge hold use complete slot coordinates and lets inertia classify only the
final owner of a normal release.

## Procedure

1. **Normal 1F pointer
   - Move one finger in the centre for several seconds, lift, then repeat.
   - Expected: ordinary cursor movement begins immediately and does not stall.

2. **Scroll plus inertia
   - Make a clear two-finger vertical scroll, lift both contacts, then touch
     the pad while the glide is still running.
   - Expected: normal scroll starts without delay; inertia starts only after a
     normal release and stops immediately on the new touch.

3. **Edge-hold continuation
   - Start a committed two-finger scroll, reach its forward matching edge with
     either finger, hold past the configured delay, and release.
   - Expected: continuation begins only after the scroll has committed, and
     stops on release. It does not delay the initial scroll classification.

4. **No observer cross-talk
   - After either test above, immediately make a centre 1F pointer move and a
     stationary 2F right-click.
   - Expected: neither inertia nor edge continuation begins; the pointer and
     right-click remain immediate.

## Expected log and failure criteria

Boot logs should identify recognizers 0–4 as `arbiter primary recognizer=...`
and IDs 5–6 as `arbiter observer=...`. Normal use must not contain `arbiter
frame queue overflow`, `arbiter output rejected`, or unexpected `session=...
cancelled`. A stall in pointer/scroll output while edge hold or inertia is idle
is a failure.

## 日本語

# Primary裁定とobserver境界試験

## 対象

arbiterは、有効な**primary** recognizerすべてがframeを観測した時点で裁定するようになった。
`scroll_inertia` と `edge_hold_continue` は不変lifecycle snapshotを読む登録済みframe observerの
ままだが、owner選択およびrouterの接触frame forwardingを遅らせない。

observer handlerは、6 primaryの裁定完了と、そのobserver自身による同じ生frameの消費という二条件が
そろった後だけに実行される。これによりedge holdは完全なslot座標を使い、inertiaは通常releaseの
最終ownerだけを分類できる。

## 手順

1. 通常1本指ポインタ**
1. - 中央で1本指ポインタを数秒動かして離し、もう一度繰り返す。
   - 期待: 通常のカーソル移動が直ちに始まり、停止しない。

2. scrollと慣性**
2. - 明確な2本指縦scrollを行い、両指を離した後、慣性が動いている間に再接触する。
   - 期待: 通常scrollは遅延なく開始する。慣性は通常release後だけ開始し、新規接触で直ちに止まる。

3. エッジホールド継続**
3. - 確定した2本指scrollを開始し、いずれかの指を進行方向に対応する辺へ到達させ、設定delayを
     越えて保持してから離す。
   - 期待: 継続はscroll確定後だけに開始し、解放で止まる。初回scroll分類を遅らせない。

4. observer相互干渉なし**
4. - 上記のいずれかの直後に、中央で1本指ポインタ移動、続いて静止2本指右クリックを行う。
   - 期待: inertiaもedge continuationも開始せず、ポインタと右クリックは即時に動作する。

## 期待ログと不合格基準

起動ログではrecognizer 0〜4が `arbiter primary recognizer=...`、ID 5〜6が
`arbiter observer=...` と表示される。通常操作で `arbiter frame queue overflow`、
`arbiter output rejected`、想定外の `session=... cancelled` が出れば不合格です。edge hold
またはinertiaがidle中にpointer/scroll出力が停止する場合も不合格です。
