# External module migration / 外部moduleへの移行

## Target and status / 目標と状態

Travelers Pad uses independent driver and function projects under the ZMK workspace's
`module/`, acquired by west. User instruction: local repositories only; no GitHub
creation or push. Hardware approval is still pending for this extraction.

Travelers Padはworkspaceの`module/`にある独立driver・機能projectをwestで取り込む。
ユーザー指定によりローカルrepoのみ。GitHub作成・pushは行わない。
今回の切り出しは実機確認待ち。

| Repository / repo | Responsibility / 責務 | Pinned commit / 固定commit |
| --- | --- | --- |
| `module/zmk-driver-iqs9150` | Driver, EV02 init, sensor binding, contact ABI / driver・初期化・センサbinding・接触ABI | `fe8c69a48b24d43b63111db49ac6925be24c5d8c` |
| `module/zmk-input-travelers-pad` | All function implementations, arbiter, bindings, shared profile / 全機能実装・arbiter・binding・共通profile | `819fce3da423c33bda7d3efba83f1572b2843351` |
| `config/zmk-config-travelers_pad` | Hardware wiring, config, virtual matrix, keymap, diagnostics/tests / 配線・設定・仮想matrix・keymap・診断／試験 | Branch `refactor/external-trackpad-modules` |

The driver owns `include/travelers_pad/multitouch.h`; the function module consumes
it without a duplicate. The keyboard no longer contains driver/function C files,
headers, bindings, or their Kconfig/build definitions. Removed files are recoverable
from the keyboard's `b0ad83d` checkpoint and now tracked in the external repositories.

共有接触headerはdriverが所有し、機能moduleは複製せず利用する。
キーボードrepoからdriver／機能のCファイル・header・binding・Kconfig/build定義を除去した。
除去したファイルは旧checkpoint `b0ad83d`から復元でき、外部repoでGit管理されている。

## west / 取り込み

`config/west.yml` pins the projects using local `file:///home/takec/zmk-workspace/module/...`
URLs. Existing workspace manifest generation was refreshed to include these projects.

本repoのmanifestはローカルURLでprojectを固定している。workspace側の生成manifestも更新済み。

```sh
# From zmk-workspace / workspaceから
nix develop --command west update zmk-driver-iqs9150 zmk-input-travelers-pad
```

west normally checks out modules in detached HEAD state. Before changing a module,
create a feature branch inside it, commit the changes, then update the keyboard's
manifest revision and regenerate its workspace manifest (`just init`). Updating
only a checkout without changing the pin is not a reproducible integration.

westは通常detached HEADでmoduleをcheckoutする。module改修時はmodule内でbranchを作り、
commit後にキーボードmanifestのrevisionを更新し、`just init`で生成manifestも更新する。
checkoutだけの変更では再現可能な統合にはならない。

Local-only limitations: the source and working checkout currently share the same
path; keep the source repositories and their pinned commits. Another PC must copy
both Git repositories and adapt the file URLs, or later use published URLs after
explicit authorization. Do not remove `module/` assuming west can redownload it
from GitHub. This extraction has not published any repository.

ローカル限定の制約：取得元と作業checkoutは同じ場所。取得元Git repoと固定commitを保持する。
別PCでは両Git repoのコピーとfile URLの調整が必要。公開URLへの変更は後日明示的な指示後に行う。
GitHubから再取得できる前提で`module/`を削除しない。今回repo公開は一切していない。

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
firmware. Report deviations and serial logs; do not merge the extraction branch until
hardware approval. The contact-arbiter architecture remains unchanged.

細かい再調整は不要。既存の合格済みファームと挙動が変わらないことが基準。
差異とシリアルログを共有し、実機合格まで切り出しbranchをmainへ統合しない。
接触arbiterのアーキテクチャは変更していない。
