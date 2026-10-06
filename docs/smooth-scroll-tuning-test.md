## English

# Smooth scrolling tuning

## Firmware settings

- `CONFIG_ZMK_POINTING_SMOOTH_SCROLLING=y`
- `iqs9150_pointer_listener`: `zip_scroll_scaler 1 36`
- Inertia parameters and recognition logic are unchanged.

Smooth scrolling uses HID Resolution Multipliers negotiated by the host; application support can affect the result. It does not itself implement inertia or acceleration.

## Hardware check

1. Flash `travelers-pad-smooth-scroll-1-36-20261006.uf2` from Windows Downloads. Wait for initialization before touching the pad.
2. In a browser, start two fingers away from the edges and move slowly vertically and horizontally. Check direction, speed, and small movement response.
3. Scroll faster and lift both fingers while moving. Check that inertia begins without a speed jump; touch again to stop it.
4. Check one-finger pointing, taps, drag, and right/bottom edge scrolling briefly.

Submit the serial log and note whether contact scrolling and inertia each feel too slow or too fast.

Official reference

## 日本語

# 高解像度スクロール調整

## ファーム設定

- 慣性パラメータと認識ロジックは変更しない。

高解像度スクロールはホストが設定するHID Resolution Multiplierを使う。アプリの対応状況によって体感が異なることがある。慣性・加速の実装そのものではない。

## 実機確認

1. Windows DownloadsのUF2を書き込み、初期化が終わってから触る。
2. ブラウザで辺から離れた場所に2本指を置き、縦横へゆっくり動かす。方向・速度・小さい動きへの反応を確認する。
3. 速くスクロールし、動かしたまま2本とも離す。速度の段差なく慣性に移行し、再接触で止まることを確認する。
4. 1本指ポインタ、タップ、ドラッグ、右辺・下辺スクロールも簡単に確認する。

シリアルログと、接触中・慣性中それぞれが速い／遅いかを共有する。

公式資料: https://zmk.dev/docs/config/pointing
