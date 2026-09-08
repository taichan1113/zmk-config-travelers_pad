/*
 * Contact-session manager / 接触セッション管理器.
 *
 * Stage 1 only observes the normalized raw frame stream and establishes the
 * canonical session boundary. Legacy recognizers remain connected while they
 * are migrated one by one in later stages.
 */
#define DT_DRV_COMPAT zmk_travelers_pad_contact_session_manager

#include <zephyr/device.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <travelers_pad/contact_session.h>

LOG_MODULE_REGISTER(travelers_pad_session, LOG_LEVEL_INF);

struct session_data {
	uint32_t next_session_id;
	struct travelers_pad_session_snapshot snapshot;
	bool reset_pending;
};

static void session_process(const struct device *dev)
{
	struct session_data *data = dev->data;
	struct travelers_pad_session_snapshot *s = &data->snapshot;

	if (data->reset_pending) {
		if (s->contact_count || s->session_id) {
			LOG_WRN("session=%u cancelled", s->session_id);
		}
		*s = (struct travelers_pad_session_snapshot){.tap_allowed = false, .cancelled = true};
		data->reset_pending = false;
		return;
	}
	if (s->contact_count && s->session_id == 0) {
		s->session_id = ++data->next_session_id;
		s->peak_contact_count = s->contact_count;
		s->tap_allowed = true;
		LOG_INF("session=%u start contacts=%u", s->session_id, s->contact_count);
	}
	if (s->contact_count > s->peak_contact_count) {
		s->peak_contact_count = s->contact_count;
	}
	if (s->contact_count == 0 && s->session_id) {
		LOG_INF("session=%u end peak=%u owner=%u", s->session_id, s->peak_contact_count,
			s->owner);
		*s = (struct travelers_pad_session_snapshot){.tap_allowed = true};
	}
}

static void session_input(const struct device *dev, struct input_event *evt)
{
	struct session_data *data = dev->data;
	if (evt->type == INPUT_EV_ABS && evt->code == INPUT_ABS_Z) {
		data->snapshot.contact_count = evt->value;
	} else if (evt->type == INPUT_EV_ABS && evt->code == INPUT_ABS_RZ && evt->value) {
		data->reset_pending = true;
	}
	if (evt->sync) {
		data->snapshot.frame_seq++;
		data->snapshot.timestamp_ms = k_uptime_get();
		session_process(dev);
	}
}

#define SESSION_INST(n) \
	static struct session_data session_data_##n = {.snapshot.tap_allowed = true}; \
	static void session_input_##n(struct input_event *evt) { session_input(DEVICE_DT_INST_GET(n), evt); } \
	INPUT_CALLBACK_DEFINE(DEVICE_DT_GET(DT_INST_PHANDLE(n, input_device)), session_input_##n); \
	DEVICE_DT_INST_DEFINE(n, NULL, NULL, &session_data_##n, NULL, POST_KERNEL, \
		CONFIG_TRAVELERS_PAD_IQS9150_FUNCTION_INIT_PRIORITY, NULL);
DT_INST_FOREACH_STATUS_OKAY(SESSION_INST)
