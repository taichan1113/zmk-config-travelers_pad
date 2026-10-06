[English](#english) | [日本語](#日本語)

# Project handoff

## English

Last updated: 2026-10-07

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
