/* Host regression / host回帰試験。 */
#include <assert.h>
#include <stdio.h>
#include <travelers_pad/inertia_math.h>

static int magnitude(int value) { return value < 0 ? -value : value; }

int main(void)
{
	const int one = TRAVELERS_PAD_INERTIA_ONE;
	/* Equal physical speed at different sensor report periods.
	 * sensor report周期によらず同じ物理速度を推定する。 */
	assert(travelers_pad_inertia_velocity(0, 10, 20, 80, 1) ==
	       travelers_pad_inertia_velocity(0, 25, 50, 80, 1));
	int half = travelers_pad_inertia_decay(one, 250, 250);
	assert(magnitude(half - one / 2) < 100);
	assert(travelers_pad_inertia_decay(-one, 250, 250) == -half);
	int a = one;
	int b = one;
	for (int i = 0; i < 25; i++) a = travelers_pad_inertia_decay(a, 10, 250);
	for (int i = 0; i < 5; i++) b = travelers_pad_inertia_decay(b, 50, 250);
	assert(magnitude(a - b) < 100);
	/* Fractional tail eventually produces a count, symmetrically.
	 * 小さな移動端数も最終的にcountへ変換し、正負対称にする。 */
	int64_t remainder = 0;
	int total = 0;
	for (int i = 0; i < 100; i++) total += travelers_pad_inertia_integrate(one / 100, 10, &remainder);
	assert(total == 9);
	remainder = 0;
	int negative = 0;
	for (int i = 0; i < 100; i++) negative += travelers_pad_inertia_integrate(-one / 100, 10, &remainder);
	assert(negative == -total);
	/* Direction reversal replaces, not mixes with, old velocity.
	 * 逆方向は古い速度と混ぜず置換する。 */
	assert(travelers_pad_inertia_velocity(one, -10, 20, 80, 0) == -one / 2);
	assert(travelers_pad_inertia_decay(one, 100000, 250) == 0);
	assert(travelers_pad_inertia_decay(one, 1, 0) == 0);
	assert(travelers_pad_inertia_velocity(0, 1, 0, 80, 1) == one);
	puts("PASS: inertia velocity, half-life, report-period independence, fractional travel, reversal");
	return 0;
}
