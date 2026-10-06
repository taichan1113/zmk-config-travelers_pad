# Scroll / pinch threshold regression

## English

The conservative pinch profile raised the transform recognizer's squared-distance
start threshold to 300000, but left the router's scroll-exclusion threshold at
30000. The router can latch geometry exclusion for the rest of a contact session
before the transform recognizer accepts a pinch. Thus small changes in finger
spacing can block a new scroll stroke without producing a pinch.

The keyboard overlay now uses one shared constant for both thresholds. Inertia
timing, decay, scroll scaling, and pinch repeat thresholds are unchanged.

A host harness compiled the actual router recognition code with platform stubs.
It reproduced exclusion at the old threshold, admitted the same short stroke at
the corrected threshold (100 repetitions), and retained exclusion for a deliberate
large pinch. This does not simulate BLE, inertia scheduling, or sensor noise.

Hardware regression checks:

1. Repeat short two-finger vertical and horizontal strokes about 20 times each.
2. Start another stroke while inertia from the previous stroke is still running.
3. Check that touching stops inertia and that deliberate pinch still works.

Intermittent stops are not conclusively explained without a capture of the failing
operation. If they persist, collect the session, router, transform, and inertia
logs. A very short stroke may also fail the existing scroll movement threshold;
this change does not lower that threshold.

## 日本語

ピンチ感度を抑える設定で、transform側の二乗距離差の開始閾値を300000へ
上げた一方、router側のスクロール除外閾値は30000のままでした。
routerはピンチが認識される前に、その接触セッションの残りをgeometryとして
除外することがあります。そのため、小さな指間隔の変化でも、新しいスクロールが
抑止され、ピンチも発火しない状態に入り得ます。

キーボードoverlayで両閾値を共通定数に統一しました。慣性の時間・減衰、
スクロール倍率、ピンチの連続発火閾値は変更していません。

実際のrouter認識コードを、プラットフォーム部分をスタブ化してホスト上で
コンパイルしました。旧閾値での除外を再現し、修正後は同じ短いストロークを
認識すること（100回反復）、大きなピンチの除外を維持することを確認しました。
BLE・慣性のスケジューリング・センサーノイズはこのテストの対象外です。

実機の回帰確認：

1. 短い2本指スクロールを縦・横それぞれ約20回繰り返す。
2. 前の慣性が続いている間に、次のストロークを開始する。
3. 接触による慣性停止と、意図的なピンチの動作を確認する。

停止した操作のログがないため、今回の現象をすべて説明できたとは断定しません。
再発する場合はsession・router・transform・inertiaのログを取得してください。
極端に短いストロークは既存のスクロール移動閾値に達しない場合もあります。
今回その移動閾値は変更していません。
