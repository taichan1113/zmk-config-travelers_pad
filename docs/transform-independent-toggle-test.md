# Independent transform enablement / ピンチ・回転の独立有効化

Branch / ブランチ: `feat/transform-independent-toggle`
Baseline / 基準: `f848b42` (`main`, edge-start policy hardware accepted / 辺取得実機合格済み)

## Configuration / 設定

Both operations remain enabled when no disable flags are supplied. Configuration
is build-time, not a live runtime switch. Key positions and bindings are unchanged.
無効化flagがない場合は従来どおり両方有効。ビルド時設定であり実行中の切替ではない。
キー位置・割当は変更しない。

```dts
/* Pinch only / ピンチのみ */
&iqs9150_transform_gesture { rotation-disabled; };

/* Rotation only / 回転のみ（上記とは別の設定例） */
&iqs9150_transform_gesture { pinch-disabled; };

/* Neither / 両方無効（別の設定例） */
&iqs9150_transform_gesture { pinch-disabled; rotation-disabled; };
```

The router's `transform-device = <&iqs9150_transform_gesture>` reads those flags,
status and CONFIG at build time. An omitted phandle preserves legacy geometry
exclusion. This reference adds no runtime calls between recognizers. Disabled
recognition never claims ownership or emits actions. The router also stops excluding
that geometry type. Disabling transforms does not make opposing-finger motion
scroll: scroll still requires coherent movement of both fingers.

routerの`transform-device`はflag・status・CONFIGをビルド時に参照する。参照を省略した場合は
従来のジオメトリ除外を保持する。recognizer間のruntime呼出しは増やさない。無効な認識は
所有権を要求せず、出力せず、routerの当該ジオメトリ除外も無効になる。無効化しても対向移動を
スクロールへ強制変換するわけではなく、スクロールには両指の同方向移動が必要。

## Hardware checks / 実機確認

UF2 files are in Windows Downloads / UF2はWindows Downloadsに保存する。

| UF2 | Check / 確認 |
|---|---|
| `travelers-pad-transform-pinch-only-20261005.uf2` | Pinch still zooms; rotation does not emit F15/F16 / ピンチで拡縮、回転でF15/F16が出ない |
| `travelers-pad-transform-rotation-only-20261005.uf2` | Rotation emits F15/F16; pinch does not zoom / 回転でF15/F16、ピンチで拡縮しない |
| `travelers-pad-transform-both-disabled-20261005.uf2` | Neither transform emits actions or owns a session / 両方の操作・transform所有権取得がない |
| `travelers-pad-transform-both-enabled-20261005.uf2` | Restore ordinary operation after the three checks / 上記3設定の確認後に通常の両方有効へ戻す |

Test in that order. For each image, also verify normal 2F scroll, 2F right tap,
and 1F pointer. For both-disabled, translate both fingers coherently while slowly
changing their spacing; scroll must not become permanently suppressed by geometry.
Lift all fingers before each new operation.

表の順で確認。各UF2で通常2Fスクロール・2F右タップ・1Fポインタも簡単に確認。
両方無効では、指間隔を徐々に変えつつ2本を同方向に動かし、ジオメトリ判定のため
スクロールが途中から永久に抑止されないこと。各操作の間は全指を離す。

Rotation is best checked with an F15/F16 key-event monitor plus serial logs, not
visible desktop motion. Inspect `2F transform action=...` logs: disabled directions
must never appear; with both disabled, no transform owner/action should appear.
Mixed physical pinch/rotation may still activate the enabled component.

回転は画面上の変化でなく、F15/F16を確認できるキーイベント表示とログで判断する。
`2F transform action=...`に無効な方向が出ないこと。両方無効はtransform所有権・出力が
出ないこと。ピンチと回転が混ざった実操作では、有効な成分だけが発火する可能性がある。

Hardware approval pending / 実機承認待ち。
