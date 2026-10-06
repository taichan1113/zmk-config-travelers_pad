## English

# Reusable trackpad modules

## Responsibilities

| Component | Owns | Does not own |
| --- | --- | --- |
| Sensor driver | IQS9150EV02 initialization, I2C/RDY communication, contact frames | Pointer and gesture policy |
| Input functions | Contact sessions, arbitration, pointer/tap/scroll/gesture behavior | Board wiring and key assignments |
| Keyboard configuration | Hardware overlay, feature selection, processors, KSCAN and key bindings | Sensor protocol implementation |

The tested driver and function profile use the IQS9150 contact-event contract.
A different sensor needs an adapter that publishes compatible contact events.
Changing a Devicetree phandle alone does not provide support for another IC.
Multiple pads and peripheral transport have not been verified.

## Use in a keyboard workspace

Declare the modules needed by the selected profile in the keyboard's west manifest,
then run `west update`. Keep the keyboard's existing ZMK setup and board configuration.

For the sensor, define a node inside an enabled I2C bus and set GPIO/pinctrl for
the actual board:

```dts
#include <zephyr/dt-bindings/gpio/gpio.h>

iqs9150: trackpad@56 {
    compatible = "azoteq,iqs9150";
    reg = <0x56>;
    rdy-gpios = <&gpio1 13 GPIO_ACTIVE_LOW>;
};
```

The tested device uses I2C address 0x56 and an active-low RDY input. Enable:

```ini
CONFIG_IQS9150_EV02_DRIVER=y
CONFIG_ZMK_POINTING=y
CONFIG_ZMK_POINTING_SMOOTH_SCROLLING=y
CONFIG_INPUT_QUEUE_MAX_MSGS=64
```

## Basic pointer profile

After defining an input device labelled `iqs9150`, include the profile:

```dts
#include <travelers-pad/iqs9150-pointer.dtsi>
```

It provides contact-session arbitration, pointer movement, two-finger scrolling,
one-to-three-finger tap/drag, and optional inertia. It does not select KSCAN or
define a keymap. If tap/drag is disabled, disable its HID listener as well.

Tune from the keyboard overlay:

```dts
&iqs9150_pointer_listener {
    input-processors = <&zip_scroll_scaler 1 4>;
};
&iqs9150_scroll_inertia {
    half-life-ms = <250>;
};
```

Keep the canonical wheel observation input separate from the HID listener.
Attaching a processor to the observation input can cause double scaling.

## Optional gestures

Three/four-finger swipes, five-finger Grasp/Release, Tip-Tap, pinch/rotation,
and edge dial require virtual KSCAN composition and keyboard bindings. Add only
the virtual positions used by the keyboard, and preserve its physical matrix.
The grid adapter is optional and is not part of the basic profile.

## Build and verification

From a prepared workspace, run:

```sh
nix develop --command bash config/zmk-config-travelers_pad/scripts/build-reuse-profiles.sh
```

The fixtures build a driver-only configuration and a basic pointer profile.
They are compile checks; verify actual board wiring before flashing a fixture.

For hardware regression, use the full profile and check pointer, 1/2/3-finger taps,
double-tap drag, two-finger scroll/inertia, enabled edge actions, Tip-Tap, a
three-finger swipe, and pinch. Confirm rotation remains disabled.

## 日本語

# トラックパッド機能moduleの再利用

## 責務

| 構成要素 | 担当 | 担当しない |
| --- | --- | --- |
| センサードライバー | IQS9150EV02初期化、I2C/RDY通信、接触frame | ポインタやジェスチャーの方針 |
| 入力機能 | 接触session、裁定、ポインタ・タップ・スクロール・ジェスチャー処理 | 基板配線とキー割当 |
| キーボード設定 | ハードウェアoverlay、機能選択、processor、KSCAN、キーbinding | センサープロトコル実装 |

確認済みdriverと機能profileはIQS9150接触event契約を使用します。
別センサーでは互換接触eventを出力するadapterが必要です。
Devicetreeのphandleを変えるだけでは、別ICに対応できません。
複数パッドとペリフェラル転送は未確認です。

## キーボードworkspaceでの利用

選択するprofileに必要なmoduleをキーボードのwest manifestへ記載し、
`west update`を実行します。既存のZMK設定と基板設定は維持してください。

センサーは有効なI2C bus内にnodeを定義し、実際の基板に合わせてGPIO/pinctrlを設定します:

```dts
#include <zephyr/dt-bindings/gpio/gpio.h>

iqs9150: trackpad@56 {
    compatible = "azoteq,iqs9150";
    reg = <0x56>;
    rdy-gpios = <&gpio1 13 GPIO_ACTIVE_LOW>;
};
```

確認済みデバイスのI2Cアドレスは0x56、RDY入力はactive-lowです。
次の設定を有効にします:

```ini
CONFIG_IQS9150_EV02_DRIVER=y
CONFIG_ZMK_POINTING=y
CONFIG_ZMK_POINTING_SMOOTH_SCROLLING=y
CONFIG_INPUT_QUEUE_MAX_MSGS=64
```

## 基本ポインタprofile

`iqs9150` labelのinput deviceを定義した後、profileをincludeします:

```dts
#include <travelers-pad/iqs9150-pointer.dtsi>
```

接触sessionの裁定、ポインタ移動、2本指スクロール、1〜3本指のタップ・ドラッグ、
任意の慣性機能を提供します。KSCANやkeymapは選択しません。
タップ・ドラッグを無効化する場合はHID listenerも無効にしてください。

キーボードoverlayで調整します:

```dts
&iqs9150_pointer_listener {
    input-processors = <&zip_scroll_scaler 1 4>;
};
&iqs9150_scroll_inertia {
    half-life-ms = <250>;
};
```

canonical wheel observation inputはHID listenerから分離してください。
観測inputにprocessorを接続すると、二重倍率が発生する可能性があります。

## 任意ジェスチャー

3/4本指スワイプ、5本指Grasp/Release、Tip-Tap、ピンチ・回転、エッジダイヤルには、
virtual KSCANの合成とキーボード側のbindingが必要です。
キーボードで使うvirtual positionだけを追加し、物理matrixを維持してください。
格子adapterは任意機能で、基本profileには含まれません。

## ビルドと確認

準備済みworkspaceから次を実行します:

```sh
nix develop --command bash config/zmk-config-travelers_pad/scripts/build-reuse-profiles.sh
```

fixtureではdriver単体構成と基本ポインタprofileをbuildします。
これはcompile確認用です。fixtureを書き込む前に実際の基板配線を確認してください。

実機回帰では全機能profileを使い、ポインタ、1/2/3本指タップ、ダブルタップドラッグ、
2本指スクロール・慣性、有効なエッジ操作、Tip-Tap、3本指スワイプ、ピンチを確認します。
回転が無効のままであることも確認してください。
