# Reusable modules / 再利用モジュール

## Boundary / 責務境界

| Component / 構成 | Owns / 担当 | Does not own / 担当しない |
| --- | --- | --- |
| Driver / ドライバー | `azoteq,iqs9150`、EV02初期化、I2C・RDY処理、接触フレーム / EV02 initialization and contact frames | ポインタ・クリック・操作ポリシー / interaction policy |
| Functions / 機能群 | session・arbiter・broker、認識器、慣性・継続 / ownership, recognizers and continuation | キーボードの配線・物理キー配列 / keyboard wiring and layout |
| Keyboard config / キーボード設定 | 配線、使用機能、チューニング、listener processor、KSCAN統合、キー割当 / integration and tuning | ドライバーや認識器のソース変更 / driver/recognizer implementation |

One Zephyr module is still registered by `zephyr/module.yml`. Driver and function
source lists are separated in `cmake/driver.cmake` and `cmake/functions.cmake`;
Kconfig entry points are separated under `src/driver/iqs9150` and
`src/function/iqs9150`. Public compatibles, CONFIG names and C headers are unchanged.

`zephyr/module.yml`で登録するZephyr moduleは引き続き1つ。driver/functionのsource listと
Kconfig入口を分離した。公開compatible・CONFIG名・Cヘッダは変更していない。
今後driverと機能群を2repositoryへ切り出すための第一段階で、現時点では別repoを作成・公開しない。

The driver is EV02-specific. Functions still have IQS9150-named CONFIG dependencies
and a specific contact-frame contract; do not assume they work with another IC
merely by changing a phandle. Multiple pads and peripheral transport are not verified.

ドライバーはEV02固有の初期化データを使う。機能群にもIQS9150名のCONFIG依存と接触frame契約が
残るため、phandleの変更だけで異なるICを使えるとは限らない。複数パッド・ペリフェラル転送は未検証。

## Import / 取り込み

During local development, add this repository to `ZMK_EXTRA_MODULES`. For west
distribution, add it as an ordinary project to the consuming keyboard's manifest,
using a published, validated commit for `revision`. Do not replace the keyboard's
manifest with this repository's `config/west.yml` or import its keyboard configuration.

ローカル開発では本repoを`ZMK_EXTRA_MODULES`へ追加する。west配布では接続先キーボードの
manifestへ通常のprojectとして追加し、公開・検証済みcommitを`revision`へ固定する。
本repoの`config/west.yml`やキーボード設定全体を接続先へ置き換えて取り込まない。

### Driver only / ドライバーだけ

Define the sensor in your own enabled I2C bus and use your own GPIO/pinctrl settings:

キーボード所有のI2C bus内へセンサを定義し、GPIO・pinctrlは実際の配線に合わせる：

```dts
/* Inside the keyboard's I2C bus / キーボードのI2C bus内 */
iqs9150: trackpad@56 {
    compatible = "azoteq,iqs9150";
    reg = <0x56>;
    /* Add rdy-gpios matching the wiring / 配線に合わせrdy-gpiosを追加 */
};
```

`rdy-gpios` is required and active-low. Enable `CONFIG_IQS9150_EV02_DRIVER=y`.
Without function nodes, no function CONFIG is automatically enabled, and the sensor
does not generate mouse HID by itself. Do not flash driver-only builds expecting pointing.

`rdy-gpios`は必須・active-low。`CONFIG_IQS9150_EV02_DRIVER=y`を設定する。
機能nodeを定義しなければ機能CONFIGは自動有効化されず、センサ単体ではマウスHIDを生成しない。

### Basic pointer profile / 基本ポインタprofile

After defining a sensor labelled `iqs9150`, include:

`iqs9150`ラベルのセンサを定義し、次を追加する：

```dts
#include <travelers-pad/iqs9150-pointer.dtsi>
```

This provides session/arbiter/broker, router, pointer and 2F scroll, 1–3F tap/drag,
canonical wheel observation, inertia, and their HID listeners. It does not choose
KSCAN, define a physical keymap, or require `zmk-input-matrix`. A disabled transform
extension node keeps router configuration explicit; enabling it needs KSCAN integration.

session/arbiter/broker、router、ポインタ・2Fスクロール、1〜3Fタップ/ドラッグ、
専用wheel観測、慣性、HID listenerを提供する。chosen KSCAN・物理keymapを定義せず、
`zmk-input-matrix`も不要。transform拡張nodeは無効で、routerがその状態を参照する。
ピンチなどを有効にする場合は別途KSCAN統合が必要。

Keyboard `.conf` / キーボードの`.conf`：

```ini
CONFIG_IQS9150_EV02_DRIVER=y
CONFIG_ZMK_POINTING=y
CONFIG_ZMK_POINTING_SMOOTH_SCROLLING=y
CONFIG_INPUT_QUEUE_MAX_MSGS=64
```

Keyboard tuning overrides / キーボード側の調整例：

```dts
&iqs9150_pointer_listener {
    input-processors = <&zip_scroll_scaler 1 4>;
};
&iqs9150_scroll_inertia { half-life-ms = <250>; };
/* Optional omission / 任意で無効化 */
/* &iqs9150_scroll_inertia { status = "disabled"; }; */
/* &iqs9150_tap_drag { status = "disabled"; }; */
/* &iqs9150_tap_drag_listener { status = "disabled"; }; */
```

When omitting tap/drag, disable its listener too. Keep the private wheel observation
source separate from the HID listener: adding a processor/listener there reintroduces
the double-scaling problem. Do not include the basic profile twice under different labels.

tap/dragを無効にする際は対応listenerも無効にする。専用wheel観測sourceには
HID listener／processorを接続しない（二重倍率対策）。別ラベルでprofileを重複生成しない。

### Virtual-key extensions / 仮想キー拡張

3/4F swipe, 5F Grasp/Release, Tip-Tap, pinch/rotation and edge dial require keyboard
bindings and KSCAN composition. The full XIAO shield remains the reference for these;
do not import its `chosen` or matrix directly into an existing keyboard. Preserve
the keyboard's physical transform and append only the virtual positions actually used.
Edge-hold continuation also needs surface dimensions and its dedicated drag listener.

3/4Fスワイプ、5F Grasp/Release、Tip-Tap、ピンチ/回転、辺ダイヤルにはキー割当とKSCAN合成が
必要。完全XIAO shieldは参照実装として残すが、既存キーボードへそのchosenやmatrixをそのまま
取り込まない。物理キーtransformを維持し、使用する仮想位置だけを追加する。
edge holdもパッド寸法と専用ドラッグlistenerが必要。これらの共通拡張DTS化は次の段階。

## Verification / 検証

From a prepared ZMK workspace / 準備済みZMK workspaceから：

```sh
nix develop --command bash config/zmk-config-travelers_pad/scripts/build-reuse-profiles.sh
```

This builds driver-only and basic pointer fixtures with only this extra module,
without the Travelers Pad shield or external grid dependency, and checks feature
CONFIG activation. `examples/reuse/driver-only.overlay` uses XIAO-specific wiring
and a mock keyboard solely as a build fixture; it is not the generic include.

本moduleだけを追加し、Travelers Pad shield・外部格子依存無しでドライバー単独／基本profileを
buildし、CONFIGの有効状態も確認する。examples内の配線・mockはXIAO向けビルド検証用であり、
汎用includeそのものではない。

For hardware regression, flash the full-profile UF2 and briefly check pointer,
1/2/3F taps, double-tap drag, 2F scroll/inertia, edge operations, Tip-Tap and a
3F swipe. Check pinch and confirm rotation remains disabled. This refactor must
not change the user's tuned values or key assignments.

実機回帰は完全profileのUF2でポインタ、1/2/3Fタップ、ダブルタップドラッグ、
2Fスクロール／慣性、辺操作、Tip-Tap、3Fスワイプを簡単に確認する。
ピンチと回転無効も確認する。この整理で手動調整値やキー割当は変更しない。
