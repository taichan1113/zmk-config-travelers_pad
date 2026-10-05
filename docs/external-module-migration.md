# External module migration / 外部moduleへの移行

## Target and status / 目標と状態

Travelers Pad uses independent driver and function projects under the ZMK workspace's
`module/`, acquired by west. Both modules are now private GitHub repositories;
the function module is named `zmk-features-trackpad`. User hardware approval
was received on 2026-10-06 for the extraction: behavior is
unchanged from the previous firmware. Local extraction and west integration are complete.

Travelers Padはworkspaceの`module/`にある独立driver・機能projectをwestで取り込む。
両moduleはユーザー指示によりprivate GitHub repoへ保存済み。
機能module名は`zmk-features-trackpad`に確定。
2026-10-06、従来と同様に動作するとのユーザー実機合格を受領。
ローカル切り出しとwest統合は完了。

| Repository / repo | Responsibility / 責務 | Pinned commit / 固定commit |
| --- | --- | --- |
| `module/zmk-driver-iqs9150` | Driver, EV02 init, sensor binding, contact ABI / driver・初期化・センサbinding・接触ABI | `b86127ab125b96dcd222b357ee4cc6893ea4bfdf` |
| `module/zmk-features-trackpad` | All function implementations, arbiter, bindings, shared profile / 全機能実装・arbiter・binding・共通profile | `b9f70c4245759a7eed25803bbecb88ff7d3cfd66` |
| `config/zmk-config-travelers_pad` | Hardware wiring, config, virtual matrix, keymap, diagnostics/tests / 配線・設定・仮想matrix・keymap・診断／試験 | Keyboard configuration / キーボード設定 |

The driver owns `include/travelers_pad/multitouch.h`; the function module consumes
it without a duplicate. The keyboard no longer contains driver/function C files,
headers, bindings, or their Kconfig/build definitions. Removed files are recoverable
from the keyboard's `b0ad83d` checkpoint and now tracked in the external repositories.

共有接触headerはdriverが所有し、機能moduleは複製せず利用する。
キーボードrepoからdriver／機能のCファイル・header・binding・Kconfig/build定義を除去した。
除去したファイルは旧checkpoint `b0ad83d`から復元でき、外部repoでGit管理されている。

## west / 取り込み

`config/west.yml` pins projects from `https://github.com/taichan1113/zmk-driver-iqs9150.git`
and `https://github.com/taichan1113/zmk-features-trackpad.git`. The generated workspace
manifest uses the same names, paths and revisions.

manifestは上記private GitHub URLのcommitを固定する。workspace生成manifestも同じ名前・配置・revisionへ更新済み。

```sh
# From zmk-workspace / workspaceから
nix develop --command west update zmk-driver-iqs9150 zmk-features-trackpad
```

west normally checks out modules in detached HEAD state. Before changing a module,
create a feature branch inside it, commit the changes, then update the keyboard's
manifest revision and regenerate its workspace manifest (`just init`). Updating
only a checkout without changing the pin is not a reproducible integration.

westは通常detached HEADでmoduleをcheckoutする。module改修時はmodule内でbranchを作り、
commit後にキーボードmanifestのrevisionを更新し、`just init`で生成manifestも更新する。
checkoutだけの変更では再現可能な統合にはならない。

Both repositories are PRIVATE. On another PC, authenticate Git for an account
with repository access before running `west update` (for GitHub CLI HTTPS access,
`gh auth login` and `gh auth setup-git`). CI likewise needs read credentials with
access to both projects; a public/anonymous build cannot fetch them. Do not commit
tokens to manifests. Local uncommitted work must still be backed up separately.

両repoはprivate。別PCではアクセス権のあるアカウントでGit認証を設定してから取得する。
GitHub CLIのHTTPS取得では上記authコマンドを利用できる。
CIにも両repoを読める認証情報が必要で、匿名buildでは取得できない。
tokenはmanifestへ記録しない。ローカル未commitの作業は引き続き別途保存する。

## Verification / 検証

- All driver/function implementation and shared profile files were copied byte-for-byte from `b0ad83d`. / 実装と共通profileを旧版からバイト単位で変更せずコピー。
- Fresh full-profile build uses the modules discovered from west, with no local implementation fallback. / west検出したmoduleだけで全機能をクリーンbuild。
- Generated DTS is byte-identical to the baseline. Sorted CONFIG differs only in new module registration flags. / 生成DTSは旧版と完全一致、CONFIG差分は新module登録flagだけ。
- Driver-only and basic pointer builds, structural checks and host inertia math tests pass. / driver単独・基本pointerのbuild、構造検査、host慣性数学試験は合格。
- A separate temporary west workspace successfully cloned both pinned local projects. / 別の一時west workspaceでも固定projectを取得できた。

Firmware UF2 / ファーム：
`C:\Users\takec\Downloads\travelers-pad-external-modules-20261006.uf2`

### Hardware check / 実機確認

1. Flash the UF2, wait for initialization, then check pointing, 1/2/3F taps and immediate double-tap drag. / UF2書き込み・初期化後、ポインタ、1/2/3Fタップ、すぐ動かすダブルタップドラッグを確認。
2. Check vertical/horizontal 2F scroll, inertia on release and stop on retouch. / 縦横2Fスクロール、離した後の慣性、再接触停止を確認。
3. Check all enabled edges/circular continuation, Tip-Tap, a 3F/4F swipe, 5F Grasp/Release and pinch. Rotation remains disabled. / 有効辺・円継続、Tip-Tap、3/4Fスワイプ、5F Grasp/Release、ピンチを確認。回転は無効のまま。
4. Disconnect/reconnect USB once and verify recovery. / USBを一度抜き差しして復帰を確認。

No fine retuning is required: the criterion is unchanged behavior from the validated
firmware. User approval has now satisfied this integration gate. Keep this procedure
for future regressions. The contact-arbiter architecture remains unchanged.

細かい再調整は不要。既存の合格済みファームと挙動が変わらないことが基準。
ユーザー実機合格により今回の統合条件は満たされた。手順は今後の回帰確認用に保持する。
接触arbiterのアーキテクチャは変更していない。
