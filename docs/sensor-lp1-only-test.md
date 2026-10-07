# LP1-only comparison

## English

The Idle-limited firmware passed battery/BLE idle tests including approximately
33 minutes without contact. History shows input/output before USB reconnection,
errors=0, and no additional rescue after the startup count of 1. Preserve that
UF2 and branch as the accepted fallback; this does not prove a specific LP cause.

This test uses `power-mode-limit = "lp1"`: zero only **0x11B2–0x11B3** (LP1
Mode Timeout), blocking automatic LP1-to-LP2. Other timeout words remain at EV02
values, including Idle-to-LP1=5s and Active-to-Idle=1500ms. LP1 sampling remains
50ms. Manual control, automatic reference handling, features, BLE settings and
RAM history are unchanged. No runtime polling/reset is added. USB remains
power/serial only and HID remains BLE only.

The driver adds a build-time `power-mode-limit` enum: none (default, original
timeouts), active, idle, lp1. Original initialization arrays are unchanged.
Legacy diagnostic flags retain precedence for older tests. This overlay removes
both legacy flags. RDY-gated readback must verify the modified two bytes before
ATI. The boot confirmation is:

```text
power limit lp1: LP1-to-LP2 timeout verified zero; Active/Idle/LP1 transitions retained
```

UF2: `C:\Users\takec\Downloads\travelers-pad-sensor-lp1-only-20261007.uf2`.
Driver/pad branch: `debug/sensor-lp1-only`; features remain at `13d16ea`.
Local module overrides are used, not a remote manifest revision update.

1. Flash and capture the confirmation above; confirm pointer/taps/scroll on BLE
   after disconnecting USB.
2. Leave untouched for 5 minutes. Before USB/RESET, try pointer for 30 seconds,
   taps and two-finger scroll. If these fail, stop and capture history via USB.
3. If successful, repeat after 30 minutes untouched and capture history afterward.
4. Share whether/when it stopped and the complete history. Current sensor mode
   is not directly recorded, so even success is not proof of LP1 residency.

Failure while Idle-only passes implicates the LP1 entry/wake path; success points
toward the deeper LP2 path but needs repetition and mode confirmation. Neither
rules out supply/electrical conditions. Current consumption remains unmeasured.
Fallback: `travelers-pad-sensor-idle-only-20261007.uf2`.

## 日本語

Idle維持版はバッテリー/BLEで約33分無接触後の操作にも合格。historyでUSB再接続前の
入力/出力、errors=0、起動時rescue=1から増加なしを確認しました。合格UF2/ブランチは
戻し先として保持します。原因LP段階の断定はまだしません。

今回は `power-mode-limit = "lp1";` を指定し、**0x11B2–0x11B3のLP1 Mode Timeoutのみ0**
にして自動LP1→LP2を停止します。他のtimeout（Idle→LP1=5秒、Active→Idle=1500msを含む）
はEV02初期値を維持。LP1 samplingは50msのままです。自動reference管理、機能、BLE設定、
RAM historyは変更せず、runtimeポーリング/定期リセットは追加しません。
USBは給電/シリアルのみ、HIDはBLEのみです。

driverにビルド時選択 `power-mode-limit`（none既定/active/idle/lp1）を追加しました。
元の初期化配列は変更しません。旧診断設定は互換性のため優先されるので、今回overlayでは
両方削除しています。ATI前のRDY通信窓で対象2バイトのreadbackが必要です。確認行：

```text
power limit lp1: LP1-to-LP2 timeout verified zero; Active/Idle/LP1 transitions retained
```

UF2：`C:\Users\takec\Downloads\travelers-pad-sensor-lp1-only-20261007.uf2`。
driver/pad：`debug/sensor-lp1-only`。features：`13d16ea` のまま。
ローカルmodule overrideでビルドし、remote manifestは更新しません。

1. 書き込み後に確認行を保存。USBを外しBLEでポインタ・タップ・スクロールを確認。
2. 5分無接触で放置。USB/RESET前に1本指移動30秒、タップ・2本指スクロールを確認。
   止まった場合はその時点でUSB接続してhistoryを保存。
3. 成功したら30分無接触でも同様に確認してからUSB接続しhistory保存。
4. 停止の有無・タイミングとhistory全文を共有。現在modeは記録していないので、
   成功してもLP1に実際に入ったことの直接確認ではありません。

Idle維持で成功し今回失敗ならLP1移行/復帰経路が候補。今回成功なら深いLP2経路が候補ですが、
再現性とmode確認が必要です。給電/電気的条件の影響は除外できません。消費電流も未測定。
戻し先：`travelers-pad-sensor-idle-only-20261007.uf2`。
