# IQS9150EV02 / XIAO nRF52840 — Step 1A / Step 1B

## 現在の状態 / Current status

STEP1Aでは実機でI²C、RDY割込み、タッチ座標の生データ取得を確認済みです。STEP1Bは
`IQS9150_init.h` のEV02設定を書き込み、ATI（自動調整）完了後にEvent Modeへ移行します。
この段階でもマウス/HIDイベントは出力しません。

Step 1A has verified I²C, RDY interrupts, and raw touch reports on hardware.
Step 1B writes the EV02 settings from `IQS9150_init.h`, runs ATI, and enters
Event Mode. It still does not emit mouse/HID events.

This is a ZMK/Zephyr module for safe, read-only bring-up of an IQS9150EV02.
It deliberately does **not** reset the IC, acknowledge a reset, write the GUI
exported configuration, run ATI, or emit mouse/HID events.

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
CONFIG_LOG=y
CONFIG_IQS9150_STEP1A_LOG_LEVEL_INF=y
```

For this repository, `build.yaml` already selects `seeeduino_xiao_ble` plus
the `iqs9150_xiao` shield. Build it from the ZMK workspace with:

```
west build -s zmk/app -d build/iqs9150-ev02-step1a -b seeeduino_xiao_ble \
  -S zmk-usb-logging -- \
  -DSHIELD=iqs9150_xiao \
  -DZMK_CONFIG=$PWD/config \
  -DZMK_EXTRA_MODULES=$PWD
```

The USB logging snippet provides a CDC serial port in addition to the normal
ZMK USB functions. Open that port at 115200 baud to read the logs.

USB CDCシリアルポートを115200 baudで開きます。STEP1Bでは、初期化の進捗・ATI完了・
Event Modeへの移行がログに出力されます。

Expected startup output includes `product=0x076a` followed by `waiting for
RDY`. Each falling RDY edge queues work outside the ISR and logs relative XY,
gesture XY, info flags, trackpad flags, finger count, and finger 1 absolute
coordinates. No pointer events are emitted.

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
| `0x11E2–0x11F5` | 20 | 26 Rx, 18 Tx, 3 fingers; X/Y 2000/1213; filters, trims 20/20 |
| `0x11F6–0x1217` | 34 | Gesture enable and tap/swipe/scroll/zoom/palm parameters |
| `0x1218–0x1245` | 46 | Rx/Tx map: 44 assignments plus two open bytes |
| `0x1246–0x129D` | 88 | All channel-disable bytes zero |
| `0x129E–0x12F5` | 88 | All snap-enable bytes zero |
| `0x12F6–0x14EF` | 506 | Individual threshold adjustments: zero |
| `0x14F0–0x15EB` | 252 | Virtual button/slider/wheel configuration |
| `0x2000–0x2005` | 6 | Engineering configuration: `01 00 03 03 14 00` |

System Control (`0x11BC`) is not blindly written: the driver uses
read-modify-write to acknowledge reset and request ATI, as required by the
datasheet. All other listed H-file blocks are written by STEP1B in chunks of
at most 30 data bytes per RDY window. Event Mode is set only after ATI reports
completion.

System Control (`0x11BC`) はH-file値をそのまま書き込まず、データシートの指定どおり
read-modify-writeでリセットACKとATI要求を行います。その他のH-fileブロックはRDYウィンドウ
ごとに最大30データバイトずつ書き込み、ATI完了を確認してからEvent Modeを有効化します。
