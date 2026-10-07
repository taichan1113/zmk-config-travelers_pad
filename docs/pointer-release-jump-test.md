# Pointer release jump regression

[English](#english) | [日本語](#日本語)

## English

The observed log shows session end followed by relative movement -1334/-369
and -1623/-430. These are firmware-generated deltas, not just host-side jitter.
They are consistent with subtracting a previous coordinate from the normalized
`65535` invalid-coordinate sentinel and narrowing to signed 16-bit. The supplied
log does not include raw coordinates, so the exact originating sequence remains
unconfirmed.

The pointer adapter now rejects invalid coordinates, including negative and
out-of-range incoming values, before difference calculation. It also requires
X/Y/contact-count fields in a complete routed frame. Invalid/incomplete frames
discard the motion baseline; the next valid frame only rebases. Zero-contact
frames always clear pointer/scroll state. Transient invalid frames preserve an
owned scroll session, so they do not turn scrolling into pointer motion.
The sensor driver and drag tracking path are unchanged. Terminal click pacing
remains 5 ms down and 5 ms up. Three-finger middle click is user-confirmed; other
click tests are still pending.

Run `bash scripts/test-pointer-release.sh [features-checkout]` and
`bash scripts/test-click-spacing.sh [features-checkout]`.
Both extract production code. Pointer tests cover 100 invalid/release/recontact
cycles, separate invalid axes, unexpected sync/partial frames, pointer-to-scroll
transition, partial release, and routed cancellation.

### Hardware check

Flash `travelers-pad-pointer-release-guard-20261007.uf2` from Windows Downloads.
Use the host/environment where the jump occurred.

1. Move one finger in circles, lift, and place it elsewhere; repeat 30 times.
   The cursor must not jump on lift or first contact.
2. Move rapidly and lift; repeat 20 times. Repeat near pad edges.
3. Confirm two-finger scrolling/inertia and tap dragging still work.
4. Confirm three-finger middle click remains reliable. Ubuntu double-click tests
   from the separate click-spacing procedure remain pending.

If a jump recurs, submit logs around that instant. `invalid pointer coordinates
discarded` and `incomplete pointer frame discarded` indicate rejected input;
they must not be followed by sentinel-derived large relative motion. Their
presence helps locate the upstream problem; rejecting input does not prove its
origin. Hardware acceptance is pending; no main merge or push is performed.

## 日本語

観測ログにはsession終了直後に移動量 -1334/-369 と -1623/-430 が出ています。
これはホスト側のカクツキだけではなく、ファームが生成した移動です。
無効座標 `65535` から直前の座標を引き、符号付き16bitへ変換した場合と整合します。
ただし生座標がログにないため、実際にその値が入った経路は未確定です。

pointer adapterで差分計算前に無効座標を拒否し、負数・入力範囲外も無効扱いにします。
また、routed frameのX/Y/指本数が揃っていることを確認します。
無効・不完全frameでは移動基準を捨て、次の有効frameは基準の再設定だけに使います。
指本数0のframeではpointer/scroll状態を必ず解除します。一時的な無効frameでは
確定scroll状態を維持し、スクロールからポインタ移動へ切り替わらないようにします。
センサードライバ・ドラッグ追跡経路は変更していません。タップclickの押下5ms・解放
間隔5msは維持します。3本指の中クリックはユーザー実機確認済み、それ以外のclick
試験は未確認です。

`bash scripts/test-pointer-release.sh [features-checkout]` と
`bash scripts/test-click-spacing.sh [features-checkout]` で実コードの抽出試験を実行します。
pointer試験は無効座標・解放・再接触100回、片軸だけ無効な場合、不完全frame、
pointer→scroll遷移、部分解放、routed取消しを含みます。

### 実機確認

Windows Downloadsの `travelers-pad-pointer-release-guard-20261007.uf2` を書き込み、
ジャンプが起きた接続先・環境で確認してください。

1. 1本指で円を描く→離す→別位置に置く、を30回。離した瞬間と再接触時に
   ポインタが飛ばないこと。
2. 速く動かして離す操作を20回。パッド端付近でも確認。
3. 2本指スクロール・慣性とタップドラッグが従来どおり動くこと。
4. 3本指中クリックも継続して動くこと。Ubuntuのダブルクリック試験は
   別のclick-spacing手順に従い、まだ未確認として扱います。

再発した場合は瞬間の前後ログを提出してください。
`invalid pointer coordinates discarded` と `incomplete pointer frame discarded` は
入力を拒否した診断です。その後に無効座標由来の大きな移動が出ないことを確認します。
診断は上流の原因追跡に役立ちますが、入力を拒否しただけで原因まで確定するわけでは
ありません。実機合格は未判定で、main統合・pushは行いません。
