/*
 * Travelers Pad IQS9150 pointer function.
 *
 * This module consumes normalized contact frames from the sensor driver and
 * publishes ordinary ZMK relative-pointer events. It deliberately contains
 * all interaction policy (tap, scroll and scale), leaving the driver reusable.
 */

#define DT_DRV_COMPAT zmk_travelers_pad_iqs9150_pointer

#include <zephyr/device.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

LOG_MODULE_REGISTER(travelers_pad_pointer, LOG_LEVEL_INF);

struct pointer_config {
	uint16_t scroll_divisor;
	uint16_t tap_max_ms;
	uint16_t tap_max_move;
};

struct pointer_data {
	const struct device *dev;
	uint16_t x;
	uint16_t y;
	uint8_t fingers;
	uint8_t previous_fingers;
	uint8_t max_fingers;
	uint16_t previous_x;
	uint16_t previous_y;
	uint16_t touch_start_x;
	uint16_t touch_start_y;
	int64_t touch_started_ms;
	bool have_position;
	bool touch_moved;
};

static int16_t scale_scroll_delta(int16_t delta, uint16_t divisor) {
	int16_t scaled = delta / (int16_t)MAX(divisor, 1);

	return scaled == 0 && delta != 0 ? (delta > 0 ? 1 : -1) : scaled;
}

static int absolute_int(int value) {
	return value < 0 ? -value : value;
}

static void report_click(const struct device *dev, uint16_t button) {
	int rc = input_report_key(dev, button, 1, true, K_NO_WAIT);

	if (rc == 0) {
		rc = input_report_key(dev, button, 0, true, K_NO_WAIT);
	}
	if (rc != 0) {
		LOG_ERR("click report failed: %d", rc);
	}
}

static void report_motion(const struct device *dev, int16_t dx, int16_t dy) {
	int rc;

	if (dx != 0) {
		rc = input_report_rel(dev, INPUT_REL_X, dx, dy == 0, K_NO_WAIT);
		if (rc != 0) {
			LOG_ERR("pointer X report failed: %d", rc);
		}
	}
	if (dy != 0) {
		rc = input_report_rel(dev, INPUT_REL_Y, dy, true, K_NO_WAIT);
		if (rc != 0) {
			LOG_ERR("pointer Y report failed: %d", rc);
		}
	}
}

static void report_scroll(const struct device *dev, const struct pointer_config *cfg, int16_t dx,
			  int16_t dy) {
	int rc;

	if (dx != 0) {
		rc = input_report_rel(dev, INPUT_REL_HWHEEL, scale_scroll_delta(dx, cfg->scroll_divisor),
				      dy == 0, K_NO_WAIT);
		if (rc != 0) {
			LOG_ERR("horizontal scroll report failed: %d", rc);
		}
	}
	if (dy != 0) {
		rc = input_report_rel(dev, INPUT_REL_WHEEL,
				      -scale_scroll_delta(dy, cfg->scroll_divisor), true, K_NO_WAIT);
		if (rc != 0) {
			LOG_ERR("vertical scroll report failed: %d", rc);
		}
	}
}

static void process_frame(const struct device *dev) {
	const struct pointer_config *cfg = dev->config;
	struct pointer_data *data = dev->data;
	int16_t dx = 0;
	int16_t dy = 0;
	int64_t now = k_uptime_get();

	if (data->fingers != 0 && data->previous_fingers == 0) {
		data->touch_started_ms = now;
		data->touch_start_x = data->x;
		data->touch_start_y = data->y;
		data->max_fingers = data->fingers;
		data->touch_moved = false;
	}

	if (data->fingers != 0) {
		data->max_fingers = MAX(data->max_fingers, data->fingers);
		if (absolute_int((int)data->x - (int)data->touch_start_x) > cfg->tap_max_move ||
		    absolute_int((int)data->y - (int)data->touch_start_y) > cfg->tap_max_move) {
			data->touch_moved = true;
		}

		if (data->have_position && data->previous_fingers != 0) {
			dx = (int16_t)((int)data->x - (int)data->previous_x);
			dy = (int16_t)((int)data->y - (int)data->previous_y);
			if (data->fingers >= 2) {
				report_scroll(dev, cfg, dx, dy);
			} else {
				report_motion(dev, dx, dy);
			}
		}
		data->previous_x = data->x;
		data->previous_y = data->y;
		data->have_position = true;
	} else if (data->previous_fingers != 0) {
		if (!data->touch_moved && now - data->touch_started_ms <= cfg->tap_max_ms) {
			if (data->max_fingers == 1) {
				LOG_INF("one-finger tap (%lld ms)", now - data->touch_started_ms);
				report_click(dev, INPUT_BTN_0);
			} else if (data->max_fingers == 2) {
				LOG_INF("two-finger tap (%lld ms)", now - data->touch_started_ms);
				report_click(dev, INPUT_BTN_1);
			}
		}
		data->have_position = false;
		data->max_fingers = 0;
	}

	data->previous_fingers = data->fingers;
}

static void pointer_handle_input(const struct device *dev, struct input_event *evt) {
	struct pointer_data *data = dev->data;

	switch (evt->type) {
	case INPUT_EV_ABS:
		if (evt->code == INPUT_ABS_X) {
			data->x = evt->value;
		} else if (evt->code == INPUT_ABS_Y) {
			data->y = evt->value;
		} else if (evt->code == INPUT_ABS_Z) {
			data->fingers = evt->value;
		}
		break;
	default:
		break;
	}

	if (evt->sync) {
		process_frame(dev);
	}
}

#define POINTER_INST(n)                                                                          \
	static struct pointer_data pointer_data_##n = {.dev = DEVICE_DT_INST_GET(n)};                 \
	static const struct pointer_config pointer_config_##n = {                                     \
		.scroll_divisor = DT_INST_PROP_OR(n, scroll_divisor, 4),                                \
		.tap_max_ms = DT_INST_PROP_OR(n, tap_max_ms, 350),                                      \
		.tap_max_move = DT_INST_PROP_OR(n, tap_max_move, 75),                                  \
	};                                                                                            \
	static void pointer_input_handler_##n(struct input_event *evt) {                              \
		pointer_handle_input(DEVICE_DT_INST_GET(n), evt);                                       \
	}                                                                                             \
	INPUT_CALLBACK_DEFINE(DEVICE_DT_GET(DT_INST_PHANDLE(n, input_device)),                        \
			      pointer_input_handler_##n);                                                      \
	DEVICE_DT_INST_DEFINE(n, NULL, NULL, &pointer_data_##n, &pointer_config_##n, POST_KERNEL,    \
			      CONFIG_TRAVELERS_PAD_IQS9150_FUNCTION_INIT_PRIORITY, NULL);

DT_INST_FOREACH_STATUS_OKAY(POINTER_INST)
