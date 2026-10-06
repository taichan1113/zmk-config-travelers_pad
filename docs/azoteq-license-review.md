[English](#english) | [日本語](#日本語)

# Azoteq initialization-data license review

## English

Reviewed: 2026-10-06

### Decision

Original contributions in the three repositories use MIT, as selected by the
maintainer. No suitable redistribution license has been established for the
Azoteq-derived initialization H-file. Do not relabel it MIT, BSD, Apache-2.0,
CC0, or public domain. Retain its attribution and keep it outside our MIT grant.

This is a release-readiness assessment, not a legal opinion. Absence of a
license does not establish permission or determine whether each numeric
constant is copyrightable.

### Scope

`zmk-driver-iqs9150/src/driver/iqs9150/iqs9150_ev02_init.h` was imported from
the user-provided GUI H-file. Its header credits Azoteq but contains no explicit
license grant. This review does not assert byte-for-byte identity with the
currently downloaded official presets.

File SHA-256:

```text
5bd34cd2f92afda3ac5d05e15f4d2e8dc38e816038473c54ab97eca03ef46795
```

### Evidence

| Source | Finding | Consequence |
| --- | --- | --- |
| [Website Terms of Use](https://www.azoteq.com/images/stories/pdf/Azoteq-Terms-of-Use.pdf), SOFTWARE DOWNLOADS | Software downloads are subject to their associated EULA. | Website availability is not an open-source grant. |
| [Official software listing](https://www.azoteq.com/design/software-and-tools/) | IQS9150 example code v1.2.0 and GUI v1.2.2 are listed. | Identifies the inspected distributions. |
| [Official Arduino ZIP](https://www.azoteq.com/images/stories/software/iqs9150_9151_example_code.zip) | No LICENSE/EULA file or redistribution grant was found in the README or source headers. Settings headers credit Azoteq. | No MIT/BSD/Apache basis for the H-file was established. |
| `iqs9150_9151_arduino_guide.pdf` inside that ZIP | The guide reserves rights and requires prior written permission for reproduction or redistribution of its content. | This finding applies to the guide; it is not automatically the source-code or GUI-output license. |
| [Official GUI ZIP](https://www.azoteq.com/wp-content/uploads/2026/03/IQS9150_IQS9151-GUI-v1.2.2.zip) | The installer archive contained no identifiable standalone license/EULA document. The inspected preset H-file credits Azoteq without a license. The installer was not executed. | Installer UI terms and the terms for the version used to produce the supplied file remain unverified. |
| [IQS9150 product page](https://www.azoteq.com/product/iqs9150/) | Describes GUI header export for device settings. | Intended initialization use alone does not prove public redistribution permission. |

Downloaded archive SHA-256:

```text
Arduino: 548dfef43f680e33960bc0eb97ab8cfd6808095a6dd95f174237972af0c57139
GUI:     b768f451f4807d91b506dbabe7487d5d95b6065f8c35bec0e212dac8b095d162
```

### Before public release

1. Ask Azoteq for the applicable terms and written permission to redistribute
   the GUI-generated H-file/presets, modified values, and compiled firmware
   containing these data. Include the original GUI version if available.
2. Prefer an explicit MIT/BSD-3-Clause grant for these settings, or a documented
   Azoteq-specific license permitting the intended source and binary distribution.
   Do not select one on Azoteq's behalf.
3. If permission cannot be obtained, evaluate user-supplied settings without a
   bundled vendor file, or a separately reviewed independently authored
   configuration. This needs implementation and hardware validation; renaming
   the current file is insufficient. Removing it from the working tree does not
   remove copies from Git history.

Private hosting and attribution do not substitute for permission. No repository
was made public, vendor material was not removed, and no inquiry was sent during
this review.

### Inquiry draft (not sent)

We maintain an MIT-licensed ZMK driver for IQS9150 EV02. It includes initialization
settings exported by the IQS9150 GUI with an “Author: Azoteq” header. Could you
confirm the license applicable to the GUI-generated settings and official preset
H-files? May we redistribute them publicly on GitHub, modify the settings, and
distribute compiled firmware containing them, including for commercial use?
Would you permit MIT or BSD-3-Clause licensing of these settings? If not, please
provide the applicable source/binary redistribution terms and required notices.

## 日本語

### 調査日

2026-10-06

### 判断

3リポジトリの独自成果には、メンテナーの指定によりMITを適用します。
Azoteq由来の初期化H-fileに適用できる再配布ライセンスは確認できていません。
独自判断でMIT・BSD・Apache-2.0・CC0・パブリックドメインと表示せず、
原作者表記を維持し、本プロジェクトのMIT許諾対象から除外します。

これは公開準備のための調査であり、法的意見ではありません。
ライセンスの記載がないことだけでは、再配布許諾の有無や個々の数値定数の著作権性は決まりません。

### 対象

`zmk-driver-iqs9150/src/driver/iqs9150/iqs9150_ev02_init.h` は、ユーザー提供のGUI H-fileから取り込まれました。
ヘッダにはAzoteqの原作者表記がありますが、明示的なライセンス許諾はありません。
現在ダウンロード可能な公式プリセットとバイト単位で一致するとは主張していません。

対象ファイルのSHA-256:

```text
5bd34cd2f92afda3ac5d05e15f4d2e8dc38e816038473c54ab97eca03ef46795
```

### 根拠

| 資料 | 確認結果 | 解釈 |
| --- | --- | --- |
| [公式サイト利用規約](https://www.azoteq.com/images/stories/pdf/Azoteq-Terms-of-Use.pdf)の「SOFTWARE DOWNLOADS」 | ソフトウェアのダウンロードは関連EULAに従う。 | Webサイトから取得できることはOSS許諾を意味しない。 |
| [公式ソフトウェア一覧](https://www.azoteq.com/design/software-and-tools/) | IQS9150サンプルv1.2.0とGUI v1.2.2を掲載。 | 調査した配布物の識別根拠。 |
| [公式Arduino ZIP](https://www.azoteq.com/images/stories/software/iqs9150_9151_example_code.zip) | README・ソースヘッダ内にLICENSE/EULAファイルや再配布許諾は見つからず、設定ヘッダにはAzoteq表記。 | H-fileに適用できるMIT/BSD/Apacheの根拠は確認できない。 |
| 同ZIP内の `iqs9150_9151_arduino_guide.pdf` | ガイド自身の内容は権利留保され、複製・再配布に事前の書面許可を求める記述がある。 | これはガイドについての確認であり、ソースコードやGUI出力へ自動的に適用されるとは限らない。 |
| [公式GUI ZIP](https://www.azoteq.com/wp-content/uploads/2026/03/IQS9150_IQS9151-GUI-v1.2.2.zip) | installer内に独立したライセンス/EULA文書は確認できず、調べたプリセットH-fileにはAzoteq表記のみ。installer自体は実行していない。 | installer画面の条件とユーザー提供ファイルを出力したGUI版の条件は未確認。 |
| [IQS9150製品ページ](https://www.azoteq.com/product/iqs9150/) | デバイス設定用のGUI header exportを案内。 | 初期化用途が案内されているだけでは、公開再配布許諾の根拠にならない。 |

ダウンロードした配布物のSHA-256:

```text
Arduino: 548dfef43f680e33960bc0eb97ab8cfd6808095a6dd95f174237972af0c57139
GUI:     b768f451f4807d91b506dbabe7487d5d95b6065f8c35bec0e212dac8b095d162
```

### 公開前に必要なこと

1. Azoteqへ適用条件を確認し、GUI生成H-file・プリセット・変更後データ、および
   それらを含むファームウェアの再配布許諾を書面で得てください。元のGUI版が分かれば併記します。
2. 設定に対するMIT/BSD-3-Clauseの明示許諾、または目的のソース・バイナリ配布を認める
   Azoteq固有ライセンスが望まれます。Azoteqに代わってこちらで条件を決めてはいけません。
3. 許諾を得られない場合は、ベンダーファイルを同梱せずユーザー設定を要求する方法か、
   独自に作成し別途レビューした設定を検討してください。実装と実機検証が必要です。
   現ファイルの名前を変えるだけでは解決しません。作業treeから削除してもGit履歴にはコピーが残ります。

privateで保管することや原作者を表示することは、再配布許諾の代わりにはなりません。
今回の調査では、リポジトリ公開、ベンダー資料の削除、問い合わせ送信は行っていません。

### 問い合わせ文案（未送信）

IQS9150 EV02用のMITライセンスZMK driverを開発しています。IQS9150 GUIから出力した、
「Author: Azoteq」と記載された初期化設定を使用しています。
GUI生成設定と公式プリセットH-fileに適用されるライセンスを教えてください。
GitHubでの公開再配布、設定の変更、設定を含むファームウェアの配布（商用を含む）は可能でしょうか。
設定についてMITまたはBSD-3-Clauseでの配布許諾は可能ですか。
難しい場合は、適用されるソース・バイナリ配布条件と必要な表示をご教示ください。
