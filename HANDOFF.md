[English](#english) | [日本語](#日本語)

# Project handoff

## English

Last updated: 2026-10-06

### Project status

The Travelers Pad configuration, IQS9150 driver, and trackpad function module
are maintained as three repositories. Their current READMEs are being prepared
for eventual public release. The two reusable module repositories are private
at present. No repository has been made public during this documentation pass.

### Hardware

Validated target: Seeed XIAO nRF52840 with the IQS9150EV02 trackpad.

| XIAO pin | Trackpad signal |
| --- | --- |
| 3V3 | VDD |
| GND | GND |
| xiao_11 / D10 / P1.15 | SDA |
| xiao_10 / D9 / P1.14 | SCL |
| xiao_9 / D8 / P1.13 | RDY (active low) |
| P1.12 | Unused |

The tested profile uses 3.3 V and common ground. Do not connect USB 5 V to the
EV02 supply.

### Implemented behavior

The driver initializes the EV02, performs ATI calibration, reads contact data,
and emits normalized contact events. The function module handles pointer motion,
one-to-three-finger taps and tap-drag, two-finger scrolling and inertia,
Tip-Tap, three-to-five-finger gestures, edge controls, and pinch. Rotation is
disabled in the validated profile.

The XIAO keymap and overlay configure host bindings and tuning. The host receives
ordinary pointer, button, wheel, and key events rather than a native precision
touchpad report.

### License status

All three repositories now contain an MIT license for original project
contributions. The Azoteq-derived initialization settings in the driver are
excluded from that grant until redistribution rights are confirmed. See
[`docs/azoteq-license-review.md`](docs/azoteq-license-review.md) and the
driver's `THIRD_PARTY_NOTICES.md`.

### Documentation status

- The three user-facing READMEs use English and Japanese sections with page links.
- Development and regression documents are retained in the Travelers Pad repository.
- No firmware behavior or hardware configuration was changed in this documentation work.

### Next steps

1. Review the three README drafts and the MIT-license scope.
2. Obtain written Azoteq terms for public redistribution of the GUI-generated
   initialization settings and firmware containing them.
3. Once the rights are clear, complete the public-release review and publish only
   with the maintainer's instruction.
4. Keep the hardware-validated XIAO/EV02 profile as the regression reference.

### Build

From a ZMK workspace initialized with the configuration manifest:

```sh
west update
west build -p always -s zmk/app -d build/travelers-pad \
  -b seeeduino_xiao_ble -- \
  -DSHIELD=iqs9150_xiao \
  -DZMK_CONFIG=$PWD/config/zmk-config-travelers_pad/config \
  -DZMK_EXTRA_MODULES=$PWD/config/zmk-config-travelers_pad
```

See [the test guide](docs/README.md) for the project regression procedures.

## 日本語

# 引継ぎ書

更新日: 2026-10-06

### プロジェクトの状態

Travelers Pad設定、IQS9150 driver、トラックパッド機能moduleは、それぞれ別の3リポジトリで管理しています。
現在、それぞれのREADMEを将来の公開に向けて整備中です。
再利用moduleの2リポジトリは現時点でprivateです。今回の文書整備では公開化していません。

### ハードウェア

動作確認済み構成: Seeed XIAO nRF52840とIQS9150EV02トラックパッド。

| XIAOピン | トラックパッド信号 |
| --- | --- |
| 3V3 | VDD |
| GND | GND |
| xiao_11 / D10 / P1.15 | SDA |
| xiao_10 / D9 / P1.14 | SCL |
| xiao_9 / D8 / P1.13 | RDY (active low) |
| P1.12 | 未使用 |

確認済みprofileでは3.3 Vと共通GNDを使用します。USB 5 VをEV02電源へ接続しないでください。

### 実装済みの動作

driverはEV02を初期化し、ATIキャリブレーションと接触データ取得を行い、
正規化した接触eventを出力します。機能moduleはポインタ移動、1〜3本指のタップと
タップドラッグ、2本指スクロールと慣性、Tip-Tap、3〜5本指ジェスチャー、
エッジ操作、ピンチを処理します。確認済みprofileでは回転を無効にしています。

XIAOのkeymapとoverlayで、ホストへの割当とチューニングを行います。
ホストにはnative precision touchpadではなく、通常のポインタ、button、wheel、key eventを送ります。

### ライセンス状況

3リポジトリには、本プロジェクト独自の成果に対するMITライセンスを追加しました。
driver内のAzoteq由来初期化設定は、再配布権が確認できるまでMIT許諾の対象外です。
[Azoteqライセンス調査](docs/azoteq-license-review.md)とdriverの `THIRD_PARTY_NOTICES.md` を参照してください。

### 文書の状態

- 利用者向け3つのREADMEは、ページ内リンクと英語・日本語別の本文を備えています。
- 開発・回帰試験文書はTravelers Padリポジトリに保持します。
- 今回の文書整備ではファームウェア動作や実機設定を変更していません。

### 次の作業

1. 3つのREADME案とMITライセンスの適用範囲を確認する。
2. GUI生成初期化設定と、その設定を含むファームウェアの公開再配布条件をAzoteqから書面で得る。
3. 権利条件を確認した後、公開前レビューを完了し、メンテナーの指示に従って公開する。
4. XIAO/EV02の実機確認済みprofileを回帰確認の基準として維持する。

### ビルド

設定manifestで初期化したZMK workspaceから実行します:

```sh
west update
west build -p always -s zmk/app -d build/travelers-pad \
  -b seeeduino_xiao_ble -- \
  -DSHIELD=iqs9150_xiao \
  -DZMK_CONFIG=$PWD/config/zmk-config-travelers_pad/config \
  -DZMK_EXTRA_MODULES=$PWD/config/zmk-config-travelers_pad
```

回帰試験手順は[テスト手順一覧](docs/README.md)を参照してください。
