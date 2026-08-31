/*
 * Travelers Pad IQS9150 tap and double-tap drag function.
 *
 * 1/2/3-finger taps produce left/right/middle clicks. The first tap is held
 * briefly so that a nearby second tap with the same finger count can become
 * a hold-to-drag operation. This listens to raw frames so router suppression
 * for scrolling, geometry, and multi-finger swipes cannot lose tap history.
 *
 * 1/2/3本指タップを左/右/中クリックへ変換する。最初のタップを短時間保留し、
 * 近い位置・同本数の二回目タップをホールドするとドラッグへ移行する。生フレームを
 * 監視するため、スクロール等のルーター排他によってタップ履歴を失わない。
 */

#define DT_DRV_COMPAT zmk_travelers_pad_iqs9150_tap_drag

#include <zephyr/device.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include <travelers_pad/multitouch.h>
#include <travelers_pad/tap_drag.h>

LOG_MODULE_REGISTER(travelers_pad_tap_drag, LOG_LEVEL_INF);

static bool drag_active;
static bool drag_tracking;

bool travelers_pad_tap_drag_is_active(void)
{
	return drag_active;
}

bool travelers_pad_tap_drag_is_tracking(void) { return drag_tracking; }

struct tap_drag_config {
	uint16_t tap_max_ms;
	uint16_t tap_max_move;
	uint16_t double_tap_max_gap_ms;
	uint16_t double_tap_max_distance;
	uint16_t drag_start_move;
};

struct tap_drag_data {
	const struct device *dev;
	struct k_work_delayable deferred_click_work;
	uint16_t x;
	uint16_t y;
	uint8_t slot;
	uint16_t slot_x[TRAVELERS_PAD_MAX_CONTACTS];
	uint16_t slot_y[TRAVELERS_PAD_MAX_CONTACTS];
	bool slot_valid[TRAVELERS_PAD_MAX_CONTACTS];
	uint8_t fingers;
	uint8_t previous_fingers;
	uint8_t max_fingers;
	uint16_t start_x;
	uint16_t start_y;
	int64_t started_ms;
	int64_t first_released_ms;
	uint16_t first_x;
	uint16_t first_y;
	uint8_t first_fingers;
	uint8_t drag_fingers;
	uint16_t drag_previous_x;
	uint16_t drag_previous_y;
	bool drag_position_valid;
	bool moved;
	bool first_valid;
	bool second_tap;
	bool button_down;
	bool sensor_reset;
};

static int absolute_int(int value)
{
	return value < 0 ? -value : value;
}

static void update_contact_centroid(struct tap_drag_data *data)
{
	uint32_t sum_x = 0;
	uint32_t sum_y = 0;
	uint8_t count = 0;

	for (uint8_t index = 0; index < TRAVELERS_PAD_MAX_CONTACTS; index++) {
		if (!data->slot_valid[index]) {
			continue;
		}
		sum_x += data->slot_x[index];
		sum_y += data->slot_y[index];
		count++;
	}
	if (count != 0) {
		data->x = sum_x / count;
		data->y = sum_y / count;
	}
}

static uint16_t button_for_fingers(uint8_t fingers)
{
	return fingers == 1 ? INPUT_BTN_0 : fingers == 2 ? INPUT_BTN_1 : INPUT_BTN_2;
}

static void report_click(const struct device *dev, uint8_t fingers)
{
	uint16_t button = button_for_fingers(fingers);
	int rc = input_report_key(dev, button, 1, true, K_NO_WAIT);

	if (rc == 0) {
		rc = input_report_key(dev, button, 0, true, K_NO_WAIT);
	}
	if (rc != 0) {
		LOG_ERR("tap click report failed: %d", rc);
	}
}

static void report_button(const struct device *dev, uint8_t fingers, bool pressed)
{
	int rc = input_report_key(dev, button_for_fingers(fingers), pressed, true, K_NO_WAIT);

	if (rc != 0) {
		LOG_ERR("drag button report failed: %d", rc);
	}
}

static void report_drag_motion(const struct device *dev, int16_t dx, int16_t dy)
{
	int rc;

	if (dx != 0) {
		rc = input_report_rel(dev, INPUT_REL_X, dx, dy == 0, K_NO_WAIT);
		if (rc != 0) {
			LOG_ERR("drag X report failed: %d", rc);
		}
	}
	if (dy != 0) {
		rc = input_report_rel(dev, INPUT_REL_Y, dy, true, K_NO_WAIT);
		if (rc != 0) {
			LOG_ERR("drag Y report failed: %d", rc);
		}
	}
}

static void start_drag(struct tap_drag_data *data, bool include_current_move);

static void deferred_click_work(struct k_work *work)
{
	struct tap_drag_data *data = CONTAINER_OF(work, struct tap_drag_data,
						     deferred_click_work.work);

	if (data->first_valid) {
		LOG_INF("%uF single tap confirmed", data->first_fingers);
		report_click(data->dev, data->first_fingers);
		data->first_valid = false;
	}
}

static void start_drag(struct tap_drag_data *data, bool include_current_move)
{
	drag_active = true;
	data->drag_previous_x = include_current_move ? data->start_x : data->x;
	data->drag_previous_y = include_current_move ? data->start_y : data->y;
	data->drag_position_valid = true;
	report_button(data->dev, data->drag_fingers, true);
	data->button_down = true;
	LOG_INF("%uF double-tap drag started", data->drag_fingers);
}

static bool matches_first_tap(const struct tap_drag_data *data, const struct tap_drag_config *cfg,
			      int64_t now)
{
	return data->first_valid && data->first_fingers == data->fingers &&
	       now - data->first_released_ms <= cfg->double_tap_max_gap_ms &&
	       absolute_int((int)data->x - data->first_x) <= cfg->double_tap_max_distance &&
	       absolute_int((int)data->y - data->first_y) <= cfg->double_tap_max_distance;
}

static void start_double_tap_candidate(struct tap_drag_data *data, const struct tap_drag_config *cfg)
{
	data->second_tap = true;
	k_work_cancel_delayable(&data->deferred_click_work);
	data->first_valid = false;
	data->drag_fingers = data->fingers;
	drag_tracking = true;
	LOG_INF("%uF double-tap candidate; waiting for movement or release", data->drag_fingers);
}

static void begin_contact(struct tap_drag_data *data, const struct tap_drag_config *cfg, int64_t now)
{
	data->started_ms = now;
	data->start_x = data->x;
	data->start_y = data->y;
	data->max_fingers = data->fingers;
	data->moved = false;
	data->second_tap = false;
	if (data->first_valid) {
		LOG_INF("%uF tap start centroid=(%u,%u), previous=%uF centroid=(%u,%u) gap=%lld ms",
			data->fingers, data->x, data->y, data->first_fingers, data->first_x,
			data->first_y, now - data->first_released_ms);
	}
	if (matches_first_tap(data, cfg, now)) {
		start_double_tap_candidate(data, cfg);
	}
}

static void finish_contact(const struct device *dev, const struct tap_drag_config *cfg, int64_t now)
{
	struct tap_drag_data *data = dev->data;
	bool valid_tap = !data->moved && data->max_fingers >= 1 && data->max_fingers <= 3 &&
			 now - data->started_ms <= cfg->tap_max_ms;

	if (data->button_down) {
		report_button(dev, data->drag_fingers, false);
		drag_active = false;
		drag_tracking = false;
		LOG_INF("%uF drag dropped", data->drag_fingers);
		data->button_down = false;
		data->drag_position_valid = false;
	} else if (data->second_tap) {
		if (!data->moved) {
			LOG_INF("%uF double tap released without drag", data->max_fingers);
			report_click(dev, data->max_fingers);
			report_click(dev, data->max_fingers);
		}
		drag_tracking = false;
	} else if (valid_tap) {
		LOG_INF("%uF tap released centroid=(%u,%u), duration=%lld ms", data->max_fingers,
			data->start_x, data->start_y, now - data->started_ms);
		data->first_valid = true;
		data->first_released_ms = now;
		data->first_x = data->start_x;
		data->first_y = data->start_y;
		data->first_fingers = data->max_fingers;
		k_work_schedule(&data->deferred_click_work, K_MSEC(cfg->double_tap_max_gap_ms));
	} else {
		data->first_valid = false;
		k_work_cancel_delayable(&data->deferred_click_work);
	}
	data->second_tap = false;
}

static void process_frame(const struct device *dev)
{
	const struct tap_drag_config *cfg = dev->config;
	struct tap_drag_data *data = dev->data;
	int64_t now = k_uptime_get();

	if (data->fingers != 0) {
		/* F-slot assignment can change between taps; centroid is invariant. */
		update_contact_centroid(data);
	}

	if (data->sensor_reset) {
		k_work_cancel_delayable(&data->deferred_click_work);
		if (data->button_down) {
			report_button(dev, data->drag_fingers, false);
		}
		drag_active = false;
		drag_tracking = false;
		*data = (struct tap_drag_data){.dev = dev};
		k_work_init_delayable(&data->deferred_click_work, deferred_click_work);
		LOG_WRN("tap/drag session cancelled after sensor reset");
		return;
	}
	if (data->fingers != 0 && data->previous_fingers == 0) {
		begin_contact(data, cfg, now);
	}
	if (data->fingers != 0) {
		bool fingers_increased = data->fingers > data->max_fingers;

		data->max_fingers = MAX(data->max_fingers, data->fingers);
		/* A nominally simultaneous multi-finger tap can arrive as 1F then 2F
		 * (or 3F) on adjacent reports. While it is still stationary, rebase the
		 * tap origin to the completed contact centroid; otherwise the change from
		 * F1 to the centroid looks like a large, false movement. */
		if (fingers_increased && !data->moved && !data->button_down) {
			data->start_x = data->x;
			data->start_y = data->y;
		}
		/* IQS9150 may reveal contacts across adjacent reports. Recheck while the
		 * second touch is still stationary, rather than relying only on its first
		 * report. This is essential for a simultaneous 2F/3F double tap. */
		if (!data->second_tap && !data->button_down && data->max_fingers == data->fingers &&
		    matches_first_tap(data, cfg, now)) {
			start_double_tap_candidate(data, cfg);
		}
		if (absolute_int((int)data->x - data->start_x) > cfg->tap_max_move ||
		    absolute_int((int)data->y - data->start_y) > cfg->tap_max_move) {
			data->moved = true;
		}
		if (data->second_tap && !data->button_down &&
		    (absolute_int((int)data->x - data->start_x) >= cfg->drag_start_move ||
		     absolute_int((int)data->y - data->start_y) >= cfg->drag_start_move)) {
			start_drag(data, true);
		}
		if (data->button_down && data->fingers == data->drag_fingers) {
			if (data->drag_position_valid) {
				report_drag_motion(dev, (int16_t)((int)data->x - data->drag_previous_x),
						    (int16_t)((int)data->y - data->drag_previous_y));
			}
			data->drag_previous_x = data->x;
			data->drag_previous_y = data->y;
			data->drag_position_valid = true;
		} else if (data->button_down) {
			/* Do not convert a partial 2F/3F release into a centroid jump. The
			 * next complete frame is used only to rebase drag motion. */
			data->drag_position_valid = false;
		}
	} else if (data->previous_fingers != 0) {
		finish_contact(dev, cfg, now);
	}
	data->previous_fingers = data->fingers;
}

static void tap_drag_handle_input(const struct device *dev, struct input_event *evt)
{
	struct tap_drag_data *data = dev->data;

	if (evt->type == INPUT_EV_ABS) {
		if (evt->code == INPUT_ABS_MT_SLOT) {
			data->slot = evt->value;
		} else if (evt->code == TRAVELERS_PAD_INPUT_ABS_MT_TRACKING_ID &&
			   data->slot < TRAVELERS_PAD_MAX_CONTACTS) {
			data->slot_valid[data->slot] = evt->value >= 0;
		} else if (evt->code == TRAVELERS_PAD_INPUT_ABS_MT_POSITION_X &&
			   data->slot < TRAVELERS_PAD_MAX_CONTACTS) {
			data->slot_x[data->slot] = evt->value;
		} else if (evt->code == TRAVELERS_PAD_INPUT_ABS_MT_POSITION_Y &&
			   data->slot < TRAVELERS_PAD_MAX_CONTACTS) {
			data->slot_y[data->slot] = evt->value;
		} else if (evt->code == INPUT_ABS_X) {
			data->x = evt->value;
		} else if (evt->code == INPUT_ABS_Y) {
			data->y = evt->value;
		} else if (evt->code == INPUT_ABS_Z) {
			data->fingers = evt->value;
		} else if (evt->code == INPUT_ABS_RZ && evt->value) {
			data->sensor_reset = true;
		}
	}
	if (evt->sync) {
		process_frame(dev);
	}
}

static int tap_drag_init(const struct device *dev)
{
	struct tap_drag_data *data = dev->data;

	k_work_init_delayable(&data->deferred_click_work, deferred_click_work);
	return 0;
}

#define TAP_DRAG_INST(n)                                                                      \
	static struct tap_drag_data tap_drag_data_##n = {.dev = DEVICE_DT_INST_GET(n)};            \
	static const struct tap_drag_config tap_drag_config_##n = {                                \
		.tap_max_ms = DT_INST_PROP_OR(n, tap_max_ms, 180),                                    \
		.tap_max_move = DT_INST_PROP_OR(n, tap_max_move, 75),                                \
		.double_tap_max_gap_ms = DT_INST_PROP_OR(n, double_tap_max_gap_ms, 250),             \
		.double_tap_max_distance = DT_INST_PROP_OR(n, double_tap_max_distance, 120),          \
		.drag_start_move = DT_INST_PROP_OR(n, drag_start_move, 20),                          \
	};                                                                                           \
	static void tap_drag_input_handler_##n(struct input_event *evt)                            \
	{                                                                                            \
		tap_drag_handle_input(DEVICE_DT_INST_GET(n), evt);                                   \
	}                                                                                            \
	INPUT_CALLBACK_DEFINE(DEVICE_DT_GET(DT_INST_PHANDLE(n, input_device)),                     \
			      tap_drag_input_handler_##n);                                                 \
	DEVICE_DT_INST_DEFINE(n, tap_drag_init, NULL, &tap_drag_data_##n, &tap_drag_config_##n, POST_KERNEL, \
			      CONFIG_TRAVELERS_PAD_IQS9150_FUNCTION_INIT_PRIORITY, NULL);

DT_INST_FOREACH_STATUS_OKAY(TAP_DRAG_INST)
