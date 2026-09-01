/*
 * Travelers Pad IQS9150 continuous edge-trace virtual KSCAN.
 *
 * A single contact that starts on an enabled edge acquires that edge for the
 * rest of the contact. Motion parallel to the acquired edge emits repeated
 * KSCAN press/release pairs. It deliberately does not decide whether a pair
 * means scrolling, volume, or another action: that remains a ZMK keymap
 * decision.
 *
 * 有効な辺から始まる1本指接触を、その接触が続く間だけ辺操作として取得する。
 * 取得した辺と平行な移動は、繰り返しKSCAN押下/解放へ変換する。スクロール、音量
 * などの意味付けは、このモジュールではなくZMKキーマップが決める。
 */

#define DT_DRV_COMPAT zmk_travelers_pad_iqs9150_edge_dial_kscan

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

#include <zephyr/device.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>
#include <zephyr/drivers/kscan.h>
#include <zephyr/input/input.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(travelers_pad_edge_dial, LOG_LEVEL_INF);

enum edge_dial_edge {
	EDGE_DIAL_LEFT,
	EDGE_DIAL_RIGHT,
	EDGE_DIAL_TOP,
	EDGE_DIAL_BOTTOM,
	EDGE_DIAL_NONE,
};

enum edge_dial_mode {
	EDGE_DIAL_LINEAR,
	EDGE_DIAL_CIRCLE,
};

struct edge_dial_config {
	const struct device *input_device;
	uint16_t surface_x;
	uint16_t surface_y;
	uint16_t edge_size;
	uint16_t step_move;
	uint16_t circle_entry_depth;
	uint16_t circle_entry_span;
	uint16_t circle_step_move;
	uint16_t circle_corner_inset_x;
	uint16_t circle_corner_inset_y;
	bool enabled[4];
};

struct edge_dial_data {
	kscan_callback_t callback;
	uint16_t x;
	uint16_t y;
	uint16_t last_x;
	uint16_t last_y;
	uint8_t fingers;
	enum edge_dial_edge edge;
	enum edge_dial_mode mode;
	int8_t linear_direction;
	bool active;
	int32_t accumulated_move;
};

static int edge_dial_configure(const struct device *dev, kscan_callback_t callback)
{
	if (callback == NULL) {
		return -EINVAL;
	}
	((struct edge_dial_data *)dev->data)->callback = callback;
	return 0;
}

static const struct kscan_driver_api edge_dial_api = {
	.config = edge_dial_configure,
};

static enum edge_dial_edge edge_dial_edge_for(const struct edge_dial_config *cfg, uint16_t x,
						      uint16_t y)
{
	if (x <= cfg->edge_size) {
		return EDGE_DIAL_LEFT;
	}
	if (x + cfg->edge_size >= cfg->surface_x) {
		return EDGE_DIAL_RIGHT;
	}
	if (y <= cfg->edge_size) {
		return EDGE_DIAL_TOP;
	}
	if (y + cfg->edge_size >= cfg->surface_y) {
		return EDGE_DIAL_BOTTOM;
	}
	return EDGE_DIAL_NONE;
}

static int32_t edge_dial_parallel_delta(const struct edge_dial_data *data)
{
	if (data->edge == EDGE_DIAL_LEFT || data->edge == EDGE_DIAL_RIGHT) {
		return (int32_t)data->y - data->last_y;
	}
	return (int32_t)data->x - data->last_x;
}

static int32_t edge_dial_circle_center_x(const struct edge_dial_config *cfg,
					 const struct edge_dial_data *data)
{
	switch (data->edge) {
	case EDGE_DIAL_LEFT:
		return cfg->circle_corner_inset_x;
	case EDGE_DIAL_RIGHT:
		return cfg->surface_x - cfg->circle_corner_inset_x;
	case EDGE_DIAL_TOP:
	case EDGE_DIAL_BOTTOM:
		return data->linear_direction > 0 ? cfg->surface_x - cfg->circle_corner_inset_x :
					    cfg->circle_corner_inset_x;
	default:
		return cfg->surface_x / 2;
	}
}

static int32_t edge_dial_circle_center_y(const struct edge_dial_config *cfg,
					 const struct edge_dial_data *data)
{
	switch (data->edge) {
	case EDGE_DIAL_LEFT:
	case EDGE_DIAL_RIGHT:
		return data->linear_direction > 0 ? cfg->surface_y - cfg->circle_corner_inset_y :
					    cfg->circle_corner_inset_y;
	case EDGE_DIAL_TOP:
		return cfg->circle_corner_inset_y;
	case EDGE_DIAL_BOTTOM:
		return cfg->surface_y - cfg->circle_corner_inset_y;
	default:
		return cfg->surface_y / 2;
	}
}

static int32_t edge_dial_absolute(int32_t value) { return value < 0 ? -value : value; }

static void edge_dial_emit_step(const struct device *dev, bool positive)
{
	struct edge_dial_data *data = dev->data;
	uint8_t column = ((uint8_t)data->edge * 2) + (positive ? 0 : 1);
	/* Circular continuation is an alternate trajectory for the same edge
	 * action, not a separate action. Keep the virtual KSCAN position unchanged.
	 * 円形継続は別機能ではなく、同じ辺操作の代替軌道であるため、仮想KSCAN位置を
	 * 直線時と同一に保つ。 */
	uint8_t row = 0;

	if (data->callback == NULL) {
		LOG_WRN("edge dial ignored: KSCAN callback is not configured");
		return;
	}
	data->callback(dev, row, column, true);
	data->callback(dev, row, column, false);
	LOG_INF("edge dial mode=%s edge=%u row=%u column=%u",
		data->mode == EDGE_DIAL_CIRCLE ? "circle" : "linear", data->edge, row, column);
}

static bool edge_dial_has_entered_circle(const struct edge_dial_config *cfg,
					  const struct edge_dial_data *data)
{
	int32_t parallel_distance;

	if (data->linear_direction == 0) {
		return false;
	}

	switch (data->edge) {
	case EDGE_DIAL_LEFT:
		parallel_distance = (int32_t)data->y - edge_dial_circle_center_y(cfg, data);
		return data->x >= cfg->edge_size + cfg->circle_entry_depth &&
		       edge_dial_absolute(parallel_distance) <= cfg->circle_entry_span;
	case EDGE_DIAL_RIGHT:
		parallel_distance = (int32_t)data->y - edge_dial_circle_center_y(cfg, data);
		return data->x + cfg->edge_size + cfg->circle_entry_depth <= cfg->surface_x &&
		       edge_dial_absolute(parallel_distance) <= cfg->circle_entry_span;
	case EDGE_DIAL_TOP:
		parallel_distance = (int32_t)data->x - edge_dial_circle_center_x(cfg, data);
		return data->y >= cfg->edge_size + cfg->circle_entry_depth &&
		       edge_dial_absolute(parallel_distance) <= cfg->circle_entry_span;
	case EDGE_DIAL_BOTTOM:
		parallel_distance = (int32_t)data->x - edge_dial_circle_center_x(cfg, data);
		return data->y + cfg->edge_size + cfg->circle_entry_depth <= cfg->surface_y &&
		       edge_dial_absolute(parallel_distance) <= cfg->circle_entry_span;
	default:
		return false;
	}
}

static int32_t edge_dial_circular_delta(const struct edge_dial_config *cfg,
					 const struct edge_dial_data *data)
{
	int32_t center_x = edge_dial_circle_center_x(cfg, data);
	int32_t center_y = edge_dial_circle_center_y(cfg, data);
	int32_t previous_x = (int32_t)data->last_x - center_x;
	int32_t previous_y = (int32_t)data->last_y - center_y;
	int32_t current_x = (int32_t)data->x - center_x;
	int32_t current_y = (int32_t)data->y - center_y;
	int32_t radius = previous_x < 0 ? -previous_x : previous_x;
	int64_t cross;

	radius += previous_y < 0 ? -previous_y : previous_y;
	if (radius == 0) {
		return 0;
	}
	/* cross/radius approximates the travelled tangential distance and avoids
	 * floating point or atan2 on the nRF52840. 正接方向の移動量を整数だけで近似する。 */
	cross = (int64_t)previous_x * current_y - (int64_t)previous_y * current_x;
	return cross / radius;
}

static void edge_dial_process_frame(const struct device *dev)
{
	const struct edge_dial_config *cfg = dev->config;
	struct edge_dial_data *data = dev->data;
	int32_t delta;
	uint16_t step_move;

	if (data->fingers != 1) {
		data->active = false;
		data->edge = EDGE_DIAL_NONE;
		data->mode = EDGE_DIAL_LINEAR;
		data->linear_direction = 0;
		return;
	}

	if (!data->active) {
		data->edge = edge_dial_edge_for(cfg, data->x, data->y);
		if (data->edge < EDGE_DIAL_NONE && cfg->enabled[data->edge]) {
			data->active = true;
			data->last_x = data->x;
			data->last_y = data->y;
			data->accumulated_move = 0;
			data->mode = EDGE_DIAL_LINEAR;
			data->linear_direction = 0;
			LOG_INF("edge dial acquired edge=%u", data->edge);
		}
		return;
	}

	if (data->mode == EDGE_DIAL_LINEAR) {
		delta = edge_dial_parallel_delta(data);
		if (delta != 0) {
			data->linear_direction = delta > 0 ? 1 : -1;
		}
		if (edge_dial_has_entered_circle(cfg, data)) {
			data->mode = EDGE_DIAL_CIRCLE;
			data->last_x = data->x;
			data->last_y = data->y;
			data->accumulated_move = 0;
			LOG_INF("edge dial circle acquired edge=%u direction=%d center=(%u,%u)",
				data->edge, data->linear_direction, edge_dial_circle_center_x(cfg, data),
				edge_dial_circle_center_y(cfg, data));
			return;
		}
	} else {
		delta = edge_dial_circular_delta(cfg, data);
	}
	data->last_x = data->x;
	data->last_y = data->y;
	data->accumulated_move += delta;
	step_move = data->mode == EDGE_DIAL_CIRCLE ? cfg->circle_step_move : cfg->step_move;

	while (data->accumulated_move >= step_move || data->accumulated_move <= -((int32_t)step_move)) {
		bool positive = data->accumulated_move > 0;

		edge_dial_emit_step(dev, positive);
		data->accumulated_move += positive ? -step_move : step_move;
	}
}

static void edge_dial_handle_input(const struct device *dev, struct input_event *evt)
{
	struct edge_dial_data *data = dev->data;

	if (evt->type == INPUT_EV_ABS) {
		if (evt->code == INPUT_ABS_X) {
			data->x = evt->value;
		} else if (evt->code == INPUT_ABS_Y) {
			data->y = evt->value;
		} else if (evt->code == INPUT_ABS_Z) {
			data->fingers = evt->value;
		}
	}
	if (evt->sync) {
		edge_dial_process_frame(dev);
	}
}

#define EDGE_DIAL_INST(n)                                                                       \
	static struct edge_dial_data edge_dial_data_##n;                                          \
	static const struct edge_dial_config edge_dial_config_##n = {                             \
		.input_device = DEVICE_DT_GET(DT_INST_PHANDLE(n, input_device)),                    \
		.surface_x = DT_INST_PROP_OR(n, surface_x, 2000),                                    \
		.surface_y = DT_INST_PROP_OR(n, surface_y, 1213),                                    \
		.edge_size = DT_INST_PROP_OR(n, edge_size, 140),                                     \
		.step_move = DT_INST_PROP_OR(n, step_move, 80),                                      \
		.circle_entry_depth = DT_INST_PROP_OR(n, circle_entry_depth, 80),                    \
		.circle_entry_span = DT_INST_PROP_OR(n, circle_entry_span, 250),                     \
		.circle_step_move = DT_INST_PROP_OR(n, circle_step_move, 80),                        \
		.circle_corner_inset_x = DT_INST_PROP_OR(n, circle_corner_inset_x, 360),            \
		.circle_corner_inset_y = DT_INST_PROP_OR(n, circle_corner_inset_y, 313),            \
		.enabled = {DT_INST_PROP_OR(n, edge_left, false),                                    \
			    DT_INST_PROP_OR(n, edge_right, false),                                       \
			    DT_INST_PROP_OR(n, edge_top, false),                                         \
			    DT_INST_PROP_OR(n, edge_bottom, false)},                                     \
	};                                                                                         \
	static void edge_dial_input_handler_##n(struct input_event *evt)                           \
	{                                                                                          \
		edge_dial_handle_input(DEVICE_DT_INST_GET(n), evt);                                  \
	}                                                                                          \
	INPUT_CALLBACK_DEFINE(DEVICE_DT_GET(DT_INST_PHANDLE(n, input_device)),                     \
			      edge_dial_input_handler_##n);                                            \
	DEVICE_DT_INST_DEFINE(n, NULL, NULL, &edge_dial_data_##n, &edge_dial_config_##n,          \
			      POST_KERNEL, CONFIG_TRAVELERS_PAD_IQS9150_FUNCTION_INIT_PRIORITY,       \
			      &edge_dial_api);

DT_INST_FOREACH_STATUS_OKAY(EDGE_DIAL_INST)
