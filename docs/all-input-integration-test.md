# Full input integration test / 全入力結合試験

## Scope / 対象

This profile runs the normal pointer path, two-finger scrolling, one-to-three
finger tap/drag, three-to-five finger gesture KSCAN, and the enabled edge dial
together. The optional grid module remains disabled because it has no selected
user keymap yet.

このプロファイルでは、通常ポインタ、二本指スクロール、1〜3本指タップ/ドラッグ、
3〜5本指ジェスチャーKSCAN、有効なエッジダイヤルを同時に動かす。任意格子モジュールは
利用者用キーマップが未選択のため、無効のままとする。

## Essential checks / 重要確認項目

1. Move one finger, perform a two-finger vertical scroll, then make 1F/2F/3F
   stationary taps. Confirm pointer, scroll, left click, right click, and middle
   click respectively.
   1本指でポインタを動かし、二本指で縦スクロールした後、1/2/3本指の静止タップを
   行う。順にポインタ、スクロール、左/右/中クリックを確認する。
2. Perform a one-finger double-tap-hold drag, then immediately make a 3F swipe.
   Confirm the drag releases once, then the 3F gesture runs without a click.
   1本指ダブルタップ＋ホールドでドラッグし、直後に3本指スワイプを行う。ドラッグが
   一度だけ解放され、その後の3本指ジェスチャーにクリックが混入しないことを確認する。
3. Test 3F left/right/up/down, 4F left/right/up/down, and 5F Grasp/Release.
   Confirm their configured Windows actions and no pointer, scroll, or click
   leakage during each gesture.
   3本/4本指の全四方向と5本指Grasp/Releaseを試す。設定済みWindows操作と、各操作中に
   ポインタ、スクロール、クリックが混入しないことを確認する。
4. Trace each edge and its local circular continuation. Confirm the mapped
   continuous host action repeats; immediately after lift, a centre 1F tap must
   still produce one left click. Left adjusts brightness, right scrolls
   vertically, top adjusts volume, and bottom scrolls horizontally.
   4辺と局所円継続をなぞる。対応する連続ホスト操作だけが反復し、指を離した直後の中央
   1本指タップは左クリック一回となることを確認する。左辺は輝度、右辺は縦スクロール、
   上辺は音量、下辺は横スクロールを操作する。

## Expected ownership / 期待する所有権

- 1F pointer and 2F scroll: router → pointer HID.
- 1F/2F/3F stationary tap or double-tap hold: tap/drag → mouse HID.
- 3F/4F swipe and 5F Grasp/Release: gesture KSCAN → keymap.
- Enabled edge motion: edge-dial KSCAN → keymap.

- 1本指ポインタと二本指スクロール: router → pointer HID。
- 1/2/3本指の静止タップまたはダブルタップホールド: tap/drag → mouse HID。
- 3/4本指スワイプと5本指Grasp/Release: gesture KSCAN → keymap。
- 有効辺の操作: edge-dial KSCAN → keymap。
