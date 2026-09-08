/*
 * Travelers Pad two-finger scroll inertia.
 *
 * Canonical pointer wheel values exist only after the router has committed a
 * two-finger session. This function observes those values, but uses the raw
 * contact stream for physical release so router-internal cancellation frames
 * (such as pinch/rotation ownership) cannot start inertia.
 *
 * 標準ポインタのホイール値はrouterが2本指セッションを確定した後だけに存在する。
 * 本機能はその値を観測するが、物理的な解放は生接触ストリームで確認する。これにより
 * ピンチ／回転取得などrouter内部のキャンセルフレームで慣性を開始しない。
 */

#define DT_DRV_COMPAT zmk_travelers_pad_scroll_inertia

#include <zephyr/device.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(travelers_pad_scroll_inertia, LOG_LEVEL_INF);

struct scroll_inertia_config {
	const struct device *wheel_output_device;
	uint16_t start_delay_ms;
	uint16_t repeat_ms;
	uint8_t decay_numerator;
	uint8_t decay_denominator;
	uint16_t stop_delta;
	uint16_t max_delta;
};

struct scroll_inertia_data {
	const struct device *dev;
	uint8_t contact_fingers;
	int16_t horizontal_velocity;
	int16_t vertical_velocity;
	bool emitting;
	bool release_pending;
	struct k_work_delayable work;
};

static int16_t absolute_int16(int16_t value)
{
	return value < 0 ? -value : value;
}

static int16_t clamp_delta(int16_t value, uint16_t maximum)
{
	if (value > (int16_t)maximum) {
		return maximum;
	}
	if (value < -(int16_t)maximum) {
		return -maximum;
	}
	return value;
}

static int16_t decay_velocity(int16_t value, const struct scroll_inertia_config *cfg)
{
	return (int16_t)(((int32_t)value * cfg->decay_numerator) / cfg->decay_denominator);
}

static void stop_inertia(const struct device *dev)
{
	struct scroll_inertia_data *data = dev->data;

	k_work_cancel_delayable(&data->work);
	data->horizontal_velocity = 0;
	data->vertical_velocity = 0;
}

static void inertia_work_handler(struct k_work *work)
{
	struct scroll_inertia_data *data = CONTAINER_OF(work, struct scroll_inertia_data, work.work);
	const struct scroll_inertia_config *cfg = data->dev->config;
	int rc;

	if (data->contact_fingers != 0) {
		stop_inertia(data->dev);
		return;
	}

	data->horizontal_velocity = decay_velocity(data->horizontal_velocity, cfg);
	data->vertical_velocity = decay_velocity(data->vertical_velocity, cfg);
	if (absolute_int16(data->horizontal_velocity) < cfg->stop_delta &&
	    absolute_int16(data->vertical_velocity) < cfg->stop_delta) {
		LOG_INF("scroll inertia stopped");
		stop_inertia(data->dev);
		return;
	}

	data->emitting = true;
	if (data->horizontal_velocity != 0) {
		rc = input_report_rel(cfg->wheel_output_device, INPUT_REL_HWHEEL,
				      data->horizontal_velocity, data->vertical_velocity == 0, K_NO_WAIT);
		if (rc != 0) {
			LOG_ERR("inertia horizontal report failed: %d", rc);
		}
	}
	if (data->vertical_velocity != 0) {
		rc = input_report_rel(cfg->wheel_output_device, INPUT_REL_WHEEL,
				      data->vertical_velocity, true, K_NO_WAIT);
		if (rc != 0) {
			LOG_ERR("inertia vertical report failed: %d", rc);
		}
	}
	data->emitting = false;
	k_work_schedule(&data->work, K_MSEC(cfg->repeat_ms));
}

static void inertia_handle_contact(const struct device *dev, struct input_event *evt)
{
	struct scroll_inertia_data *data = dev->data;
	const struct scroll_inertia_config *cfg = dev->config;
	if (evt->type == INPUT_EV_ABS && evt->code == INPUT_ABS_Z) {
		uint8_t previous_fingers = data->contact_fingers;

		data->contact_fingers = evt->value;
		if (data->contact_fingers != 0 && previous_fingers == 0) {
			/* Any new physical contact cancels an older glide. */
			stop_inertia(dev);
			data->release_pending = false;
		} else if (previous_fingers == 2) {
			data->release_pending = true;
		}
	}
	if (evt->sync && data->release_pending && data->contact_fingers == 0 &&
	    (data->horizontal_velocity != 0 || data->vertical_velocity != 0)) {
		LOG_INF("scroll inertia started h=%d v=%d", data->horizontal_velocity,
			data->vertical_velocity);
		k_work_schedule(&data->work, K_MSEC(cfg->start_delay_ms));
		data->release_pending = false;
	}
}

static void inertia_handle_wheel(const struct device *dev, struct input_event *evt)
{
	struct scroll_inertia_data *data = dev->data;
	const struct scroll_inertia_config *cfg = dev->config;
	int16_t value;

	if (data->emitting || data->contact_fingers != 2 || evt->type != INPUT_EV_REL) {
		return;
	}
	value = clamp_delta(evt->value, cfg->max_delta);
	if (evt->code == INPUT_REL_HWHEEL && value != 0) {
		data->horizontal_velocity = value;
		data->vertical_velocity = 0;
	} else if (evt->code == INPUT_REL_WHEEL && value != 0) {
		data->horizontal_velocity = 0;
		data->vertical_velocity = value;
	}
}

static int inertia_init(const struct device *dev)
{
	const struct scroll_inertia_config *cfg = dev->config;
	struct scroll_inertia_data *data = dev->data;

	k_work_init_delayable(&data->work, inertia_work_handler);
	LOG_INF("scroll inertia active: delay=%ums period=%ums decay=%u/%u stop=%u",
		cfg->start_delay_ms, cfg->repeat_ms, cfg->decay_numerator, cfg->decay_denominator,
		cfg->stop_delta);
	return 0;
}

#define SCROLL_INERTIA_INST(n)                                                                 \
	static struct scroll_inertia_data scroll_inertia_data_##n = {.dev = DEVICE_DT_INST_GET(n)};  \
	static const struct scroll_inertia_config scroll_inertia_config_##n = {                      \
		.wheel_output_device = DEVICE_DT_GET(DT_INST_PHANDLE(n, wheel_output_device)),          \
		.start_delay_ms = DT_INST_PROP_OR(n, start_delay_ms, 20),                                \
		.repeat_ms = DT_INST_PROP_OR(n, repeat_ms, 24),                                           \
		.decay_numerator = DT_INST_PROP_OR(n, decay_numerator, 3),                               \
		.decay_denominator = DT_INST_PROP_OR(n, decay_denominator, 4),                           \
		.stop_delta = DT_INST_PROP_OR(n, stop_delta, 2),                                         \
		.max_delta = DT_INST_PROP_OR(n, max_delta, 64),                                          \
	};                                                                                           \
	static void scroll_inertia_contact_handler_##n(struct input_event *evt)                      \
	{                                                                                            \
		inertia_handle_contact(DEVICE_DT_INST_GET(n), evt);                                      \
	}                                                                                            \
	static void scroll_inertia_wheel_handler_##n(struct input_event *evt)                        \
	{                                                                                            \
		inertia_handle_wheel(DEVICE_DT_INST_GET(n), evt);                                        \
	}                                                                                            \
	INPUT_CALLBACK_DEFINE(DEVICE_DT_GET(DT_INST_PHANDLE(n, contact_input_device)),               \
			      scroll_inertia_contact_handler_##n);                                          \
	INPUT_CALLBACK_DEFINE(DEVICE_DT_GET(DT_INST_PHANDLE(n, wheel_input_device)),                 \
			      scroll_inertia_wheel_handler_##n);                                            \
	DEVICE_DT_INST_DEFINE(n, inertia_init, NULL, &scroll_inertia_data_##n,                       \
			      &scroll_inertia_config_##n, POST_KERNEL,                                     \
			      CONFIG_TRAVELERS_PAD_IQS9150_FUNCTION_INIT_PRIORITY, NULL);

DT_INST_FOREACH_STATUS_OKAY(SCROLL_INERTIA_INST)
