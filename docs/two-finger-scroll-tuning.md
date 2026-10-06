## English

# Two-finger scroll tuning

Two-finger scrolling is tuned at the `iqs9150_pointer_listener`, after the
IQS9150 contact tracker has committed its axis. The listener uses standard ZMK
input processors that apply only to `REL_WHEEL` and `REL_HWHEEL`; pointer
movement, edge KSCAN actions, and the sensor driver are not changed.

The default is `<&zip_scroll_scaler 1 4>`, equal to the former standard
scroll sensitivity. To override it, uncomment and edit this block in
`config/iqs9150_xiao.keymap`:

```dts
&iqs9150_pointer_listener {
    /* Vertical natural-scroll reversal / 縦の自然スクロール反転 */
    input-processors = <&zip_scroll_transform INPUT_TRANSFORM_Y_INVERT>,
                       /* <1 2> twice as fast; <1 8> half speed
                        * <1 2> は2倍速、<1 8> は半速 */
                       <&zip_scroll_scaler 1 4>;
};
```

Add `INPUT_TRANSFORM_X_INVERT` to reverse horizontal scrolling too. For both
axes, combine the flags with `|`:

```dts
<&zip_scroll_transform (INPUT_TRANSFORM_X_INVERT | INPUT_TRANSFORM_Y_INVERT)>
```

## 日本語

# 2本指スクロール調整

2本指スクロールは、IQS9150の接触追跡と軸決定の後段にある
`iqs9150_pointer_listener` で調整する。標準ZMK input processorの対象は
`REL_WHEEL` と `REL_HWHEEL` だけなので、ポインタ移動、エッジKSCAN操作、
センサドライバは変更されない。

既定値は `<&zip_scroll_scaler 1 4>` で、従来の標準スクロール感度と等価である。
変更する場合は `config/iqs9150_xiao.keymap` の次のblockを有効化して編集する。

```dts
&iqs9150_pointer_listener {
    /* Vertical natural-scroll reversal / 縦の自然スクロール反転 */
    input-processors = <&zip_scroll_transform INPUT_TRANSFORM_Y_INVERT>,
                       /* <1 2> twice as fast; <1 8> half speed
                        * <1 2> は2倍速、<1 8> は半速 */
                       <&zip_scroll_scaler 1 4>;
};
```

横スクロールも反転するには `INPUT_TRANSFORM_X_INVERT` を追加する。両軸は `|` で
組み合わせる。

```dts
<&zip_scroll_transform (INPUT_TRANSFORM_X_INVERT | INPUT_TRANSFORM_Y_INVERT)>
```
