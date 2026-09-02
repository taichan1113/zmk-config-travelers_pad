# Two-finger scroll direction / 2本指スクロール方向

The standard direction is the default. To reverse one axis, uncomment the
matching property in `config/iqs9150_xiao.keymap`, then build normally.

標準方向が既定値です。いずれかの軸を反転する場合は、
`config/iqs9150_xiao.keymap` 内の対応するプロパティのコメントを外してから、
通常どおりビルドします。

```dts
&iqs9150_pointer {
    scroll-invert-vertical;
    /* scroll-invert-horizontal; */
};
```

`scroll-invert-vertical` reverses up/down scrolling. `scroll-invert-horizontal`
reverses left/right scrolling. These options do not alter pointer movement,
edge actions, touch/drag, or the IQS9150 contact filtering.

`scroll-invert-vertical` は上下スクロールを、`scroll-invert-horizontal` は
左右スクロールを反転します。ポインタ移動、エッジ操作、タップ/ドラッグ、IQS9150の
接触フィルタには影響しません。
