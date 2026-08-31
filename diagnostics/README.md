# XIAO USB起動診断 / XIAO USB boot diagnostic

`xiao_boot_probe.conf` は、UF2コピー後にXIAOがアプリを実行しているか、またはUSBだけが
Windowsへ列挙されていないかを区別するための一時的な診断機能です。通常の利用時には有効化
しません。

`xiao_boot_probe.conf` is a temporary diagnostic feature that distinguishes an
application that is running but not enumerating USB from one that never left
the UF2 bootloader. Do not enable it for normal use.

診断UF2をコピーした直後、USBを抜かずにXIAOの赤LEDを確認します。

- 赤LEDが点滅: ZMKアプリは実行済みで、USBが `CONFIGURED` へ至っていません。
- 赤LEDが消灯: ZMKアプリとUSB列挙は成功しています。
- 赤LEDが一切変化しない: アプリはこの診断コードまで到達していません。UF2ブートローダー遷移、
  電源、または書込みを調査します。

Immediately after copying the diagnostic UF2, keep USB connected and observe
the XIAO red LED.

- Flashing red: ZMK is running, but USB has not reached `CONFIGURED`.
- Red LED off: ZMK and USB enumeration succeeded.
- No red LED activity: the application did not reach this diagnostic code;
  investigate the UF2 bootloader transition, power, or flashing.

LED状態を観察してから、必要なら一度だけUSBを抜き差ししてCDCシリアルログを送ってください。
ログに `application reached XIAO USB boot probe` があれば、アプリ到達は確認できます。

After observing the LED state, if necessary unplug and reconnect USB once and
send the CDC log. `application reached XIAO USB boot probe` confirms that the
application reached the diagnostic code.
