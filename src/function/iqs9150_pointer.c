/*
 * Travelers Pad IQS9150 pointer function.
 *
 * This module consumes normalized contact frames from the sensor driver and
 * publishes ordinary ZMK relative-pointer events. It deliberately contains
 * pointer motion and scrolling policy, leaving tap/drag and the driver reusable.
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
};

struct pointer_data {
	const struct device *dev;
	uint16_t x;
	uint16_t y;
	uint8_t fingers;
	uint8_t previous_fingers;
	uint16_t previous_x;
	uint16_t previous_y;
	bool have_position;
	bool scroll_session;
	bool reset_frame;
};

static int16_t scale_scroll_delta(int16_t delta, uint16_t divisor) {
	int16_t scaled = delta / (int16_t)MAX(divisor, 1);

	return scaled == 0 && delta != 0 ? (delta > 0 ? 1 : -1) : scaled;
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

	if (data->reset_frame) {
		data->fingers = 0;
		data->previous_fingers = 0;
		data->have_position = false;
		data->scroll_session = false;
		data->reset_frame = false;
		/* The router uses this internal marker to end provisional pointer
		 * processing before it hands a contact to multi-finger logic. This is
		 * normal routing, not necessarily a physical sensor reset.
		 * ルーターは多指ロジックへ渡す前に、この内部マーカーで仮ポインタ処理を
		 * 終了する。通常の経路切替であり、必ずしもセンサーリセットではない。 */
		LOG_DBG("input session cleared by routed frame marker");
		return;
	}

	if (data->fingers != 0 && data->previous_fingers == 0) {
	}

	if (data->fingers != 0) {
		/* IQS9150 can briefly report one contact while two fingers remain on
		 * the surface. Once a two-finger session starts, never reinterpret an
		 * intermediate frame as pointer motion; only all-fingers-up ends it. */
		data->scroll_session = data->scroll_session || data->fingers >= 2;
		if (data->have_position && data->previous_fingers != 0) {
			dx = (int16_t)((int)data->x - (int)data->previous_x);
			dy = (int16_t)((int)data->y - (int)data->previous_y);
			if (data->scroll_session) {
				report_scroll(dev, cfg, dx, dy);
			} else {
				report_motion(dev, dx, dy);
			}
		}
		data->previous_x = data->x;
		data->previous_y = data->y;
		data->have_position = true;
	} else if (data->previous_fingers != 0) {
		data->have_position = false;
		data->scroll_session = false;
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
		} else if (evt->code == INPUT_ABS_RZ && evt->value) {
			data->reset_frame = true;
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
	};                                                                                            \
	static void pointer_input_handler_##n(struct input_event *evt) {                              \
		pointer_handle_input(DEVICE_DT_INST_GET(n), evt);                                       \
	}                                                                                             \
	INPUT_CALLBACK_DEFINE(DEVICE_DT_GET(DT_INST_PHANDLE(n, input_device)),                        \
			      pointer_input_handler_##n);                                                      \
	DEVICE_DT_INST_DEFINE(n, NULL, NULL, &pointer_data_##n, &pointer_config_##n, POST_KERNEL,    \
			      CONFIG_TRAVELERS_PAD_IQS9150_FUNCTION_INIT_PRIORITY, NULL);

DT_INST_FOREACH_STATUS_OKAY(POINTER_INST)
