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

struct edge_dial_config {
	const struct device *input_device;
	uint16_t surface_x;
	uint16_t surface_y;
	uint16_t edge_size;
	uint16_t step_move;
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

static void edge_dial_emit_step(const struct device *dev, bool positive)
{
	struct edge_dial_data *data = dev->data;
	uint8_t column = ((uint8_t)data->edge * 2) + (positive ? 0 : 1);

	if (data->callback == NULL) {
		LOG_WRN("edge dial ignored: KSCAN callback is not configured");
		return;
	}
	data->callback(dev, 0, column, true);
	data->callback(dev, 0, column, false);
	LOG_INF("edge dial edge=%u column=%u", data->edge, column);
}

static void edge_dial_process_frame(const struct device *dev)
{
	const struct edge_dial_config *cfg = dev->config;
	struct edge_dial_data *data = dev->data;
	int32_t delta;

	if (data->fingers != 1) {
		data->active = false;
		data->edge = EDGE_DIAL_NONE;
		return;
	}

	if (!data->active) {
		data->edge = edge_dial_edge_for(cfg, data->x, data->y);
		if (data->edge < EDGE_DIAL_NONE && cfg->enabled[data->edge]) {
			data->active = true;
			data->last_x = data->x;
			data->last_y = data->y;
			data->accumulated_move = 0;
			LOG_INF("edge dial acquired edge=%u", data->edge);
		}
		return;
	}

	delta = edge_dial_parallel_delta(data);
	data->last_x = data->x;
	data->last_y = data->y;
	data->accumulated_move += delta;

	while (data->accumulated_move >= cfg->step_move ||
	       data->accumulated_move <= -((int32_t)cfg->step_move)) {
		bool positive = data->accumulated_move > 0;

		edge_dial_emit_step(dev, positive);
		data->accumulated_move += positive ? -cfg->step_move : cfg->step_move;
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
