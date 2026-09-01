# IQS9150EV02 / XIAO nRF52840 — Travelers Pad

## 現在の状態 / Current status

STEP1Aでは実機でI²C、RDY割込み、タッチ座標の生データ取得を確認済みです。STEP1Bは
`IQS9150_init.h` のEV02設定を書き込み、ATI（自動調整）完了後にEvent Modeへ移行します。
STEP1Cは検証済みの相対移動を ZMK のマウスHIDへ送ります。STEP1Dは2本指の縦横スクロールと
タップクリック（1本指=左、2本指=右）を追加します。速度調整、3本指中クリック、スワイプ、
ピンチはまだ実装しません。

Step 1A has verified I²C, RDY interrupts, and raw touch reports on hardware.
Step 1B writes the EV02 settings from `IQS9150_init.h`, runs ATI, and enters
Event Mode. Step 1C forwards verified relative motion to ZMK mouse HID.
Step 1D adds two-finger vertical/horizontal scroll plus one-/two-finger tap
for left/right click. It does not yet implement speed tuning, middle click,
swipes, or pinch.

This is a ZMK/Zephyr module for IQS9150EV02 bring-up and basic pointer output.

## モジュール構成 / Module architecture

センサ固有の処理と操作ポリシーを分離しています。`driver` はIQS9150の初期化、RDY、ATI、
Event Mode、および正規化した接触フレーム（最大7点の絶対座標、接触本数）だけを担当します。
クリック、スクロール、加速、ジェスチャーをここへ戻さないでください。

Sensor-specific work and interaction policy are separate. The `driver` owns
IQS9150 initialization, RDY, ATI, Event Mode, and normalized contact frames
(up to seven absolute contact coordinates and finger count) only. Do not put click,
scroll, acceleration, or gesture policy back into the driver.

| 内部モジュール / Internal module | 役割 / Responsibility | 状態 / Status |
| --- | --- | --- |
| `driver` (`input_iqs9150_step1a.c`) | IQS9150を生の接触フレームへ変換 / IQS9150 to raw contact frames | 有効 / enabled |
| `function/iqs9150_pointer.c` | 相対ポインタ、2本指スクロール / pointer and scroll | 有効 / enabled |
| `function/iqs9150_multifinger_router.c` | 多指操作の排他判定 / exclusive multi-finger routing | 有効 / enabled |
| `function/iqs9150_tap_drag.c` | 1〜3本指タップ、ダブルタップ＋移動開始ドラッグ / one-to-three-finger taps and double-tap-move drag | 有効 / enabled |
| `function/iqs9150_grid_kscan.c` | 格子タップ・端部タップをZMK keymap向け仮想KSCANへ変換 / grid and edge taps to virtual KSCAN | 実装済み、初期無効 / implemented, disabled by default |
| `function/iqs9150_multifinger_gesture_kscan.c` | 3〜5本指スワイプを仮想KSCANへ変換 / 3-to-5-finger swipes to virtual KSCAN | 有効 / enabled |
| `function/iqs9150_inertia.c` | 慣性 / inertia | 次段階、独立実装 / next, independent |

## マルチタッチ座標 / Multi-touch coordinates

IQS9150ドライバは、Finger 1を `ABS_X/Y`、Finger 2を `ABS_RX/RY`、接触本数を
`ABS_Z`、Finger 2座標の有効性を `ABS_RUDDER` として、同じ入力フレームで出力します。
既存のポインタ機能はFinger 1だけを使用するため、この追加だけでポインタ動作は変わりません。
将来のピンチ・正確な2本指スクロールなどの機能モジュールは、この2点座標を利用します。

IQS9150ドライバは最大7点を、スロット付きの内部マルチタッチフレームとして公開します。
各スロットは追跡ID、X、Y、有効性を含み、同じ指はフレーム間で同じスロットに維持されます。
既存互換のため、Finger 1は `ABS_X/Y`、Finger 2は `ABS_RX/RY`、接触本数は `ABS_Z`、
Finger 2有効性は `ABS_RUDDER` としても同時出力します。

The IQS9150 driver publishes up to seven contacts as an internal slot-based
multi-touch frame. Every slot contains tracking ID, X, Y, and validity, and a
finger remains in the same slot across frames. For existing compatibility it
also emits Finger 1 as `ABS_X/Y`, Finger 2 as `ABS_RX/RY`, contact count as
`ABS_Z`, and Finger 2 validity as `ABS_RUDDER`.

未接触スロットはイベント送信しません。接触開始・終了時だけ追跡IDを出力し、接触中は
座標だけを更新します。これにより、非同期入力キューを無駄に消費せず、5点フレームを安全に
処理できます。

Inactive slots do not emit events. Tracking IDs are emitted only when a
contact starts or ends, while active contacts update only their coordinates.
This avoids wasting the asynchronous input queue and keeps five-contact
frames safe to process.

二本指スクロールは既存互換フレームの代表座標（Finger 1）と接触数だけを使います。一方、
ピンチ・回転・二本指の方向整合判定は両方のスロット座標を必要とします。将来のジェスチャー
ルーターはこの二経路を排他的に振り分け、ピンチ／回転候補をスクロールへ流しません。

Two-finger scrolling uses only the representative legacy coordinate (Finger 1)
and contact count. Pinch, rotation, and two-finger direction-coherence checks
require both slot coordinates. A future gesture router will route these paths
exclusively, so pinch/rotation candidates do not become scroll output.

実機検証時はoverlayの `two-finger-diagnostics` を有効にします。2本指で静止・平行移動・
指間を広げる／狭める操作を行い、100 msごとの `2F frame` ログで `f1=(x,y,1)` と
`f2=(x,y,1)` が別々に変化することを確認してください。`f2` の末尾が `0` の場合は、
そのフレームでは第2接触点を信頼しません。続く `2F geometry` のベクトルと距離二乗は、
平行移動ではほぼ一定、ピンチでは距離が変化、回転ではベクトル方向が変化します。

For hardware validation, enable `two-finger-diagnostics` in the overlay. Hold,
move, spread, and pinch two fingers while checking the throttled `2F frame`
logs. `f1=(x,y,1)` and `f2=(x,y,1)` must change independently. A trailing
`0` on `f2` means that frame's second contact is not trusted. The following
`2F geometry` vector and squared distance stay nearly constant for parallel
motion, change distance for pinch, and change vector direction for rotation.

3〜5本指の実機検証では `multitouch-diagnostics` を有効にし、`MT frame: count=3`、
`count=4`、`count=5` の各ログで、使用中の `f1`〜`fN` がすべて末尾 `1`、かつ別座標で
あることを確認します。2本指の平行移動では二点間距離がほぼ一定、ピンチでは距離が変化、
回転では二点ベクトルの向きが変化することを記録します。これらは次段階のルーター検証条件です。

For three- to five-finger hardware validation, enable `multitouch-diagnostics`.
For each `MT frame: count=3`, `count=4`, and `count=5`, verify every active
`f1` through `fN` ends in `1` and occupies a distinct position. Record that
parallel two-finger motion preserves distance, pinch changes distance, and
rotation changes the two-finger vector angle. These are the acceptance checks
for the next router stage.

## 多指操作ルーター / Multi-finger router

ポインタ機能の前にルーターを置き、操作セッションを一度だけ分類します。1本指は直ちに
ポインタへ渡します。2本指は短時間保留し、平行移動だけをスクロールへ渡します。指間距離の
変化（ピンチ）または二点ベクトルの回転はジオメトリ操作として確保され、スクロールを出力
しません。3本・4本指は完了時に仮想KSCANの位置へ変換します。5本指は独立座標による
Grasp/Releaseとして、確定時に専用の仮想KSCAN位置へ変換します。

The router sits before the pointer function and classifies each session once.
One-finger contact passes through immediately. Two-finger contact is briefly
held: only parallel motion reaches scrolling. Distance change (pinch) or
two-point-vector rotation is claimed as geometry and produces no scroll.
Three and four fingers are claimed as swipe candidates and become virtual KSCAN positions on
release. Five fingers use independent coordinates for Grasp/Release and emit dedicated virtual
KSCAN positions when confirmed.

## 3〜4本指スワイプと5本指形状ログ / Three-to-four-finger swipes and five-finger geometry logs

`iqs9150_gesture_kscan` は重心が `swipe-min-move`（初期値180）以上移動し、接触時間が
`swipe-max-ms`（初期値700 ms）以内の3本・4本指操作をスワイプとして確定する。3本指を行0、
4本指を行1、左・右・上・下を列0・1・2・3として、対応する仮想KSCANを一度だけ押下・解放する。

5接触が `five-finger-stable-frames`（初期値3）フレーム継続して
同じ追跡IDであることを確認してから、5点それぞれの重心からのマンハッタン距離を基準値として
記録する。4点以上が `five-finger-radial-move`（初期値80）以上、内側へ移動すると`Grasp`、
外側へ移動すると`Release`候補を確定する。確定時にrow 2のcolumn 0へGrasp、column 1へReleaseを
一度だけ出力し、全指を離すまで再発火しない。
`RC(2,2)`と`RC(2,3)`はmatrix transformから省略するため、keymapの論理positionは10個だけとなる。

ダブルタップ＋ドラッグが接触を所有した場合は、このモジュールはジェスチャーを出力しない。
従って3本指の中クリックドラッグと3本指スワイプは混在しない。

`iqs9150_gesture_kscan` recognizes 3/4-finger swipes when centroid displacement reaches
`swipe-min-move` (180 by default) within `swipe-max-ms` (700 ms by default). It emits one virtual
KSCAN press/release: row 0 selects 3 fingers, row 1 selects 4 fingers, and columns 0/1/2/3 select
left/right/up/down. For five fingers it first captures a baseline only after
`five-finger-stable-frames` (3 by default) complete frames with the same tracking IDs. If at least
four contacts move inward or outward from the centroid by `five-finger-radial-move` (80 by default),
it confirms `Grasp` or `Release` and emits row 2 column 0 or 1 once, respectively, until all fingers
are released. `RC(2,2)` and `RC(2,3)` are omitted from the matrix transform, so the keymap has only
10 logical positions. A double-tap drag owns its raw contact stream and suppresses gesture output.

検証時は次のログと画面動作を対応させます。

| 操作 / Operation | 期待ログ / Expected log | 期待HID動作 / Expected HID result |
| --- | --- | --- |
| 1本指移動 / one-finger move | ルーターログなし / no router log | ポインタ移動 / pointer movement |
| 2本指平行移動 / parallel two-finger move | `2F route=scroll committed` | スクロールのみ / scroll only |
| ピンチまたは回転 / pinch or rotation | `2F route=geometry claimed` | スクロールなし / no scroll |
| 3〜5本指移動 / three-to-five-finger move | `3F`〜`5F route=multifinger claimed` と終了時の方向 / claim plus direction on release | ポインタ・スクロールなし / no pointer or scroll |

## タップとダブルタップ・移動開始ドラッグ / Tap and double-tap-move drag

`iqs9150_tap_drag` はポインタ／スクロールとは独立して生の接触フレームを監視する。
1本指は左、2本指は右、3本指は中クリックに対応する。単発タップは、二回目の
タップを判定できるよう `double-tap-max-gap-ms` の間だけ保留する。近接した位置で
同じ本数の二回目タップが始まり、`drag-start-move` を超えて動かすと該当ボタンを
即座に押下する。以後の移動はドラッグ、指を離すとドロップとなる。

`iqs9150_tap_drag` watches raw contact frames independently of pointer and scrolling.
One, two, and three fingers select left, right, and middle click. A single tap is deferred for
`double-tap-max-gap-ms` so a nearby second tap with the same finger count can be recognized.
The second tap is a pending double-tap: releasing it produces a double click, while movement beyond
`drag-start-move` immediately presses the selected button and starts dragging. Small stationary
movement followed by release still produces a double click. The timing and distance properties are
all DTS-tunable.

二回目のタップは保留状態となる。離せばダブルクリック、`drag-start-move` を超えて動かせば
そのフレームで該当ボタンを押してドラッグを開始する。微小な揺れのまま離した場合も
ダブルクリックとなる。時間・距離の値はDTSで調整できる。

ドラッグ開始後の座標差分はタップ機能自身が直ちに相対ポインタ出力へ変換する。同時に
多指ルーターはその接触ストリームを抑止するため、2本指ドラッグがスクロールへ化けず、
開始位置からずれてドラッグを始めない。The tap function emits relative pointer deltas
immediately after drag ownership starts, while the router suppresses that stream; therefore a
two-finger drag cannot turn into scroll or begin from a later position.

複数本指では、指のF-slot番号が接触ごとに入れ替わっても同じ場所を正しく判定できる
よう、各接触点の重心を比較・移動座標に使う。For multiple fingers, the function uses the
contact centroid for comparison and drag movement, so IQS9150 F-slot reordering between taps does
not make the same physical area appear distant.

格子モジュールを有効にすると、短い1本指タップは通常の左クリックではなく、ZMK keymapで
設定できる仮想KSCANキーになります。`iqs9150_grid` は3×3格子の例と、左・右・上・下端部の
4つの任意セル割当を持ちます。使用時はノードを `status = "okay"` にし、`chosen` の
`zmk,kscan` とmatrix transform/keymapをその格子用に切り替えます。初期設定で無効なのは、
既存のポインタ・タップ操作を変えないためです。

When enabled, short one-finger grid taps become virtual KSCAN keys rather than
ordinary left clicks, so ZMK keymap bindings control their actions. The sample
`iqs9150_grid` node has a 3×3 grid and four configurable edge cells (left,
right, top, bottom). Set its `status = "okay"` and switch the chosen
`zmk,kscan`, matrix transform, and keymap to the grid when using it. It stays
disabled initially to preserve the existing pointer and tap behavior.

## 配線とDTS / Wiring and DTS

Use the XIAO's right-hand header as follows: GND→EV02 GND, 3V3→EV02 VDD,
P1.14→SDA, P1.13→SCL, P1.15→RDY, and P1.12→MCLR (reserved; not driven during
Step 1A). Do not supply the EV02 from USB VBUS (5 V). Add the node in
the actual build overlay is `boards/shields/iqs9150_xiao/iqs9150_xiao.overlay`.
It remaps I2C0 from the board default pins to P1.14/P1.13 and disables SPI2,
which otherwise uses P1.13--P1.15. `rdy-gpios` has no internal pull-up:
IQS9150 RDY is active-low open-drain, and the EV02 pull-up should be used when
fitted.

配線は変更しません。GND→GND、3V3→VDD、P1.14→SDA、P1.13→SCL、P1.15→RDYです。
P1.12は未接続のままにします。EV02へ5 Vを供給しないでください。

Add this repository as a ZMK module (or put it under `config/`), then enable:

```
CONFIG_IQS9150_STEP1A=y
CONFIG_ZMK_POINTING=y
CONFIG_LOG=y
CONFIG_IQS9150_STEP1A_LOG_LEVEL_INF=y
```

For this repository, `build.yaml` already selects `seeeduino_xiao_ble` plus
the `iqs9150_xiao` shield. Build it from the ZMK workspace with:

```
west build -s zmk/app -d build/iqs9150-ev02-step1c -b seeeduino_xiao_ble \
  -S zmk-usb-logging -- \
  -DSHIELD=iqs9150_xiao \
  -DZMK_CONFIG=$PWD/config \
  -DZMK_EXTRA_MODULES=$PWD
```

The USB logging snippet provides a CDC serial port in addition to the normal
ZMK USB functions. Open that port at 115200 baud to read the logs.

USB CDCシリアルポートを115200 baudで開きます。STEP1Bでは、初期化の進捗・ATI完了・
Event Modeへの移行がログに出力されます。

Each active RDY edge queues work outside the ISR. In Step 1C, each non-zero
relative X/Y report is sent through `zmk,input-listener` as a mouse HID report.
At INFO log level, normal movement reports are intentionally silent.

各RDYアクティブエッジは割込み外のワークを起動します。STEP1Cでは、ゼロでない相対X/Yを
`zmk,input-listener` 経由でマウスHIDレポートに変換します。通常の移動ログはINFOレベルでは
出力しません。

## STEP1B 動作確認 / STEP1B verification

ファーム担当の確認済み事項：`seeeduino_xiao_ble`、`iqs9150_xiao` shield、USB logging
を指定したクリーンビルドが成功しています。書込み後に次の順でログを確認してください。

1. `IQS9150 product 0x076a found at I2C 0x56`
2. `STEP1B reset acknowledged; writing EV02 initialization data`
3. `STEP1B initialization data written; starting ATI`
4. `STEP1B ATI requested; waiting for completion`
5. `STEP1B ATI complete; enabling Event Mode`
6. `STEP1B complete: Event Mode active...`、続いてパッド接触時のみ `status=...` 行

`STEP1B ... failed` または上記の途中で止まる場合は、その直前を含むシリアルログを送ってください。
ATI中はパッドに触れず、平らな非導電性の場所に置いてください。

Firmware verification: the clean build succeeds for `seeeduino_xiao_ble` with
the `iqs9150_xiao` shield and USB logging. After flashing, check the six log
messages above in order. Do not touch the pad while ATI is running; keep it on
a flat, non-conductive surface. Send the serial log if a `STEP1B ... failed`
message appears or progress stops.

## STEP1C ポインタ動作確認 / STEP1C pointer verification

ファーム担当の確認済み事項：`ZMK_POINTING` と `zmk,input-listener` を有効化した
クリーンビルドが成功しています。書込み後、USB接続時は画面上のカーソルが指の移動に追従する
ことを確認してください。初期速度はEV02の相対値をそのまま使用します。速すぎる・遅すぎる、
またはX/Y方向が逆の場合は、方向と体感を報告してください。次の調整で係数または軸反転を
加えます。

Firmware verification: a clean build succeeds with `ZMK_POINTING` and
`zmk,input-listener` enabled. After flashing, verify that the USB-connected
host cursor follows one-finger movement. The initial build uses the EV02
relative values without scaling. Report speed and axis-direction preferences;
the next tuning pass can add scaling or inversion.

## STEP1D スクロール・タップ動作確認 / STEP1D scroll and tap verification

ファーム担当の確認済み事項：2本指ジェスチャーのスクロール値を ZMK の縦・横ホイールHIDへ、
タップフラグの立上りをクリックへ変換するクリーンビルドが成功しています。書込み後、次を確認
してください。

1. 2本指で上下移動すると縦スクロールする。
2. 2本指で左右移動すると横スクロールする（対応アプリのみ）。
3. 1本指タップで左クリックする。
4. 2本指タップで右クリックする。

調整版ではスクロール開始距離を8、ホイール出力を元の1/4に設定します。2本指を置いたまま停止・再開しても
相対移動を継続してスクロールへ変換します。接触時間350 ms以下・移動75以下のソフトウェアタップ判定を
使用し、ASICタップフラグは診断ログとしても確認します。クリックが連続発生する
場合、またはスクロールの向き・速度が合わない場合は、操作内容と結果を報告してください。
中クリック、3本指、スワイプ、ピンチはこの版の対象外です。

タップの押下と解放は別々のHID同期レポートとして送信します。これにより、ホストが押下を観測
する前に解放状態へ戻ることを防ぎます。

Firmware verification: a clean build succeeds with two-finger gesture values
mapped to ZMK vertical/horizontal wheel HID and rising tap flags mapped to
clicks. The tuned build uses a scroll-start distance of 8, reduces wheel output
to one fourth. While two fingers remain down, relative movement remains mapped
to scrolling after a pause. Software tap detection uses a 350 ms maximum
contact and 75 maximum movement; ASIC tap flags are retained for diagnostic
logs. After flashing, verify the four items above. Report
unintended repeated clicks or scroll direction/speed issues. Middle click,
three-finger gestures, swipes, and pinch are outside this build's scope.

Tap press and release are sent in separate synchronized HID reports so the host
observes the press before the release.

## 実行中の復帰性 / Runtime recovery

実行中のIQS9150 `Show Reset` フラグを検出した場合、ドライバは進行中の接触セッションを
明示的に取消し、リセットACK、EV02設定書込み、ATI、Event Modeの順で復帰します。取消し用の
内部フレームを機能モジュールへ渡すため、リセット直前の短い接触が左クリックや格子タップへ
誤変換されません。連続3回のレポートI²C読出し失敗も、ソフトウェアリセットを起点に同じ復帰
経路へ入ります。

When the running IQS9150 reports `Show Reset`, the driver explicitly cancels
the active contact session, then recovers through reset acknowledgement, EV02
configuration, ATI, and Event Mode. The internal cancel frame prevents a
short pre-reset contact from becoming an unintended click or grid tap. Three
consecutive I²C report-read failures enter the same recovery path after a
software reset request.

確認時は通常のポインタ・スクロール・タップに加え、ログに `runtime recovery requested` が
出た場合に続いて `STEP1B complete: Event Mode active` まで到達することを確認してください。
復帰中は接触を解除し、ATI完了までパッドに触れないでください。

For verification, test normal pointer, scrolling, and taps. If the log emits
`runtime recovery requested`, it should subsequently reach
`STEP1B complete: Event Mode active`. Release the pad during recovery and do
not touch it until ATI completes.

## UF2書込み後の再起動 / Restart after UF2 update

UF2のコピー後にUSBを抜き差ししないと動作しない場合は、まずUSBを接続したままXIAOのRESETを
1回押してください。UF2コピー後のアプリ起動・USB再列挙はブートローダーとWindows側が関与する
ため、トラックパッドドライバだけでは原因を確定できません。RESETでも毎回必要な場合は、次に
ブートローダーのバージョンと更新可否を個別に調査します。ブートローダー更新は別途明示的な承認が
必要です。

If the board works only after unplugging USB following a UF2 copy, first keep
USB connected and press the XIAO RESET button once. Application startup and USB
re-enumeration after copying UF2 also involve the bootloader and Windows, so the
trackpad driver alone cannot establish the cause. If RESET is required after
every update, the next step is to identify the installed bootloader and assess
an update separately. A bootloader update requires explicit approval.

## EV02 H-file analysis / H-file解析

The supplied `IQS9150_init.h` is the EV02 baseline. Its configuration blocks
are listed below; values are little-endian where a field has `_0`/`_1` bytes.

| Range | Length | EV02 value / purpose |
| --- | ---: | --- |
| `0x115C–0x1175` | 26 | ALP ATI compensation: all zero (ATI-derived) |
| `0x1176–0x1179` | 4 | I2C key/address `ff/ff`; settings version 0.3 |
| `0x117A–0x1195` | 28 | TP ATI mult/div `21 4b`; 13 ALP pairs |
| `0x1196–0x11A1` | 12 | ATI targets 250/300/50; re-ATI 50/1000; drift 50/20 |
| `0x11A2–0x11BB` | 26 | Sampling 10/50/50/50/100 ms; mode timeouts; I2C timeout 100 ms |
| `0x11BC–0x11C1` | 6 | System `4b00`, config `1e46`, other `a880` |
| `0x11C2–0x11CB` | 10 | ALP setup `ffff ffd3`; ALP Tx enable `000000fcff0f` |
| `0x11CC–0x11D3` | 8 | TP thresholds 26/20; ALP 8/8; debounce 2/2; snap 50/50 |
| `0x11D4–0x11D7` | 4 | ALP count/LTA betas `46 04 b4 64` |
| `0x11D8–0x11E1` | 10 | Hardware: `28 02 02 10 07 07 00 44 40 4b` |
| `0x11E2–0x11F5` | 20 | 26 Rx, 18 Tx, max. 7 fingers; X/Y 2000/1213; filters, trims 20/20 |
| `0x11F6–0x1217` | 34 | Gesture enable and tap/swipe/scroll/zoom/palm parameters |
| `0x1218–0x1245` | 46 | Rx/Tx map: 44 assignments plus two open bytes |
| `0x1246–0x129D` | 88 | All channel-disable bytes zero |
| `0x129E–0x12F5` | 88 | All snap-enable bytes zero |
| `0x12F6–0x14EF` | 506 | Individual threshold adjustments: zero |
| `0x14F0–0x15EB` | 252 | Virtual button/slider/wheel configuration |
| `0x2000–0x2005` | 6 | Engineering configuration: `01 00 03 03 14 00` |

System Control (`0x11BC`) is handled as control actions, not copied from the
H-file: reset ACK is written as `0x0082`, and only Trackpad re-ATI is requested.
All other listed H-file blocks are written by STEP1B in chunks of at most 30
data bytes per RDY window. Event Mode is set only after ATI reports completion.

System Control (`0x11BC`) はH-file値をそのまま書き込まず、制御操作として扱います。
リセットACKは `0x0082`、ATIはTrackpadのみを要求します。その他のH-fileブロックはRDY
ウィンドウごとに最大30データバイトずつ書き込み、ATI完了を確認してからEvent Modeを有効化します。
