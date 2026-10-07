# ALP-off production candidate

## English

The diagnostic ALP-off image passed two battery/BLE idle-resume trials: about
5m27s and 6m23s without contact, followed by a one-finger contact and pointer
output before USB reconnect. Errors and rescues were zero. Initial stutter was
not repeatable; cause and battery consumption remain unknown. ALP's exact
internal failure mechanism and LP2 residency are not proven by cached history.

The production candidate uses the normal shield/build.yaml configuration, not
test overlays or /tmp module overrides. Both private module manifests reference
tag v0.2.0: driver aacfbc4, features 13d16ea. Only the Pad configuration disables
ALP; the reusable driver's default remains unchanged. Other Settings=0x80A8,
ALP Setup=0x53FFFFFF and unrestricted automatic LP2 entry are retained. Config
Settings after Event Mode is 0x471E. Startup verification must finish without
contact; no periodic runtime I2C or reset is introduced.

USB supplies power and serial logging; HID uses BLE only with requested latency
zero, matching the accepted diagnostic runs. RAM idle history and driver contact/
runtime diagnostic flags are disabled. Startup verification logs remain enabled.
The property diagnostic-disable-alp is an explicit interim policy, not a claim
that the hardware root cause is conclusively fixed or measured for power usage.

Final acceptance before main integration:

1. Flash travelers-pad-alp-off-production-candidate-20261007.uf2. Leave untouched
   until final power setup verification and STEP1B complete appear. Expected
   Other=0x80A8 and ALP=0x53FFFFFF. Report any reset needed.
2. Disconnect USB. Check pointer, taps/double-tap drag, two-finger scrolling and
   inertia, then an edge operation and a gesture. Report regressions/stutter.
3. Leave untouched five minutes. Without USB/RESET, start with one-finger
   movement for 30s, then tap and two-finger scroll. Four-finger-only wake fails.
4. Reconnect USB and verify input continues. This image has no RAM history;
   report observed behavior and supply boot/error logs if available. If this
   fails, return to the accepted diagnostic image to capture history.

Do not merge to main until this production-candidate check passes.

## 日本語

ALP無効診断版はバッテリーBLEで約5分27秒・6分23秒の無接触後、USB再接続前に1本指から
接触・ポインタ出力が再開し、2回合格。エラー・救済0。初動カクツキは常時再現せず、原因・
消費電力は未確定。キャッシュhistoryからALPの内部原因・LP2滞在を確定したわけではありません。

本番候補は通常のshield/build.yaml設定を使用し、テストoverlayや/tmpモジュール上書きを
使用しません。両privateモジュールはタグv0.2.0（driver aacfbc4、features 13d16ea）参照。
ALP無効はPad設定のみで採用し、再利用driverの既定値は変更しません。
Other=0x80A8、ALP=0x53FFFFFF、LP2自動移行許可を維持。Event Mode後のConfig=0x471E。
無接触で起動検証を完了させ、通常稼働中の定期I2C・RESETは追加しません。

合格版に合わせUSBは給電・シリアル、HIDはBLEのみ、要求latency=0。
RAM履歴、driver接触座標・health診断は無効化。起動時の読み戻しログは残します。
diagnostic-disable-alpは明示的な暫定対策として使用し、内部原因や電池消費の確定を意味しません。

main統合前の最終実機確認：

1. travelers-pad-alp-off-production-candidate-20261007.uf2を書き込み、触れずに
   final power setup verified と STEP1B complete を確認。Other=0x80A8、ALP=0x53FFFFFF。
   RESETが必要だった場合は報告。
2. USBを外し、ポインタ・タップ/ダブルタップドラッグ・2本指スクロールと慣性、
   エッジ操作とジェスチャーを簡易確認。退行・カクツキがあれば報告。
3. 5分放置後、USB/RESETなしでまず1本指移動30秒、タップ・2本指スクロール。
   4本指だけで復帰する場合は不合格。
4. USB再接続でも操作が続くことを確認。本版はRAM履歴なし。体感結果と可能なら
   起動/エラーログを提出。失敗時は合格した診断版に戻してhistory採取。

この本番候補の実機合格前にmainへ統合しないでください。
