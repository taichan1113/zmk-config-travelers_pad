# IQS9150 router-to-pointer hardware test / router→ポインタ実機試験

## Scope / 対象範囲

This UF2 enables the IQS9150 driver, multi-finger router, and pointer function.
The route is `IQS9150 → router → pointer`. The tap/drag tracker is present only
because the router observes its ownership state; its HID listener is disabled.
Grid/edge input and gesture KSCAN are disabled. This is the router's
output-contract test, not a full feature integration test.

このUF2ではIQS9150ドライバ、多指router、ポインタ機能を有効にする。経路は
`IQS9150 → router → pointer` である。tap/drag trackerはrouterが所有状態を参照する
ためだけに存在し、HID listenerは無効である。格子/辺入力、ジェスチャーKSCANは無効。
これはrouterの出力契約を検証する試験であり、全機能の結合試験ではない。

## Test sequence / 試験手順

1. Flash `travelers-pad-router-pointer-isolated.uf2`, reconnect USB, and open
   the serial log. Wait for `STEP1B complete: Event Mode active`.
   `travelers-pad-router-pointer-isolated.uf2`を書き込み、USBを再接続してシリアル
   ログを開く。`STEP1B complete: Event Mode active` を待つ。
2. Move one finger in several directions. The pointer must follow normally.
   1本指を複数方向へ動かす。通常どおりポインタが追従すること。
3. Perform five stationary two-finger touches. Each must log `2F route=tap
   candidate`; it must not move the pointer or scroll.
   静止した2本指接触を5回行う。各回で `2F route=tap candidate` が出て、ポインタ
   移動・スクロールが発生しないこと。
4. Perform five vertical and five horizontal two-finger scrolls, lifting both
   fingers between direction changes. Confirm `2F route=scroll committed` and
   the pointer module's matching axis-lock log. No pointer movement is allowed.
   縦・横をそれぞれ5回ずつ2本指スクロールする。方向を変える前に両指を離す。
   `2F route=scroll committed` とポインタの対応する軸lockログが出て、ポインタ移動が
   発生しないこと。
5. Perform a two-finger pinch and a two-finger rotation. Each must log `2F
   route=geometry claimed` and must emit neither pointer movement nor scroll.
   2本指ピンチと2本指回転を行う。各操作で `2F route=geometry claimed` が出て、
   ポインタ移動・スクロールがともに発生しないこと。
6. Perform a three-finger swipe and a five-finger grasp/release. The router
   must claim them (`3F route=multifinger claimed`, `5F route=Grasp/Release
   claimed`) and no pointer/scroll output may be emitted.
   3本指スワイプと5本指grasp/releaseを行う。routerがそれぞれをclaimし、ポインタ／
   スクロール出力が出ないこと。

## Evidence / 判定ログ

For a failure, keep roughly two seconds of serial output around the operation,
including the router line and any `Mouse movement set` / `Mouse scroll set`
lines. The known QSPI JEDEC warning is outside this test's scope.

失敗時は操作前後およそ2秒のシリアルログを保存し、routerの行と `Mouse movement set` /
`Mouse scroll set` を含める。既知のQSPI JEDEC警告は本試験の対象外である。
