[English](#english) | [日本語](#日本語)

# Travelers Pad

## English

Travelers Pad is a ZMK configuration for an IQS9150EV02 trackpad connected to a
Seeed XIAO nRF52840. The repository contains the keyboard shield configuration,
keymap, board integration, and build settings for this tested setup.

### Hardware

| XIAO pin | Trackpad connection |
| --- | --- |
| 3V3 | VDD |
| GND | GND |
| xiao_11 / D10 / P1.15 | SDA |
| xiao_10 / D9 / P1.14 | SCL |
| xiao_9 / D8 / P1.13 | RDY (active low) |
| P1.12 | Unused |

Use the trackpad board's specified supply voltage. The tested EV02 wiring uses
3.3 V and a common ground. Do not connect XIAO USB 5 V to the EV02 supply.

### Build

Clone this repository into the ZMK workspace, initialize from its manifest, then build the
`iqs9150_xiao` shield for `seeeduino_xiao_ble`:

```sh
git clone https://github.com/taichan1113/zmk-config-travelers_pad.git config/zmk-config-travelers_pad
west init -l config/zmk-config-travelers_pad/config
west update
west build -p always -s zmk/app -d build/travelers-pad \
  -b seeeduino_xiao_ble -S zmk-usb-logging -- \
  '-DSHIELD=iqs9150_xiao rgbled_adapter' \
  -DZMK_CONFIG=$PWD/config/zmk-config-travelers_pad/config \
  -DZMK_EXTRA_MODULES=$PWD/config/zmk-config-travelers_pad
```

If the workspace is already initialized, keep its current manifest setup and
update the checked-out projects before building.

### Features

The keymap and overlays configure pointer movement, clicks, two-finger scroll
and inertia, edge controls, Tip-Tap, multi-finger gestures, and pinch. Rotation
is disabled in the tested profile. Key assignments and tuning values are in
`config/iqs9150_xiao.keymap` and the shield overlay.

The sensor emits contact frames. ZMK turns those into ordinary pointer, button,
wheel, and key events; the host does not see a native precision-touchpad device.

### Documentation

HID output uses Bluetooth; USB provides power and serial logging. Hold four
fingers still for about one second to enter management while touching. Cyan
indicates management; release, then repeat the hold to exit. Four-finger swipes
select next (left), previous (right), or first (up) host. Two down swipes within
1500ms clear the current host pairing; one down swipe does nothing. Clearing
requires re-pairing. See [management controls](docs/ble-management-layer.md).

- [Two-finger interaction flow](docs/two-finger-interaction-flow.md)
- [Architecture and tuning](docs/architecture-and-tuning.md)
- [Hardware integration test guide](docs/README.md)
- [Azoteq initialization-data license review](docs/azoteq-license-review.md)

The test procedures and architecture notes are maintained for development and
hardware verification. They describe this project and do not define a general
interface for unrelated hardware.

### License

Original project contributions are licensed under the MIT License in
[`LICENSE`](LICENSE). Third-party materials keep their own terms.
The redistribution status of the Azoteq-derived initialization settings is
still being reviewed; see [the license review](docs/azoteq-license-review.md).

## 日本語

Travelers Padは、IQS9150EV02トラックパッドをSeeed XIAO nRF52840へ接続するZMK設定です。
このリポジトリには、確認済み構成向けのキーボードshield、keymap、基板統合、build設定があります。

### ハードウェア

| XIAOピン | トラックパッド接続 |
| --- | --- |
| 3V3 | VDD |
| GND | GND |
| xiao_11 / D10 / P1.15 | SDA |
| xiao_10 / D9 / P1.14 | SCL |
| xiao_9 / D8 / P1.13 | RDY (active low) |
| P1.12 | 未使用 |

トラックパッド基板の仕様に合った電源電圧を使用してください。
確認済みEV02配線では3.3 Vと共通GNDを使用します。XIAOのUSB 5 VをEV02電源へ接続しないでください。

### ビルド

このリポジトリをZMK workspaceへcloneし、そのmanifestで初期化してから、
`seeeduino_xiao_ble`向けに `iqs9150_xiao` shieldをbuildします:

```sh
git clone https://github.com/taichan1113/zmk-config-travelers_pad.git config/zmk-config-travelers_pad
west init -l config/zmk-config-travelers_pad/config
west update
west build -p always -s zmk/app -d build/travelers-pad \
  -b seeeduino_xiao_ble -S zmk-usb-logging -- \
  '-DSHIELD=iqs9150_xiao rgbled_adapter' \
  -DZMK_CONFIG=$PWD/config/zmk-config-travelers_pad/config \
  -DZMK_EXTRA_MODULES=$PWD/config/zmk-config-travelers_pad
```

workspaceを初期化済みの場合は、既存manifest設定を維持し、build前にprojectをupdateしてください。

### 機能

keymapとoverlayでは、ポインタ移動、クリック、2本指スクロールと慣性、エッジ操作、
Tip-Tap、多指ジェスチャー、ピンチを設定しています。確認済みprofileでは回転を無効にしています。
キー割当と調整値は `config/iqs9150_xiao.keymap` とshield overlayにあります。

センサーは接触フレームを出力します。ZMKはそれを通常のポインタ、ボタン、ホイール、
キーイベントに変換します。ホストからはnative precision touchpadとして認識されません。

### 文書

HID出力はBluetooth、USBは給電・シリアル用です。4本指を約1秒静止すると指を置いたまま
管理へ入り、シアン点灯します。全指を離してから再びホールドすると終了。
管理中の4本指スワイプは左で次、右で前、上で接続先1。下1回は無操作、1500ms以内の
下2回で選択中のペアリングを消去します。消去後は再ペアリングが必要。
[管理操作](docs/ble-management-layer.md)を参照してください。

- [2本指操作の処理フロー](docs/two-finger-interaction-flow.md)
- [アーキテクチャと調整](docs/architecture-and-tuning.md)
- [実機結合テスト手順](docs/README.md)
- [Azoteq初期化データのライセンス調査](docs/azoteq-license-review.md)

テスト手順とアーキテクチャ資料は開発・実機確認用です。このプロジェクトの構成を説明するもので、
別のハードウェアに共通するinterfaceを定義するものではありません。

### ライセンス

本プロジェクト独自の成果は[`LICENSE`](LICENSE)に記載したMIT Licenseです。
第三者資料には各資料固有の条件が適用されます。
Azoteq由来の初期化設定の再配布条件は確認中です。[調査資料](docs/azoteq-license-review.md)を参照してください。
