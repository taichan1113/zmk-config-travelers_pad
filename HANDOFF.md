[English](#english) | [日本語](#日本語)

# Project handoff

## English

Last updated: 2026-10-07

Active development: BLE management gestures on feat/ble-management-layer.
The grid Stage 1 passed hardware, but was rejected for poor cell discoverability.
Grid stream/listener and mapped cells are now disabled. Management 4F swipes:
left BT_NXT, right BT_PRV, up BT_SEL 0, down once none, twice BT_CLR via standard
tap dance (1500ms). Hold/release enters/exits; switching/clear stays in management.
Only the selected profile is erased; testing must use a disposable pairing.
Gesture revision hardware is pending. Driver/features unchanged for this change.
Original resumption details: Stage 1 resumed on feat/ble-management-layer,
features feat/four-finger-hold (8dd8305) after merging the accepted main fixes.
Driver remains v0.2.0; ALP-off and BLE-only HID are retained. Four-finger hold
opens a 2x3 host-selection/exit grid; BT_CLR is deliberately unassigned until
entry/exit and selection pass hardware. Build and production-C host tests pass;
hardware pending. Travelers-system is not changed. See
[management checks](docs/ble-management-layer.md).

Latest accepted baseline: Pad/driver/features local main. The production
candidate passed all four user checks on 2026-10-07; startup completed at
2.709s with Other=0x80A8 / ALP=0x53FFFFFF. The boot log has the pre-existing
QSPI JEDEC mismatch and one successful startup RDY level rescue, not a startup
failure. No new blocker was found. Integration does not change the tested UF2.
Older investigation paragraphs below are historical; pending statements there
are superseded by this latest acceptance. Remote main push is not included.
ALP-off passed two battery/BLE idle-resume runs
(5m27s and 6m23s), each starting with one finger before USB reconnect, errors/
rescues zero. Production configuration now disables ALP and contact/runtime/RAM
diagnostics; mandatory startup verification remains. HID stays BLE-only with
requested latency zero. Private driver/features tags v0.2.0 are published and
referenced by west (driver aacfbc4 / features 13d16ea). Final normal-build
hardware regression passed and main integration is approved. See
[production check](docs/alp-off-production-regression.md).
Post-Event verification passed on hardware at
2.708s without contact, but Auto-Prox-off alone failed idle resume: no raw input
until a four-finger contact, then one-finger input resumed. Retract permanent-fix
acceptance of Auto-Prox-off. Initial USB-disconnect stutter is unresolved; its
time interval was overwritten in the 96-sample history. The new comparison
retains the startup scheduling fix, disables ALP (0x53FFFFFF), restores Other
Settings 0x80A8 and permits LP2. Features unchanged; no runtime polling added.
Diagnostic hardware acceptance passed. See [ALP-off test](docs/lp2-alp-off-test.md).

### Project status

The Travelers Pad configuration, IQS9150 driver, and trackpad function module
are maintained as three repositories. Their current READMEs are being prepared
for eventual public release. The two reusable module repositories are private
at present. No repository has been made public during this documentation pass.

### Active investigation: idle resume

Current diagnostic branch: `debug/ble-idle-isolation`. Driver manifest revision:
`fix/warm-reset-startup` (commit `aa6a049`); features use `fix/inertia-cancel-state`.
The latest RESET log shows product `0xEEEE` throughout startup, ERROR state 11,
and zero reports despite a configured USB endpoint. Startup now requests RDY
windows and explicitly resets the sensor before initialization. Hardware
acceptance passed: the latest boot log identifies the product, verifies reset,
completes ATI at 2.61 seconds and then emits pointer movement; see
[warm-reset startup](docs/warm-reset-startup.md).
The features change clears the transient inertia emission flag on cancellation,
so a revoked glide does not suppress future wheel velocity observations.
The old source fails the new regression; the fix passes 256 cancel/restart cycles.
See [inertia cancellation recovery](docs/inertia-cancel-recovery.md).
The repeated Show Reset warning was traced to an incorrect software reset command
(`0x0002` instead of documented `0x0200`). Reset verification now has a deadline;
RDY logical polarity and termination-bit detection are corrected, with GPIO-only
level rescue. Host state-machine tests and firmware build pass; hardware idle/resume
acceptance is still pending. USB idle/resume passed a three-minute user check,
but battery-only BLE stopped after three minutes and recovered on USB attachment.
The post-attachment log shows normal driver state, not the state while stopped.
The BLE-only HID diagnostic retains USB CDC/power so transport switching cannot
mask the failure. See [BLE idle isolation](docs/ble-idle-isolation.md).
The original trigger of recovery remains unconfirmed.

The latest battery-only repeat lost pointer/tap/scroll while Windows still showed
connected. USB reconnection recovered BLE output; its first driver health line
showed 549 seconds since the last successful sensor report, without reset/recovery.
The BLE-only diagnostic now retains RAM history across USB removal to distinguish
RDY silence, system-workqueue stalls, raw-frame progression and BLE connection
state before power/USB attachment changes the condition. No automatic fix has
been applied to this still-open battery-only issue.

Latest follow-up: the host accepted `lat=0`, but continuous movement still stopped.
At 22–27 seconds raw frames increased 584→980 while REL stayed 819; one sample
reported two fingers during the intended one-finger test. Coordinates were not
recorded, so neither frozen sensor coordinates nor routing suppression can yet
be confirmed. The new diagnostic retains latency zero and adds `flow` history
for raw coordinate changes, contact-count changes, forwarded pointer frames and
separate XY/wheel counts. Build and arbiter regressions pass; hardware pending.
UF2: `C:\Users\takec\Downloads\travelers-pad-ble-input-flow-20261007.uf2`.
Next: repeat immediate USB-removal central one-finger motion; keep moving ten
seconds after stopping, then collect history without RESET. Idle testing waits.

Input-flow follow-up: raw coordinates continue moving while routing pauses
(27.362s raw=1335/raw-move=1332, routed=1219, routed-fingers=0, marker=1).
The user reports faster movement can stop pointer and lifting/recontact restores
it. Router code latches AWAIT_TWO after a >=2 count and does not resume ordinary
pointer at count one until full release; a transient extra contact is suspected
but not yet captured. Do not broadly resume multi-to-one: preserve gesture isolation.
Latest UF2: `C:\Users\takec\Downloads\travelers-pad-contact-transitions-20261007.uf2`.
It adds a 96-entry raw count-change/reset ring, including both contact coordinates
and second validity, printed inside the history dump. No routing changes.
Next: one finger down, slow then fast motion, ten seconds of motion after stop,
one deliberate lift/recontact, USB attach without RESET; save the whole dump.

Contact-transition capture now confirms 1→2 at 20.355s, 2→1 at 20.367s (12ms),
and XY frozen at 1240 until the deliberate release at 32.598s/recontact at 33.097s.
Numerous later two-contact bursts last 12–26ms. Some count-one frames have
Finger 1=65535/65535 while Finger 2 retains coordinates. Official datasheet 7.2.6
states that tracked fingers retain their memory-map location; active count is
not a valid upper bound on slot indices. Current driver uses `index < fingers`
for slot validity and hardcodes legacy position to slot zero: both need review.
No fix applied yet. Next: correct sparse-slot normalization while preserving
tracking identity and rebasing representative changes; separately design short
extra-contact filtering/rollback without bypassing committed gesture ownership.
This reproduction implicates the input interpretation path, not a BLE disconnect;
the older long-idle/no-RDY failure remains separately unproven.

Implemented candidate (hardware pending): driver `fix/sparse-contact-slots` and
features `fix/transient-contact-count`, in the existing /tmp worktrees. The driver
decodes every physical slot, publishes only confident/present contacts, and
chooses a sticky valid legacy representative. Features buffer complete frames
in the session manager and confirm 1→2 additions for 35ms (DT
`extra-contact-confirm-ms`, 0 disables). Initial multi-touch/release are immediate.
All recognizers see the same filtered slots/counts; arbiter ownership is unchanged.
Representative changes rebase ordinary pointer and drag motion without reset.
Production decoder/session tests and existing recovery/inertia/architecture/
dependency checks pass, and combined UF2 builds. Manifest/remote branches stay
unchanged pending acceptance; the build uses local ZMK_EXTRA_MODULES overrides.
UF2: `C:\Users\takec\Downloads\travelers-pad-contact-stability-fix-20261007.uf2`.
See [contact stability](docs/contact-slot-stability.md). First check continuous
fast 1F motion plus 2F scroll/pinch, Tip-Tap and tap/drag; then resume battery-only
3–5min/30min idle investigation. No idle fix is claimed.

Contact stability hardware acceptance is now confirmed. The old-firmware idle
capture plus user timing confirms no RDY/raw progression during the final 30s of
attempted motion, while BLE/workqueue remain alive; USB restores operation.
Next comparison: driver/pad `debug/sensor-active-only`, features unchanged at
`13d16ea`. An optional driver diagnostic zeros only 0x11AC–0x11B5 mode timeouts
and verifies readback before ATI. It retains automatic reference management,
sampling, Event Mode, latency zero and RAM history; no runtime polling/reset.
UF2: `C:\Users\takec\Downloads\travelers-pad-sensor-active-only-20261007.uf2`.
Test 5min then 15min idle on battery only; before USB/RESET, attempt pointer/tap/
scroll, then dump history. Increased battery consumption; not a production fix.
See [active-only comparison](docs/sensor-active-only-test.md).

Active-only hardware comparison passed 5min/15min idle. History confirms battery
input/output resumed before USB, with errors/rescues zero. Next narrower test:
driver/pad `debug/sensor-idle-only`, features unchanged. Zero only Idle Mode
Timeout 0x11B0 (Idle-to-LP1), verify two-byte readback before ATI, retain other
timeouts and automatic reference management. No exact mode/root cause is proven.
UF2: `C:\Users\takec\Downloads\travelers-pad-sensor-idle-only-20261007.uf2`.
See [Idle-only comparison](docs/sensor-idle-only-test.md); hardware acceptance pending.

Idle-only is now hardware accepted, including approximately 33min without contact;
history confirms battery input/output before USB, errors=0 and rescues unchanged
at 1 (startup). Preserve `debug/sensor-idle-only` and its UF2 as fallback.
Current driver/pad branch is `debug/sensor-lp1-only`; features remain at `13d16ea`.
Driver adds `power-mode-limit` enum none/active/idle/lp1 (none default), retaining
legacy diagnostic precedence. The test selects lp1 and zeros/verifies only
0x11B2–0x11B3 before ATI. Other timeouts/reference/sampling remain unchanged.
UF2: `C:\Users\takec\Downloads\travelers-pad-sensor-lp1-only-20261007.uf2`.
See [LP1-only comparison](docs/sensor-lp1-only-test.md); hardware acceptance pending.

LP1-only 5min hardware check passed, with a small wake lag; 30min was explicitly
skipped. errors=0, rescues=1 unchanged from boot. Last-mode residency and physical
touch-to-output delay remain unmeasured. Current driver/pad branch:
`debug/sensor-mode-observation`. Optional `power-diagnostics` reads core power/ALP
settings before ATI and caches Info Flags from existing runtime reports. RAM
history adds timestamped sensor mode/ALP flags/counters; no runtime I2C is added.
This UF2 restores unrestricted LP2 (none). Findings: ALP enabled, LP2 Auto-Prox
enabled but LP1 Auto-Prox disabled. Not a root-cause proof. See
[passive mode diagnostic](docs/sensor-mode-observation-test.md) for limitations,
5min steps and UF2/fallback. Hardware acceptance pending; no main merge/push.

Unrestricted passive diagnostic failed after idle: last report ~71s, stale mode
Active age~539s at ~610s, raw unchanged, BLE/queue alive, errors=0 and rescue=1.
USB had not restored reports within the captured history. Do not infer current
mode from cached Active. Current driver/pad: `debug/lp2-autoprox-off`. Clear only
0x11C0 bit5, full Other Settings readback must equal 0x8088 before ATI; all mode
timeouts and ALP settings otherwise retained. LP2 remains allowed. Passive
history remains enabled. See [Auto-Prox-off comparison](docs/lp2-autoprox-off-test.md)
for UF2 and 5min steps. Hardware acceptance pending; keep Idle fallback.

Correction: old Auto-Prox-off hardware failure is invalid as that isolation:
Event Mode wrote four bytes and restored Other Settings after the pre-ATI check.
Current `debug/lp2-alp-off` fixes Event Mode to write only Config bytes, adds
post-Event readback, and clears only ALP Setup bit31 for the primary test. LP2
and baseline Auto-Prox remain allowed. A corrected Auto-Prox-off UF2 is supplied
separately. See [ALP-off and correction](docs/lp2-alp-off-test.md). Corrected
Auto-Prox-off has since passed the user's idle check; ALP-off is still untested.
Idle-only acceptance remains valid (its timeout is not overwritten).

BLE management work is paused and preserved separately on `feat/ble-management-layer`
(Pad) and `feat/four-finger-hold` (features). It is not included in this diagnostic
firmware. Do not merge to main or resume management changes before checking the
[idle/resume procedure](docs/idle-resume-recovery.md).

### Hardware configuration

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

現在の開発：feat/ble-management-layerでBLE管理ジェスチャー版。
格子版第1段階は実機合格したが区画位置のUXが不適で撤回し、格子転送・割当を無効化。
管理中の4本指は左BT_NXT、右BT_PRV、上BT_SEL 0、下単発なし、下2回BT_CLR
（ZMK tap dance、1500ms）。ホールド解放で入退場、切替・消去後も管理を維持。
選択中のみ消去。消してよいペアリングで確認すること。新版の実機未確認。
今回driver/featuresの追加変更なし。
再開時の経過：BLE管理第1段階のfeaturesは
feat/four-finger-hold（8dd8305）、合格済みmain修正を両方へmerge済み。
driverはv0.2.0、ALP無効・BLEのみHIDを維持。4本指ホールドで2x3の接続先選択/終了格子。
入口・終了・選択の実機合格までBT_CLRは未割当。ビルド・実コードホストテスト合格、
実機未確認。travelers-systemは変更なし。[管理確認](docs/ble-management-layer.md)参照。

最新の合格基準：Pad/driver/featuresのローカルmain。2026-10-07に本番候補の4項目すべて
実機合格。起動2.709秒でOther=0x80A8 / ALP=0x53FFFFFFを確認。既知のQSPI JEDEC不一致と
起動時RDYレベル救済1回は残るものの、起動失敗・新たな阻害要因なし。統合でUF2は変更しない。
以下の調査経過は履歴であり、古い未確認表記はこの最新合格を優先。remote mainのpushは含まない。
ALP無効版はバッテリーBLEで5分27秒・6分23秒放置後に
USB再接続前の1本指から復帰し2回合格、エラー・救済0。本番設定へALP無効を反映し、
接触座標/health/RAM診断は無効化。必須の起動検証は維持。HIDはBLEのみ、要求latency=0。
既存privateリポジトリへdriver/featuresタグv0.2.0をpushしwestで参照
（driver aacfbc4 / features 13d16ea）。通常ビルド本番候補の最終実機は合格、main統合承認済み。
[本番確認](docs/alp-off-production-regression.md)参照。
最終設定確認は無接触で実機2.708秒に完了し起動修正は合格。
ただしAuto-Prox停止だけでは放置後の1本指入力に反応せず、4本指の生接触を契機に復帰。
恒久対策としての合格は撤回。USB切断直後のカクツキは該当時間帯のhistory上書きで原因未確定。
今回の比較版は起動修正を維持しALP無効（0x53FFFFFF）、Other=0x80A8、LP2許可へ変更。
機能は不変、runtimeポーリング追加なし。診断版は実機合格。[ALP比較手順](docs/lp2-alp-off-test.md)参照。

# 引継ぎ書

更新日: 2026-10-07

### プロジェクトの状態

Travelers Pad設定、IQS9150 driver、トラックパッド機能moduleは、それぞれ別の3リポジトリで管理しています。
現在、それぞれのREADMEを将来の公開に向けて整備中です。
再利用moduleの2リポジトリは現時点でprivateです。今回の文書整備では公開化していません。

### 現在の調査：放置後の復帰

診断ブランチは `debug/ble-idle-isolation` です。driverのmanifest参照は
`fix/warm-reset-startup`（commit `aa6a049`）、featuresは `fix/inertia-cancel-state` です。
最新RESETログは製品番号が全て `0xEEEE`、state 11、レポート0で、USB構成は正常でした。
起動時のRDY窓要求と、初期化前の明示的センサーリセットを追加しました。
最新の起動ログで製品番号取得・reset確認・2.61秒でのATI完了・ポインタ出力が確認でき、
RESET後の起動は合格です。[RESET後の起動](docs/warm-reset-startup.md)を参照してください。
featuresは慣性キャンセル時にも一時的な出力中フラグを解除し、次の速度観測を再開します。
慣性キャンセル後の実機再開も確認済みです。一方、USBで3分放置後の復帰は通りましたが、
バッテリー/BLEのみでは3分後に停止し、USB接続で復帰しました。
接続後のdriverは正常ですが、停止中の状態は不明です。USBをCDC・給電のみとし、
HIDをBLEに限定した診断版で切り分けます。[BLE放置停止の切り分け](docs/ble-idle-isolation.md)を参照してください。

最新のバッテリー試験ではWindows接続表示を維持したまま移動・タップ・スクロールが停止し、
USB再接続でBLE出力が復帰しました。接続直後のhealthは最後のセンサーレポートから549秒、
reset/recoveryはありません。USB接続が状態を変える前のRDY・raw frame・queue・BLE状態を
RAMに保持する診断を追加しました。バッテリーのみの停止について、自動復旧などの確定対策はまだ実施していません。

追加確認では実際の `lat=0` が反映されても連続移動が停止しました。
起動22〜27秒でrawは584→980に増え、RELは819のままです。1本指試験中にも1サンプルは2本を記録しています。
座標未採取のため、センサー座標の停止か経路抑制かはまだ未確定です。
次の診断版はlatency=0を維持し、生座標変化・指本数変化・転送ポインタフレーム・XY/wheel別回数を
`flow` 履歴へ追加します。ビルド・arbiter回帰試験は通過し、実機確認待ちです。
UF2は `C:\Users\takec\Downloads\travelers-pad-ble-input-flow-20261007.uf2`。
次はUSB取り外し直後から中央で1本指を動かし、停止しても10秒動かし続け、RESETせず履歴取得します。
直後の停止を切り分けるまで放置テストは保留します。

入力経路の追加確認では生座標変化が続く一方、転送が停止する区間を確認しました
（27.362秒 raw=1335/raw-move=1332、routed=1219、routed-fingers=0、marker=1）。
速い操作で止まりやすく、離して再接触すると復帰するとの報告です。
routerは本数2以上でAWAIT_TWOになり、その後1本でも全接触解除までは通常ポインタへ戻しません。
一時的な余分な接触を疑いますが、まだ瞬間の本数を採取できていません。
多指→1本を全て戻す修正はgesture分離を壊す恐れがあるため実施していません。
最新UF2は `C:\Users\takec\Downloads\travelers-pad-contact-transitions-20261007.uf2`。
指本数変化/resetの直近96件（2点の座標・2点目有効フラグを含む）を履歴へ追加し、経路は変更しません。
次は1本指を置いたまま遅い→速い移動、停止後も10秒移動、一度だけ離して再接触で復帰確認、
RESETせずUSB接続して履歴全体を保存します。

指本数履歴で20.355秒の1→2、20.367秒の2→1（12ms）を確認し、XY=1240は
32.598秒の意図的な解除・33.097秒の再接触まで停止しています。後続にも12〜26msの2点報告が多数あります。
本数1なのにFinger 1=65535/65535、Finger 2に座標が残る例も多数あります。
公式datasheet 7.2.6は追跡中の指が同じメモリ位置に残ると規定しています。
現driverの `index < fingers` によるslot有効判定と、legacy代表座標をslot 0へ固定する前提を見直します。
修正はまだ未実施です。次はtracking IDを保った疎なslotの正規化と代表交代時の基準更新、
別途、確定済み所有権を破らない短い余分な接触の除外・未確定経路の復帰を設計します。
今回の再現はBLE切断より入力解釈経路に原因があります。過去の長時間放置時のRDY停止とは、まだ同一原因と確定しません。

実装した検証候補：driver `fix/sparse-contact-slots`、features `fix/transient-contact-count`
（既存/tmp worktree内）。driverは全物理slotをdecodeし、有効座標・confidenceのある接触を配信、
legacy代表指は有効な以前のslotを維持します。featuresのsession managerで完全frameを保持し、
継続中1本指への2本目追加だけ35ms確認します（DT `extra-contact-confirm-ms`、0で無効）。
初回多指・解除は即時です。全認識器が同じslot/countを見て、arbiter所有権は変更しません。
代表指交代は普通のポインタ・drag差分基準だけを更新し、session resetはしません。
実decoder/session試験と既存復旧・慣性・構造・依存関係の回帰試験、組合せUF2ビルドは通過、実機待ちです。
manifest/remote branchは合格待ちで変更せず、ビルドはローカルZMK_EXTRA_MODULESを指定しています。
UF2は `C:\Users\takec\Downloads\travelers-pad-contact-stability-fix-20261007.uf2`。
[接触安定化](docs/contact-slot-stability.md)参照。まず高速1F連続移動と2F scroll/pinch・Tip-Tap・tap/dragを確認、
その後バッテリーのみで3〜5分/30分放置の調査へ戻ります。放置問題の修正完了は未確認です。

接触安定化の実機合格が確認されました。旧版の放置履歴と利用者の操作時刻から、
最後30秒の移動試行中もRDY/rawが増えず、BLE/workqueueは生存し、USBで復帰したことが確認できています。
次はdriver/pad `debug/sensor-active-only`、featuresは `13d16ea` のままで比較します。
driverの任意診断設定で0x11AC〜0x11B5のtimeoutだけ0にし、ATI前に読み戻します。
自動reference管理・sampling・Event Mode・latency=0・RAM履歴は維持し、実行中のpoll/resetは追加しません。
UF2は `C:\Users\takec\Downloads\travelers-pad-sensor-active-only-20261007.uf2`。
バッテリーのみで5分→15分放置後、USB/RESET前に移動・tap・scrollを確認し履歴を取得します。
電池消費が増える診断版で、恒久対策ではありません。[Active維持比較](docs/sensor-active-only-test.md)参照。

Active維持版の5分/15分放置は実機合格。historyでもUSB再接続前の電池動作で
入力/出力が再開し、errors/rescues=0を確認。次はdriver/pad `debug/sensor-idle-only`、
featuresは維持し、Idle→LP1 timeout（0x11B0）のみ0にしてATI前に2バイトreadback確認。
他のtimeoutと自動reference管理は維持。原因modeの断定はまだしない。
UF2：`C:\Users\takec\Downloads\travelers-pad-sensor-idle-only-20261007.uf2`。
[Idle維持比較](docs/sensor-idle-only-test.md)参照。実機確認待ち。

その後Idle維持版は約33分無接触後にも実機合格。USB再接続前の電池動作で出力再開、
errors=0、rescueは起動時1から増加なし。`debug/sensor-idle-only` とUF2を戻し先として保持。
現在driver/padは `debug/sensor-lp1-only`、featuresは `13d16ea` のまま。
driverに `power-mode-limit`（none既定/active/idle/lp1）を追加。旧診断設定が優先。
比較版はlp1を選び0x11B2–0x11B3だけ0にしてATI前にreadback確認。他の設定は維持。
UF2：`C:\Users\takec\Downloads\travelers-pad-sensor-lp1-only-20261007.uf2`。
[LP1維持比較](docs/sensor-lp1-only-test.md)参照。実機確認待ち。

その後LP1維持版の5分試験は合格（復帰時に少しラグ）。30分はユーザー判断で省略。
errors=0、rescueは起動時1から増加なし。現在modeや実接触からの遅延は未測定です。
現在driver/padは `debug/sensor-mode-observation`。power-diagnosticsでATI前に設定採取、
runtime通常report内のInfo FlagsをRAM保存しhistoryへ追加。runtime追加I2Cはありません。
このUF2はnoneに戻してLP2を許可。ALP有効、LP2のみAuto-Prox有効が具体的調査対象です。
原因確定ではありません。[受動mode診断](docs/sensor-mode-observation-test.md)に制限・
5分手順・UF2/戻し先を記載。実機確認待ち、main統合/pushなし。

その後LP2許可の受動診断版で放置後停止再現。最終report約71秒、約610秒でもraw不変、
BLE/queue継続、errors=0、rescue=1。Active採取値は約539秒古く現在modeは不明。
USB再接続後history内でもreport復帰未確認。現在driver/padは `debug/lp2-autoprox-off`。
0x11C0 bit5だけ無効化し、Other Settings全word=0x8088をATI前にreadback確認。
LP2移行、timeoutと他ALP設定は維持。受動historyも維持。
[Auto-Prox停止比較](docs/lp2-autoprox-off-test.md)にUF2/5分手順を記載。実機確認待ち。

訂正：旧Auto-Prox停止版はEvent Mode有効化でOther Settingsを書き戻しており、
無効化比較として結果は無効。現在 `debug/lp2-alp-off` でEvent Mode書込みをConfig2バイトに
修正し、Event Mode後のreadbackも追加。主試験はALP bit31だけ無効化しLP2と元のAuto-Proxを維持。
Auto-Prox停止修正版も別UF2で提出。[ALP比較と訂正](docs/lp2-alp-off-test.md)参照。
その後Auto-Prox停止修正版の放置試験は合格、ALP無効版は未検証。
Idle維持のtimeoutは上書きされないため、以前の合格は有効。
修正前は新しい回帰テストに失敗し、修正後は256回のキャンセル→再開に合格しています。
[慣性キャンセル後の復帰](docs/inertia-cancel-recovery.md)を参照してください。
Show Reset警告の連続出力は、ソフトウェアリセットの誤指定
（`0x0002`、正しくは `0x0200`）による待機と判明しました。
確認期限、RDY論理極性と通信終了bitの訂正、GPIOレベルの取り残し救済を追加済みです。
ホスト状態機械テストとビルドは合格、放置・復帰の実機合否はまだ未確認です。
復旧開始の最初の原因は未確定です。

BLE接続管理の作業はPadの `feat/ble-management-layer` とfeaturesの
`feat/four-finger-hold` に退避して中断中です。今回の診断版には含めません。
[放置・復帰の検証](docs/idle-resume-recovery.md)が済むまでは、mainへの統合や
接続管理の開発再開を行わないでください。

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
