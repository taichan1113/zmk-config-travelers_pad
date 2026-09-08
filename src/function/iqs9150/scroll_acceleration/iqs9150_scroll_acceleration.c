/*
 * Travelers Pad speed-based two-finger scroll acceleration input processor.
 *
 * The pointer function owns IQS9150 contact tracking, discontinuity rejection,
 * and axis locking. This processor only changes already-canonical wheel values
 * according to their per-report magnitude, which is proportional to finger
 * speed at the fixed report cadence.
 *
 * ポインタ機能がIQS9150の接触追跡、不連続除外、軸ロックを所有する。本processorは、
 * 固定レポート周期では指の速度に比例する、標準化済みホイール値の1レポート当たりの
 * 大きさだけを調整する。
 */

#define DT_DRV_COMPAT zmk_travelers_pad_scroll_acceleration

#include <zephyr/device.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <drivers/input_processor.h>

LOG_MODULE_REGISTER(travelers_pad_scroll_acceleration, LOG_LEVEL_INF);

struct scroll_acceleration_config {
	uint16_t fast_threshold;
	uint8_t fast_multiplier;
};

enum scroll_speed_tier {
	SCROLL_SPEED_NORMAL,
	SCROLL_SPEED_FAST,
};

struct scroll_acceleration_data {
	enum scroll_speed_tier tier;
};

static int16_t absolute_int16(int16_t value)
{
	return value < 0 ? -value : value;
}

static int scroll_acceleration_handle_event(const struct device *dev, struct input_event *event,
					    uint32_t param1, uint32_t param2,
					    struct zmk_input_processor_state *state)
{
	const struct scroll_acceleration_config *cfg = dev->config;
	struct scroll_acceleration_data *data = dev->data;
	int16_t original;
	enum scroll_speed_tier tier;

	ARG_UNUSED(param1);
	ARG_UNUSED(param2);
	ARG_UNUSED(state);

	if (event->type != INPUT_EV_REL ||
	    (event->code != INPUT_REL_WHEEL && event->code != INPUT_REL_HWHEEL)) {
		return ZMK_INPUT_PROC_CONTINUE;
	}

	original = event->value;
	tier = absolute_int16(original) >= cfg->fast_threshold ? SCROLL_SPEED_FAST :
		SCROLL_SPEED_NORMAL;
	if (tier != data->tier) {
		LOG_INF("scroll acceleration tier=%s raw=%d", tier == SCROLL_SPEED_FAST ? "fast" :
			"normal", original);
		data->tier = tier;
	}
	if (absolute_int16(original) >= cfg->fast_threshold) {
		event->value = original * cfg->fast_multiplier;
		LOG_DBG("accelerated wheel %d to %d", original, event->value);
	}

	return ZMK_INPUT_PROC_CONTINUE;
}

static const struct zmk_input_processor_driver_api scroll_acceleration_driver_api = {
	.handle_event = scroll_acceleration_handle_event,
};

#define SCROLL_ACCELERATION_INST(n)                                                            \
	static struct scroll_acceleration_data scroll_acceleration_data_##n;                         \
	static const struct scroll_acceleration_config scroll_acceleration_config_##n = {            \
		.fast_threshold = DT_INST_PROP_OR(n, fast_threshold, 12),                              \
		.fast_multiplier = DT_INST_PROP_OR(n, fast_multiplier, 2),                             \
	};                                                                                           \
	DEVICE_DT_INST_DEFINE(n, NULL, NULL, &scroll_acceleration_data_##n,                         \
			      &scroll_acceleration_config_##n, POST_KERNEL,                                  \
			      CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &scroll_acceleration_driver_api);

DT_INST_FOREACH_STATUS_OKAY(SCROLL_ACCELERATION_INST)
