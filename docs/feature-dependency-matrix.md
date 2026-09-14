# Travelers Pad feature dependency matrix / 機能依存 matrix

This matrix defines the supported build-time isolation boundaries. Every
profile retains `IQS9150_EV02_DRIVER` and `iqs9150_session`: the session
manager is the normalized-contact and arbiter foundation, not an optional
gesture feature.

この matrix は対応する build 時の分離境界を定義する。すべての profile は
`IQS9150_EV02_DRIVER` と `iqs9150_session` を維持する。session manager は
正規化 contact と arbiter の基盤であり、任意 gesture 機能ではない。

| Profile / profile | Required modules / 必須 module | May be disabled / 無効化可能 | Main evidence / 主な確認 |
|---|---|---|---|
| Router-pointer | session, router, pointer | tap/drag, virtual KSCAN recognizers, edge hold, inertia | 1F pointer and 2F scroll only |
| Router-pointer observers | session, router, pointer, edge hold, inertia | tap/drag, virtual KSCAN recognizers | edge continuation and post-release inertia only |
| Tap-drag | session, tap/drag | router, pointer, virtual KSCAN recognizers, observers | 1/2/3F click and drag only |
| Gesture KSCAN | session, multifinger gesture | router, pointer, tap/drag, observers, Tip-Tap, edge dial | 3--5F virtual KSCAN only |
| Tip-Tap | session, Tip-Tap | router, pointer, tap/drag, observers, gesture, edge dial | anchored 2F virtual KSCAN only |
| Edge dial | session, edge dial | router, pointer, tap/drag, observers, gesture, Tip-Tap | edge virtual KSCAN only |
| Full | session, router, pointer, tap/drag, virtual KSCAN recognizers, edge hold, inertia | optional grid/matrix | full integration suite |

`edge_hold_continue` and `scroll_inertia` require session, router, and pointer,
because their Devicetree contracts consume all three streams. `grid_kscan` and
`matrix_input` require session but remain optional and disabled in the shipped
profile. Tip-Tap explicitly selects `KSCAN`.

`edge_hold_continue` と `scroll_inertia` は、Devicetree 契約で三つすべての
stream を消費するため、session、router、pointer を必要とする。`grid_kscan` と
`matrix_input` は session を必要とするが任意であり、出荷 profile では無効である。
Tip-Tap は `KSCAN` を明示的に select する。

## Build gate / build gate

Run the structural dependency check before creating a feature-limited firmware:

```sh
./scripts/verify-contact-feature-dependencies.sh
```

From the ZMK workspace root, build all diagnostic profiles with:

```sh
nix develop --command bash config/zmk-config-travelers_pad/scripts/build-feature-matrix.sh
```

The overlays are in [`tests/feature-matrix`](../tests/feature-matrix). The ZMK
v0.3 composite KSCAN driver constructs every child even when that child is
`disabled`; an omitted virtual-KSCAN recognizer is therefore redirected to the
inert `iqs9150_kscan` mock. This retains the shipped chosen composite/layout
pair while ensuring that the omitted recognizer source is not compiled.

overlay は [`tests/feature-matrix`](../tests/feature-matrix) にある。ZMK v0.3 の
composite KSCAN driver は `disabled` な child も構築するため、外す virtual-KSCAN
recognizer は inert な `iqs9150_kscan` mock へ差し替える。これにより出荷時の
chosen composite/layout pair を保ちつつ、外した recognizer の source を compile しない。

The full configuration remains the release candidate. Feature-limited UF2
files are diagnostic artifacts: use the isolated procedure named in
[`README.md`](README.md) only to localize an integration failure. When a
profile overlay retains the session manager and disables each omitted function
node; its matching Kconfig symbol then becomes unavailable through `DT_HAS`.

完全構成がリリース候補である。機能限定 UF2 は診断用成果物であり、結合失敗の切り分け
だけに [`README.md`](README.md) の単体手順を使用する。profile overlay を追加する時は、
session manager を残し、外す機能の Devicetree node と対応する Kconfig symbol の両方を
無効化して build する。
