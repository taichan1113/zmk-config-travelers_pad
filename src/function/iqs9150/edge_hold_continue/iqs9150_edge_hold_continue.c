/*
 * Travelers Pad IQS9150 edge-hold continuation detector.
 *
 * Confirmed drag can continue beyond the physical pad edge with timer-driven
 * REL_X/Y reports. Two-finger scroll remains detection-only in this stage.
 *
 * 確定済みドラッグはタイマー駆動のREL_X/Yレポートでパッド端の外まで継続する。
 * 2本指スクロールは、この段階では検出専用のままとする。
 */

#define DT_DRV_COMPAT zmk_travelers_pad_iqs9150_edge_hold_continue

#include <zephyr/device.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <travelers_pad/multitouch.h>
#include <travelers_pad/tap_drag.h>

LOG_MODULE_REGISTER(travelers_pad_edge_hold, LOG_LEVEL_INF);

enum edge_hold_edge {
	EDGE_HOLD_NONE,
	EDGE_HOLD_LEFT,
	EDGE_HOLD_RIGHT,
	EDGE_HOLD_TOP,
	EDGE_HOLD_BOTTOM,
};

struct edge_hold_config {
	uint16_t surface_x;
	uint16_t surface_y;
	uint16_t edge_size;
	uint16_t drag_start_delay_ms;
	uint16_t drag_repeat_ms;
	int16_t drag_repeat_delta;
};

struct edge_hold_data {
	const struct device *dev;
	uint8_t slot;
	uint16_t slot_x[TRAVELERS_PAD_MAX_CONTACTS];
	uint16_t slot_y[TRAVELERS_PAD_MAX_CONTACTS];
	bool slot_valid[TRAVELERS_PAD_MAX_CONTACTS];
	uint8_t drag_fingers;
	uint16_t scroll_x;
	uint16_t scroll_y;
	uint8_t scroll_fingers;
	enum edge_hold_edge drag_edge;
	enum edge_hold_edge scroll_edge;
	struct k_work_delayable drag_repeat_work;
};

static const char *edge_name(enum edge_hold_edge edge)
{
	switch (edge) {
	case EDGE_HOLD_LEFT:
		return "left";
	case EDGE_HOLD_RIGHT:
		return "right";
	case EDGE_HOLD_TOP:
		return "top";
	case EDGE_HOLD_BOTTOM:
		return "bottom";
	default:
		return "none";
	}
}

static enum edge_hold_edge edge_at(const struct edge_hold_config *cfg, uint16_t x, uint16_t y)
{
	/* Pick the nearest edge in a corner so a single contact has one stable
	 * continuation direction. The final output stage will latch that direction
	 * until release; this detector verifies the same choice now.
	 * 角では最も近い辺を選び、1接触に対する継続方向を一意にする。最終出力段階は
	 * 解放まで方向をラッチするため、検出段階でも同じ選択を検証する。 */
	uint16_t left = x;
	uint16_t right = x >= cfg->surface_x ? 0 : cfg->surface_x - x;
	uint16_t top = y;
	uint16_t bottom = y >= cfg->surface_y ? 0 : cfg->surface_y - y;
	uint16_t minimum = left;
	enum edge_hold_edge edge = EDGE_HOLD_LEFT;

	if (right < minimum) {
		minimum = right;
		edge = EDGE_HOLD_RIGHT;
	}
	if (top < minimum) {
		minimum = top;
		edge = EDGE_HOLD_TOP;
	}
	if (bottom < minimum) {
		minimum = bottom;
		edge = EDGE_HOLD_BOTTOM;
	}
	return minimum <= cfg->edge_size ? edge : EDGE_HOLD_NONE;
}

static bool drag_centroid(const struct edge_hold_data *data, uint16_t *x, uint16_t *y)
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
	if (count == 0) {
		return false;
	}
	*x = sum_x / count;
	*y = sum_y / count;
	return true;
}

static void stop_drag_repeat(const struct device *dev)
{
	struct edge_hold_data *data = dev->data;

	k_work_cancel_delayable(&data->drag_repeat_work);
}

static void drag_repeat_work_handler(struct k_work *work)
{
	struct edge_hold_data *data = CONTAINER_OF(work, struct edge_hold_data,
						     drag_repeat_work.work);
	const struct edge_hold_config *cfg = data->dev->config;
	int16_t dx = 0;
	int16_t dy = 0;
	int rc;

	/* The tap/drag module owns button state. This module only adds motion while
	 * that established drag remains valid.
	 * ボタン状態はtap/dragモジュールが所有する。本モジュールは確定ドラッグが
	 * 有効な間だけ移動を追加する。 */
	if (!travelers_pad_tap_drag_is_active() || data->drag_edge == EDGE_HOLD_NONE) {
		return;
	}

	switch (data->drag_edge) {
	case EDGE_HOLD_LEFT:
		dx = -cfg->drag_repeat_delta;
		break;
	case EDGE_HOLD_RIGHT:
		dx = cfg->drag_repeat_delta;
		break;
	case EDGE_HOLD_TOP:
		dy = -cfg->drag_repeat_delta;
		break;
	case EDGE_HOLD_BOTTOM:
		dy = cfg->drag_repeat_delta;
		break;
	default:
		return;
	}

	if (dx != 0) {
		rc = input_report_rel(data->dev, INPUT_REL_X, dx, true, K_NO_WAIT);
	} else {
		rc = input_report_rel(data->dev, INPUT_REL_Y, dy, true, K_NO_WAIT);
	}
	if (rc != 0) {
		LOG_ERR("drag continuation report failed: %d", rc);
	}
	k_work_schedule(&data->drag_repeat_work, K_MSEC(cfg->drag_repeat_ms));
}

static void start_drag_repeat(const struct device *dev)
{
	const struct edge_hold_config *cfg = dev->config;
	struct edge_hold_data *data = dev->data;

	k_work_cancel_delayable(&data->drag_repeat_work);
	k_work_schedule(&data->drag_repeat_work, K_MSEC(cfg->drag_start_delay_ms));
	LOG_INF("drag continuation started edge=%s delay=%ums period=%ums delta=%d",
		edge_name(data->drag_edge), cfg->drag_start_delay_ms, cfg->drag_repeat_ms,
		cfg->drag_repeat_delta);
}

static void detect_drag_edge(const struct device *dev)
{
	const struct edge_hold_config *cfg = dev->config;
	struct edge_hold_data *data = dev->data;
	uint16_t x;
	uint16_t y;
	enum edge_hold_edge edge = EDGE_HOLD_NONE;

	if (travelers_pad_tap_drag_is_active() && data->drag_fingers != 0 &&
	    drag_centroid(data, &x, &y)) {
		edge = edge_at(cfg, x, y);
	}
	if (edge != data->drag_edge) {
		stop_drag_repeat(dev);
		if (edge != EDGE_HOLD_NONE) {
			LOG_INF("drag continuation eligible edge=%s centroid=(%u,%u) fingers=%u",
				edge_name(edge), x, y, data->drag_fingers);
		} else if (data->drag_edge != EDGE_HOLD_NONE) {
			LOG_INF("drag continuation disarmed edge=%s", edge_name(data->drag_edge));
		}
		data->drag_edge = edge;
		if (edge != EDGE_HOLD_NONE) {
			start_drag_repeat(dev);
		}
	}
}

static void detect_scroll_edge(const struct device *dev)
{
	const struct edge_hold_config *cfg = dev->config;
	struct edge_hold_data *data = dev->data;
	enum edge_hold_edge edge = data->scroll_fingers == 2
					? edge_at(cfg, data->scroll_x, data->scroll_y)
					: EDGE_HOLD_NONE;

	if (edge != data->scroll_edge) {
		if (edge != EDGE_HOLD_NONE) {
			LOG_INF("scroll continuation eligible edge=%s representative=(%u,%u)",
				edge_name(edge), data->scroll_x, data->scroll_y);
		} else if (data->scroll_edge != EDGE_HOLD_NONE) {
			LOG_INF("scroll continuation disarmed edge=%s", edge_name(data->scroll_edge));
		}
		data->scroll_edge = edge;
	}
}

static void edge_hold_drag_input(const struct device *dev, struct input_event *evt)
{
	struct edge_hold_data *data = dev->data;

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
		} else if (evt->code == INPUT_ABS_Z) {
			data->drag_fingers = evt->value;
		}
	}
	if (evt->sync) {
		detect_drag_edge(dev);
	}
}

static void edge_hold_scroll_input(const struct device *dev, struct input_event *evt)
{
	struct edge_hold_data *data = dev->data;

	if (evt->type == INPUT_EV_ABS) {
		if (evt->code == INPUT_ABS_X) {
			data->scroll_x = evt->value;
		} else if (evt->code == INPUT_ABS_Y) {
			data->scroll_y = evt->value;
		} else if (evt->code == INPUT_ABS_Z) {
			data->scroll_fingers = evt->value;
		}
	}
	if (evt->sync) {
		detect_scroll_edge(dev);
	}
}

static int edge_hold_init(const struct device *dev)
{
	const struct edge_hold_config *cfg = dev->config;

	k_work_init_delayable(&((struct edge_hold_data *)dev->data)->drag_repeat_work,
			     drag_repeat_work_handler);
	LOG_INF("edge hold active: surface=%ux%u edge=%u; drag delay=%ums period=%ums delta=%d; "
		"scroll output disabled", cfg->surface_x, cfg->surface_y, cfg->edge_size,
		cfg->drag_start_delay_ms, cfg->drag_repeat_ms, cfg->drag_repeat_delta);
	return 0;
}

#define EDGE_HOLD_INST(n)                                                                      \
	static struct edge_hold_data edge_hold_data_##n = {.dev = DEVICE_DT_INST_GET(n)};           \
	static const struct edge_hold_config edge_hold_config_##n = {                               \
		.surface_x = DT_INST_PROP_OR(n, surface_x, 2000),                                      \
		.surface_y = DT_INST_PROP_OR(n, surface_y, 1213),                                      \
		.edge_size = DT_INST_PROP_OR(n, edge_size, 80),                                         \
		.drag_start_delay_ms = DT_INST_PROP_OR(n, drag_start_delay_ms, 120),                    \
		.drag_repeat_ms = DT_INST_PROP_OR(n, drag_repeat_ms, 24),                               \
		.drag_repeat_delta = DT_INST_PROP_OR(n, drag_repeat_delta, 8),                          \
	};                                                                                           \
	static void edge_hold_drag_input_handler_##n(struct input_event *evt)                       \
	{                                                                                            \
		edge_hold_drag_input(DEVICE_DT_INST_GET(n), evt);                                      \
	}                                                                                            \
	static void edge_hold_scroll_input_handler_##n(struct input_event *evt)                     \
	{                                                                                            \
		edge_hold_scroll_input(DEVICE_DT_INST_GET(n), evt);                                    \
	}                                                                                            \
	INPUT_CALLBACK_DEFINE(DEVICE_DT_GET(DT_INST_PHANDLE(n, drag_input_device)),                 \
			      edge_hold_drag_input_handler_##n);                                             \
	INPUT_CALLBACK_DEFINE(DEVICE_DT_GET(DT_INST_PHANDLE(n, scroll_input_device)),               \
			      edge_hold_scroll_input_handler_##n);                                           \
	DEVICE_DT_INST_DEFINE(n, edge_hold_init, NULL, &edge_hold_data_##n, &edge_hold_config_##n,  \
			      POST_KERNEL, CONFIG_TRAVELERS_PAD_IQS9150_FUNCTION_INIT_PRIORITY, NULL);

DT_INST_FOREACH_STATUS_OKAY(EDGE_HOLD_INST)
