## English

# Startup ATI recovery

## Purpose

Verify that touching the pad immediately after UF2 programming does not leave
the IQS9150 in the permanent error state. A contact during the initial ATI can
make the sensor report `TP_REATI_OCCURRED | TP_ATI_ERROR` (typically
`Info Flags=0x0118`). The driver must reset, reapply the EV02 image, and reach
Event Mode automatically, without a physical RESET press.

## Procedure

1. Program the startup-recovery UF2, and let the USB drive disconnect normally.
2. Without pressing the XIAO RESET button, touch and hold the pad within about
   one second of the application boot, then release it after 0.5--1 second.
3. Wait up to ten seconds. Confirm that the pointer works normally.

## Expected serial evidence

Either a normal startup is valid, or a recovery sequence is valid:

* Normal: `STEP1B complete: Event Mode active`.
* Recovery: `STEP1B ATI error ... startup recovery 1/8`, followed by
  `runtime recovery: software reset requested`,
  `runtime recovery: Show Reset observed; acknowledging reset`, and finally
  `STEP1B complete: Event Mode active`.

`STEP1B ATI error ... startup recovery 1/8`、
`runtime recovery: software reset requested`、

## Fail criteria

## 日本語

# 起動時ATI自動復旧

## 目的

UF2書込み直後にパッドへ触れても、IQS9150が恒久的なエラー状態に残らないことを
確認する。初回ATI中の接触ではセンサが `TP_REATI_OCCURRED | TP_ATI_ERROR`
（典型例: `Info Flags=0x0118`）を通知し得る。driverは物理RESETなしでreset、
EV02 image再適用、Event Mode到達までを自動で行わなければならない。

## 手順

1. UF2を書き込み、USB driveが通常どおり切断されるまで待つ。
2. XIAOのRESETは押さず、application起動からおよそ1秒以内にパッドへ触れ、
   0.5--1秒後に離す。
3. 最大10秒待ち、ポインタが通常どおり動くことを確認する。

## 期待するシリアルログ

正常起動では `STEP1B complete: Event Mode active` が出る。復旧した場合は
`runtime recovery: Show Reset observed; acknowledging reset` の後に、最終的な
`STEP1B complete: Event Mode active` が出る。

## 失敗判定

`STEP1B ATI recovery retry limit` または `STEP1B ... failed` で停止すること、
あるいは物理RESETを押さないとポインタが復帰しないことは失敗である。
