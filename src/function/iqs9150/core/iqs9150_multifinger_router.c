/*
 * Travelers Pad IQS9150 multi-finger router.
 *
 * Raw contact data is classified before it reaches the ordinary pointer
 * function. This makes gesture ownership explicit: pinch/rotation and 3--5
 * finger sessions never leak into two-finger scroll output.
 *
 * 生の接触データを通常ポインタ機能の前で分類する。ピンチ・回転・3--5本指操作を
 * 二本指スクロールへ漏らさないため、操作の所有権をここで明示的に決める。
 */

#define DT_DRV_COMPAT zmk_travelers_pad_iqs9150_multifinger_router

#include <zephyr/device.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include <travelers_pad/tap_drag.h>
#include <travelers_pad/contact_arbiter.h>
#include <travelers_pad/edge_dial.h>
#include <travelers_pad/contact_session.h>

LOG_MODULE_REGISTER(travelers_pad_router, LOG_LEVEL_INF);

enum router_state {
	ROUTER_IDLE,
	ROUTER_PASS_ONE,
	ROUTER_AWAIT_TWO,
	ROUTER_SCROLL_TWO,
	ROUTER_CLAIM_GEOMETRY,
	ROUTER_CLAIM_MULTIFINGER,
	ROUTER_TAP_DRAG,
	ROUTER_EDGE_DIAL,
};

struct router_config {
	uint16_t scroll_classify_move;
	uint32_t geometry_distance_threshold;
	uint32_t geometry_rotation_threshold;
};

struct router_data {
	const struct device *dev;
	enum router_state state;
	uint16_t x;
	uint16_t y;
	uint16_t finger2_x;
	uint16_t finger2_y;
	bool finger2_valid;
	uint8_t fingers;
	uint8_t max_fingers;
	uint16_t start_x;
	uint16_t start_y;
	uint16_t start_finger2_x;
	uint16_t start_finger2_y;
	bool have_geometry;
	int32_t start_geometry_x;
	int32_t start_geometry_y;
	uint32_t start_distance_squared;
	bool sensor_reset;
	uint32_t shadow_frame_seq;
};

static int32_t abs32(int32_t value)
{
	return value < 0 ? -value : value;
}

static uint32_t squared_distance(int32_t x, int32_t y)
{
	return (uint32_t)(x * x + y * y);
}

static void router_emit(const struct device *dev, const struct router_data *data, uint8_t fingers)
{
	int rc = input_report_abs(dev, INPUT_ABS_X, data->x, false, K_NO_WAIT);

	rc |= input_report_abs(dev, INPUT_ABS_Y, data->y, false, K_NO_WAIT);
	rc |= input_report_abs(dev, INPUT_ABS_RX, data->finger2_x, false, K_NO_WAIT);
	rc |= input_report_abs(dev, INPUT_ABS_RY, data->finger2_y, false, K_NO_WAIT);
	rc |= input_report_abs(dev, INPUT_ABS_RUDDER, data->finger2_valid, false, K_NO_WAIT);
	rc |= input_report_abs(dev, INPUT_ABS_Z, fingers, false, K_NO_WAIT);
	rc |= input_report_key(dev, INPUT_BTN_TOUCH, fingers != 0, true, K_NO_WAIT);
	if (rc != 0) {
		LOG_ERR("routed contact frame report failed: %d", rc);
	}
}

static void router_cancel_pointer(const struct device *dev)
{
	int rc = input_report_abs(dev, INPUT_ABS_RZ, 1, false, K_NO_WAIT);

	rc |= input_report_abs(dev, INPUT_ABS_Z, 0, false, K_NO_WAIT);
	rc |= input_report_key(dev, INPUT_BTN_TOUCH, 0, true, K_NO_WAIT);
	if (rc != 0) {
		LOG_ERR("pointer cancellation report failed: %d", rc);
	}
}

static void router_begin(struct router_data *data)
{
	data->max_fingers = data->fingers;
	data->start_x = data->x;
	data->start_y = data->y;
	data->start_finger2_x = data->finger2_x;
	data->start_finger2_y = data->finger2_y;
	data->have_geometry = data->finger2_valid;
	if (data->have_geometry) {
		data->start_geometry_x = (int32_t)data->finger2_x - data->x;
		data->start_geometry_y = (int32_t)data->finger2_y - data->y;
		data->start_distance_squared = squared_distance(data->start_geometry_x,
							data->start_geometry_y);
	}
}

static const char *swipe_direction(const struct router_data *data)
{
	int32_t dx = (int32_t)data->x - data->start_x;
	int32_t dy = (int32_t)data->y - data->start_y;

	if (abs32(dx) >= abs32(dy)) {
		return dx >= 0 ? "right" : "left";
	}
	return dy >= 0 ? "down" : "up";
}

static void router_process_frame(const struct device *dev)
{
	const struct router_config *cfg = dev->config;
	struct router_data *data = dev->data;
	int32_t geometry_x;
	int32_t geometry_y;
	uint32_t distance_squared;
	uint32_t distance_delta;
	uint32_t rotation_cross;
	uint32_t primary_move;

	/* The tap/drag function owns movement after a confirmed double-tap hold.
	 * It emits relative movement with the button held, so never reinterpret the
	 * same raw frames as pointer motion or two-finger scrolling here. */
	if (travelers_pad_tap_drag_is_tracking()) {
		if (data->state != ROUTER_TAP_DRAG) {
			router_cancel_pointer(dev);
			data->state = ROUTER_TAP_DRAG;
			LOG_INF("tap/drag owns %uF contact stream", data->fingers);
		}
		if (data->fingers == 0) {
			data->state = ROUTER_IDLE;
			data->max_fingers = 0;
		}
		return;
	}
	if (data->state == ROUTER_TAP_DRAG) {
		if (data->fingers == 0) {
			data->state = ROUTER_IDLE;
			data->max_fingers = 0;
		}
		return;
	}

	if (data->sensor_reset) {
		router_cancel_pointer(dev);
		data->state = ROUTER_IDLE;
		data->max_fingers = 0;
		data->sensor_reset = false;
		LOG_WRN("router cancelled session after sensor reset");
		return;
	}

	/* An enabled edge claims a one-finger session as soon as the contact is
	 * acquired. Keep all its raw frames out of the pointer path until release;
	 * this prevents the initial edge travel before the first detent from moving
	 * the cursor. Two-or-more-finger contacts never activate the edge function.
	 * 有効辺は1本指接触を取得した時点で所有する。解放まで生フレームをポインタ経路
	 * から除外し、最初のステップ前の辺移動でカーソルが動くことを防ぐ。2本指以上の
	 * 接触はエッジ機能を有効化しない。 */
	if (travelers_pad_edge_dial_is_active()) {
		if (data->state != ROUTER_EDGE_DIAL) {
			router_cancel_pointer(dev);
			data->state = ROUTER_EDGE_DIAL;
			data->max_fingers = 1;
			LOG_INF("edge dial owns 1F contact stream");
		}
		return;
	}
	if (data->state == ROUTER_EDGE_DIAL) {
		if (data->fingers == 0) {
			data->state = ROUTER_IDLE;
			data->max_fingers = 0;
			LOG_INF("edge dial contact released; router idle");
		}
		return;
	}

	if (data->fingers == 0) {
		enum travelers_pad_owner owner = travelers_pad_session_owner();

		/* A terminal owner already emitted its semantic action. In particular,
		 * Tip-Tap must not create the router's legacy synthetic 2F tap pair. */
		if (owner == TRAVELERS_PAD_OWNER_TIP_TAP || owner == TRAVELERS_PAD_OWNER_MULTI_GESTURE ||
		    owner == TRAVELERS_PAD_OWNER_EDGE_DIAL || owner == TRAVELERS_PAD_OWNER_DRAG) {
			data->state = ROUTER_IDLE;
			data->max_fingers = 0;
			return;
		}
		if (data->state == ROUTER_PASS_ONE || data->state == ROUTER_SCROLL_TWO) {
			router_emit(dev, data, 0);
		} else if (data->state == ROUTER_AWAIT_TWO) {
			/* A held-still two-finger session is a right-click candidate. Send a
			 * synthetic start/release pair only now, so no provisional scroll can
			 * have escaped before the gesture was classified. */
			data->x = data->start_x;
			data->y = data->start_y;
			data->finger2_x = data->start_finger2_x;
			data->finger2_y = data->start_finger2_y;
			data->finger2_valid = data->have_geometry;
			router_emit(dev, data, 2);
			router_emit(dev, data, 0);
			LOG_INF("2F route=tap candidate");
		} else if (data->state == ROUTER_CLAIM_GEOMETRY) {
			LOG_INF("2F route=geometry candidate complete");
		} else if (data->state == ROUTER_CLAIM_MULTIFINGER) {
			if (data->max_fingers == 5) {
				LOG_INF("5F route=Grasp/Release candidate complete");
			} else {
				LOG_INF("%uF route=swipe candidate direction=%s", data->max_fingers,
					swipe_direction(data));
			}
		}
		data->state = ROUTER_IDLE;
		data->max_fingers = 0;
		return;
	}

	if (data->state == ROUTER_IDLE) {
		router_begin(data);
		if (data->fingers == 1) {
			data->state = ROUTER_PASS_ONE;
			router_emit(dev, data, 1);
		} else if (data->fingers == 2) {
			data->state = ROUTER_AWAIT_TWO;
			LOG_INF("2F route=awaiting scroll-or-geometry classification");
		} else {
			data->state = ROUTER_CLAIM_MULTIFINGER;
			if (data->fingers == 5) {
				LOG_INF("5F route=Grasp/Release claimed");
			} else {
				LOG_INF("%uF route=multifinger claimed", data->fingers);
			}
		}
		return;
	}

	data->max_fingers = MAX(data->max_fingers, data->fingers);
	if (data->state == ROUTER_PASS_ONE && data->fingers >= 2) {
		router_cancel_pointer(dev);
		router_begin(data);
		data->state = data->fingers == 2 ? ROUTER_AWAIT_TWO : ROUTER_CLAIM_MULTIFINGER;
		return;
	}
	if (data->state == ROUTER_SCROLL_TWO && data->fingers >= 3) {
		router_cancel_pointer(dev);
		data->state = ROUTER_CLAIM_MULTIFINGER;
		if (data->fingers == 5) {
			LOG_INF("5F route=Grasp/Release claimed; cancelled scroll");
		} else {
			LOG_INF("%uF route=multifinger claimed; cancelled scroll", data->fingers);
		}
		return;
	}
	if (data->state == ROUTER_AWAIT_TWO && data->fingers >= 3) {
		data->state = ROUTER_CLAIM_MULTIFINGER;
		if (data->fingers == 5) {
			LOG_INF("5F route=Grasp/Release claimed");
		} else {
			LOG_INF("%uF route=multifinger claimed", data->fingers);
		}
		return;
	}
	if (data->state == ROUTER_PASS_ONE) {
		router_emit(dev, data, 1);
		return;
	}
	if (data->state == ROUTER_SCROLL_TWO) {
		router_emit(dev, data, data->fingers);
		return;
	}
	if (data->state != ROUTER_AWAIT_TWO || data->fingers != 2) {
		return;
	}

	primary_move = abs32((int32_t)data->x - data->start_x) +
		       abs32((int32_t)data->y - data->start_y);
	if (data->finger2_valid && data->have_geometry) {
		geometry_x = (int32_t)data->finger2_x - data->x;
		geometry_y = (int32_t)data->finger2_y - data->y;
		distance_squared = squared_distance(geometry_x, geometry_y);
		distance_delta = abs32((int32_t)distance_squared - data->start_distance_squared);
		rotation_cross = abs32(data->start_geometry_x * geometry_y -
				       data->start_geometry_y * geometry_x);
		if (distance_delta >= cfg->geometry_distance_threshold ||
		    rotation_cross >= cfg->geometry_rotation_threshold) {
			data->state = ROUTER_CLAIM_GEOMETRY;
			LOG_INF("2F route=geometry claimed distance-delta=%u rotation=%u", distance_delta,
				rotation_cross);
			return;
		}
	}
	if (primary_move >= cfg->scroll_classify_move) {
		/* Shadow only: express the same transition to the future resolver before
		 * the legacy shared claim performs the currently-authoritative action.
		 * shadow専用: 現行shared claimが正として動く前に、同じ遷移を将来の
		 * resolverへproposalとして表す。 */
		(void)travelers_pad_arbiter_submit(&(struct travelers_pad_arbiter_proposal){
			.frame_seq = data->shadow_frame_seq,
			.recognizer = TRAVELERS_PAD_RECOGNIZER_ROUTER,
			.intent = TRAVELERS_PAD_PROPOSAL_CLAIM,
			.requested_owner = TRAVELERS_PAD_OWNER_SCROLL,
			.capabilities = TRAVELERS_PAD_CAP_SCROLL_DELTA,
			.contact_count = data->fingers,
			.evidence = primary_move,
		});
		if (!travelers_pad_session_claim(TRAVELERS_PAD_OWNER_SCROLL)) {
			LOG_WRN("2F scroll claim rejected");
			return;
		}
		/* Record the legacy result with this recognizer's queued-frame ID, not
		 * with mutable global session state that may already hold a later frame. */
		travelers_pad_arbiter_record_legacy_claim(data->shadow_frame_seq,
						  TRAVELERS_PAD_OWNER_SCROLL);
		data->state = ROUTER_SCROLL_TWO;
		router_emit(dev, data, 2);
		LOG_INF("2F route=scroll committed; Finger 1 is the scroll representative");
	}
}

static void router_handle_input(const struct device *dev, struct input_event *evt)
{
	struct router_data *data = dev->data;

	if (evt->type == INPUT_EV_ABS) {
		if (evt->code == INPUT_ABS_X) {
			data->x = evt->value;
		} else if (evt->code == INPUT_ABS_Y) {
			data->y = evt->value;
		} else if (evt->code == INPUT_ABS_RX) {
			data->finger2_x = evt->value;
		} else if (evt->code == INPUT_ABS_RY) {
			data->finger2_y = evt->value;
		} else if (evt->code == INPUT_ABS_RUDDER) {
			data->finger2_valid = evt->value;
		} else if (evt->code == INPUT_ABS_Z) {
			data->fingers = evt->value;
		} else if (evt->code == INPUT_ABS_RZ && evt->value) {
			data->sensor_reset = true;
		}
	}
	if (evt->sync) {
		data->shadow_frame_seq =
			travelers_pad_arbiter_next_frame(TRAVELERS_PAD_RECOGNIZER_ROUTER);
		router_process_frame(dev);
		travelers_pad_arbiter_observed(TRAVELERS_PAD_RECOGNIZER_ROUTER,
					       data->shadow_frame_seq);
	}
}

static int router_init(const struct device *dev)
{
	ARG_UNUSED(dev);
	(void)travelers_pad_arbiter_register(TRAVELERS_PAD_RECOGNIZER_ROUTER);
	return 0;
}

#define ROUTER_INST(n)                                                                        \
	static struct router_data router_data_##n = {.dev = DEVICE_DT_INST_GET(n)};                \
	static const struct router_config router_config_##n = {                                    \
		.scroll_classify_move = DT_INST_PROP_OR(n, scroll_classify_move, 35),               \
		.geometry_distance_threshold = DT_INST_PROP_OR(n, geometry_distance_threshold, 30000), \
		.geometry_rotation_threshold = DT_INST_PROP_OR(n, geometry_rotation_threshold, 30000), \
	};                                                                                           \
	static void router_input_handler_##n(struct input_event *evt) {                             \
		router_handle_input(DEVICE_DT_INST_GET(n), evt);                                      \
	}                                                                                            \
	INPUT_CALLBACK_DEFINE(DEVICE_DT_GET(DT_INST_PHANDLE(n, input_device)),                     \
			      router_input_handler_##n);                                                   \
	DEVICE_DT_INST_DEFINE(n, router_init, NULL, &router_data_##n, &router_config_##n, POST_KERNEL,   \
			      CONFIG_TRAVELERS_PAD_IQS9150_FUNCTION_INIT_PRIORITY, NULL);

DT_INST_FOREACH_STATUS_OKAY(ROUTER_INST)
