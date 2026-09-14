/* Two-finger transform gesture primary recognizer.
 * 2本指pinch/rotation primary recognizer。 */

#define DT_DRV_COMPAT zmk_travelers_pad_iqs9150_transform_gesture

#include <zephyr/device.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>
#include <zephyr/input/input.h>
#include <zephyr/logging/log.h>

#include <travelers_pad/contact_arbiter.h>
#include <travelers_pad/multitouch.h>

LOG_MODULE_REGISTER(travelers_pad_transform_gesture, LOG_LEVEL_INF);

enum transform_mode {
	TRANSFORM_NONE,
	TRANSFORM_PINCH,
	TRANSFORM_ROTATE,
	TRANSFORM_PINCH_ROTATE,
};

struct transform_config {
	uint32_t distance_threshold;
	uint32_t rotation_threshold;
};

struct transform_data {
	const struct device *dev;
	uint16_t first_x;
	uint16_t first_y;
	uint16_t second_x;
	uint16_t second_y;
	bool second_valid;
	uint8_t fingers;
	bool baseline_valid;
	int32_t baseline_x;
	int32_t baseline_y;
	uint32_t baseline_distance_squared;
	bool claimed;
	enum transform_mode mode;
	bool sensor_reset;
	uint32_t arbiter_frame_seq;
};

static int32_t abs32(int32_t value)
{
	return value < 0 ? -value : value;
}

static uint32_t squared_distance(int32_t x, int32_t y)
{
	return (uint32_t)(x * x + y * y);
}

static const char *mode_name(enum transform_mode mode)
{
	switch (mode) {
	case TRANSFORM_PINCH:
		return "PINCH";
	case TRANSFORM_ROTATE:
		return "ROTATE";
	case TRANSFORM_PINCH_ROTATE:
		return "PINCH_ROTATE";
	case TRANSFORM_NONE:
	default:
		return "NONE";
	}
}

static void reset_transform(struct transform_data *data)
{
	data->baseline_valid = false;
	data->claimed = false;
	data->mode = TRANSFORM_NONE;
}

static void recognize_transform(const struct device *dev)
{
	const struct transform_config *cfg = dev->config;
	struct transform_data *data = dev->data;
	int32_t geometry_x;
	int32_t geometry_y;
	uint32_t distance_squared;
	uint32_t distance_delta;
	uint32_t rotation_cross;

	if (data->sensor_reset || data->fingers != 2 || !data->second_valid) {
		if (data->fingers != 2) {
			reset_transform(data);
		}
		return;
	}

	geometry_x = (int32_t)data->second_x - data->first_x;
	geometry_y = (int32_t)data->second_y - data->first_y;
	if (!data->baseline_valid) {
		data->baseline_x = geometry_x;
		data->baseline_y = geometry_y;
		data->baseline_distance_squared = squared_distance(geometry_x, geometry_y);
		data->baseline_valid = true;
		return;
	}
	if (data->claimed) {
		return;
	}

	distance_squared = squared_distance(geometry_x, geometry_y);
	distance_delta = abs32((int32_t)distance_squared - data->baseline_distance_squared);
	rotation_cross = abs32(data->baseline_x * geometry_y - data->baseline_y * geometry_x);
	if (distance_delta < cfg->distance_threshold && rotation_cross < cfg->rotation_threshold) {
		return;
	}

	data->mode = distance_delta >= cfg->distance_threshold &&
			     rotation_cross >= cfg->rotation_threshold ? TRANSFORM_PINCH_ROTATE :
		     distance_delta >= cfg->distance_threshold ? TRANSFORM_PINCH : TRANSFORM_ROTATE;
	if (!travelers_pad_arbiter_submit(&(struct travelers_pad_arbiter_proposal){
			.frame_seq = data->arbiter_frame_seq,
			.recognizer = TRAVELERS_PAD_RECOGNIZER_TRANSFORM_GESTURE,
			.intent = TRAVELERS_PAD_PROPOSAL_CLAIM,
			.requested_owner = TRAVELERS_PAD_OWNER_TRANSFORM_GESTURE,
			.capabilities = TRAVELERS_PAD_CAP_BEHAVIOR_ACTION,
			.contact_count = data->fingers,
			.evidence = ((uint32_t)data->mode << 30) | distance_delta,
	})) {
		data->claimed = true;
		LOG_INF("2F transform=%s proposed distance=%u rotation=%u", mode_name(data->mode),
			distance_delta, rotation_cross);
	}
}

static void transform_deliver_resolved_frame(
	void *context, const struct travelers_pad_session_snapshot *snapshot,
	const struct travelers_pad_owner_token *token)
{
	struct transform_data *data = context;

	if (snapshot->cancelled || data->sensor_reset) {
		reset_transform(data);
		data->sensor_reset = false;
		return;
	}
	if (token != NULL && token->valid && token->owner == TRAVELERS_PAD_OWNER_TRANSFORM_GESTURE &&
	    snapshot->contact_count == 0) {
		LOG_INF("2F transform=%s completed", mode_name(data->mode));
		reset_transform(data);
	}
}

static void transform_handle_input(const struct device *dev, struct input_event *evt)
{
	struct transform_data *data = dev->data;

	if (evt->type == INPUT_EV_ABS) {
		switch (evt->code) {
		case INPUT_ABS_X:
			data->first_x = evt->value;
			break;
		case INPUT_ABS_Y:
			data->first_y = evt->value;
			break;
		case INPUT_ABS_RX:
			data->second_x = evt->value;
			break;
		case INPUT_ABS_RY:
			data->second_y = evt->value;
			break;
		case INPUT_ABS_RUDDER:
			data->second_valid = evt->value;
			break;
		case INPUT_ABS_Z:
			data->fingers = evt->value;
			break;
		case INPUT_ABS_RZ:
			if (evt->value) {
				data->sensor_reset = true;
			}
			break;
		default:
			break;
		}
	}
	if (evt->sync) {
		data->arbiter_frame_seq =
			travelers_pad_arbiter_next_frame(TRAVELERS_PAD_RECOGNIZER_TRANSFORM_GESTURE);
		recognize_transform(dev);
		travelers_pad_arbiter_observed(TRAVELERS_PAD_RECOGNIZER_TRANSFORM_GESTURE,
					       data->arbiter_frame_seq);
	}
}

static int transform_init(const struct device *dev)
{
	(void)travelers_pad_arbiter_register(TRAVELERS_PAD_RECOGNIZER_TRANSFORM_GESTURE);
	(void)travelers_pad_arbiter_bind_frame_handler(
		TRAVELERS_PAD_RECOGNIZER_TRANSFORM_GESTURE, transform_deliver_resolved_frame, dev->data);
	return 0;
}

#define TRANSFORM_INST(n)                                                                       \
	static struct transform_data transform_data_##n = {.dev = DEVICE_DT_INST_GET(n)};         \
	static const struct transform_config transform_config_##n = {                              \
		.distance_threshold = DT_INST_PROP_OR(n, distance_threshold, 30000),                 \
		.rotation_threshold = DT_INST_PROP_OR(n, rotation_threshold, 30000),                 \
	};                                                                                           \
	static void transform_input_handler_##n(struct input_event *evt)                           \
	{                                                                                            \
		transform_handle_input(DEVICE_DT_INST_GET(n), evt);                                    \
	}                                                                                            \
	INPUT_CALLBACK_DEFINE(DEVICE_DT_GET(DT_INST_PHANDLE(n, input_device)),                     \
			      transform_input_handler_##n);                                                 \
	DEVICE_DT_INST_DEFINE(n, transform_init, NULL, &transform_data_##n, &transform_config_##n, \
			      POST_KERNEL, CONFIG_TRAVELERS_PAD_IQS9150_FUNCTION_INIT_PRIORITY, NULL);

DT_INST_FOREACH_STATUS_OKAY(TRANSFORM_INST)
