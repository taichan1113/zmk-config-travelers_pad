/* Diagnostic observations only. No sensor transactions, reset or host output.
 * 診断専用：センサー通信・リセット・ホストへの操作出力は行わない。 */
#include <zephyr/bluetooth/conn.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/init.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/atomic.h>
#include <hal/nrf_power.h>
#include <zmk/battery.h>
#include <zmk/ble.h>
#include <zmk/endpoints.h>
#include <iqs9150/diagnostics.h>

LOG_MODULE_REGISTER(ble_idle_history, LOG_LEVEL_INF);

#define HISTORY_COUNT 96
#define SAMPLE_MS 5000
static const struct gpio_dt_spec rdy = GPIO_DT_SPEC_GET(DT_NODELABEL(iqs9150), rdy_gpios);
static const struct device *const console = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
static atomic_t irq_count, raw_frames, pointer_events, contacts, queue_ticks;
static atomic_t raw_moves, contact_changes, raw_x, raw_y;
static atomic_t routed_frames, routed_moves, routed_resets, routed_contacts;
static atomic_t xy_events, wheel_events;

/* Callback-local positions are owned by the input thread; snapshots read only
 * atomic observations. Coordinate changes do not prove valid sensor tracking.
 * 座標状態はinput thread内だけで更新し、採取threadはatomic値だけ読む。
 * 座標変化の回数は、センサー追跡の正しさまで保証しない。 */
struct position_observer {
	int32_t x, y, previous_x, previous_y;
	int32_t second_x, second_y;
	uint8_t fingers, previous_fingers;
	bool reset, second_valid;
};
static struct position_observer raw_position, routed_position;
static struct gpio_callback rdy_callback;
static struct k_work_delayable queue_probe;

struct snapshot {
	int64_t ms;
	uint32_t irqs, frames, pointer, queue;
	int8_t ready, connected, conn_state, transport;
	uint8_t vbus, fingers, cached_soc;
	uint16_t interval, latency, timeout;
	uint32_t raw_motion, changes, routed, routed_motion, resets, xy, wheel;
	int32_t x, y;
	uint8_t routed_fingers;
	struct iqs9150_power_observation power;
};
static struct snapshot history[HISTORY_COUNT];
static unsigned int head, count;

/* Retain short contact-count changes that five-second samples can miss.
 * 5秒間隔では見えない短い指本数変化を保持する。 */
struct contact_transition {
	int64_t ms;
	int32_t x, y, second_x, second_y;
	uint32_t raw, xy;
	uint8_t before, after, second_valid, vbus, reset;
};
static struct contact_transition transitions[HISTORY_COUNT];
static unsigned int transition_head, transition_count;
static struct k_spinlock transition_lock;

static void record_transition(const struct position_observer *p)
{
	struct contact_transition event = {
		.ms = k_uptime_get(), .x = p->x, .y = p->y,
		.second_x = p->second_x, .second_y = p->second_y,
		.raw = atomic_get(&raw_frames), .xy = atomic_get(&xy_events),
		.before = p->previous_fingers, .after = p->fingers,
		.second_valid = p->second_valid, .reset = p->reset,
		.vbus = nrf_power_usbregstatus_vbusdet_get(NRF_POWER),
	};
	k_spinlock_key_t key = k_spin_lock(&transition_lock);
	transitions[transition_head] = event;
	transition_head = (transition_head + 1) % HISTORY_COUNT;
	transition_count = MIN(transition_count + 1, HISTORY_COUNT);
	k_spin_unlock(&transition_lock, key);
}

static void on_rdy(const struct device *port, struct gpio_callback *cb, uint32_t pins)
{
	ARG_UNUSED(port); ARG_UNUSED(cb); ARG_UNUSED(pins);
	atomic_inc(&irq_count);
}

static bool observe_position(struct position_observer *p, struct input_event *event)
{
	if (event->type == INPUT_EV_ABS) {
		switch (event->code) {
		case INPUT_ABS_X: p->x = event->value; break;
		case INPUT_ABS_Y: p->y = event->value; break;
		case INPUT_ABS_RX: p->second_x = event->value; break;
		case INPUT_ABS_RY: p->second_y = event->value; break;
		case INPUT_ABS_RUDDER: p->second_valid = event->value != 0; break;
		case INPUT_ABS_Z: p->fingers = event->value; break;
		case INPUT_ABS_RZ: p->reset |= event->value != 0; break;
		default: break;
		}
	}
	return event->sync;
}

static void finish_position(struct position_observer *p, atomic_t *moves)
{
	if (!p->reset && p->fingers == 1 && p->previous_fingers == 1 &&
	    (p->x != p->previous_x || p->y != p->previous_y)) {
		atomic_inc(moves);
	}
	p->previous_x = p->x;
	p->previous_y = p->y;
	p->previous_fingers = p->reset ? 0 : p->fingers;
	p->reset = false;
}

static void on_raw(struct input_event *event)
{
	if (observe_position(&raw_position, event)) {
		if (raw_position.fingers != raw_position.previous_fingers || raw_position.reset) {
			record_transition(&raw_position);
		}
		if (raw_position.fingers != raw_position.previous_fingers) {
			atomic_inc(&contact_changes);
		}
		atomic_set(&contacts, raw_position.fingers);
		atomic_set(&raw_x, raw_position.x);
		atomic_set(&raw_y, raw_position.y);
		finish_position(&raw_position, &raw_moves);
		atomic_inc(&raw_frames);
	}
}
INPUT_CALLBACK_DEFINE(DEVICE_DT_GET(DT_NODELABEL(iqs9150)), on_raw);

static void on_routed(struct input_event *event)
{
	if (observe_position(&routed_position, event)) {
		if (routed_position.reset) {
			atomic_inc(&routed_resets);
		}
		atomic_set(&routed_contacts, routed_position.fingers);
		finish_position(&routed_position, &routed_moves);
		atomic_inc(&routed_frames);
	}
}
INPUT_CALLBACK_DEFINE(DEVICE_DT_GET(DT_PHANDLE(DT_NODELABEL(iqs9150_pointer), input_device)),
		      on_routed);

static void on_pointer(struct input_event *event)
{
	if (event->type == INPUT_EV_REL && event->value != 0) {
		atomic_inc(&pointer_events);
		if (event->code == INPUT_REL_X || event->code == INPUT_REL_Y) {
			atomic_inc(&xy_events);
		} else if (event->code == INPUT_REL_WHEEL || event->code == INPUT_REL_HWHEEL) {
			atomic_inc(&wheel_events);
		}
	}
}
INPUT_CALLBACK_DEFINE(DEVICE_DT_GET(DT_NODELABEL(iqs9150_pointer)), on_pointer);

static void on_queue_probe(struct k_work *work)
{
	ARG_UNUSED(work);
	atomic_inc(&queue_ticks);
	k_work_reschedule(&queue_probe, K_MSEC(SAMPLE_MS));
}

static void collect(struct snapshot *out)
{
	*out = (struct snapshot){
		.ms = k_uptime_get(), .irqs = atomic_get(&irq_count),
		.frames = atomic_get(&raw_frames), .pointer = atomic_get(&pointer_events),
		.queue = atomic_get(&queue_ticks), .ready = gpio_pin_get_dt(&rdy),
		.vbus = nrf_power_usbregstatus_vbusdet_get(NRF_POWER),
		.fingers = atomic_get(&contacts), .cached_soc = zmk_battery_state_of_charge(),
		.connected = zmk_ble_active_profile_is_connected(), .conn_state = -1,
		.transport = zmk_endpoints_selected().transport,
		.raw_motion = atomic_get(&raw_moves), .changes = atomic_get(&contact_changes),
		.x = atomic_get(&raw_x), .y = atomic_get(&raw_y),
		.routed = atomic_get(&routed_frames), .routed_motion = atomic_get(&routed_moves),
		.resets = atomic_get(&routed_resets), .routed_fingers = atomic_get(&routed_contacts),
		.xy = atomic_get(&xy_events), .wheel = atomic_get(&wheel_events),
	};
	iqs9150_get_power_observation(DEVICE_DT_GET(DT_NODELABEL(iqs9150)), &out->power);
	/* Timestamp after copying: an observation concurrent with collect is not
	 * falsely reported as coming from the future. */
	out->ms = k_uptime_get();
	struct bt_conn *conn = zmk_ble_active_profile_conn();
	if (conn != NULL) {
		struct bt_conn_info info;
		if (bt_conn_get_info(conn, &info) == 0) {
			out->conn_state = info.state;
			if (info.type == BT_CONN_TYPE_LE) {
				out->interval = info.le.interval;
				out->latency = info.le.latency;
				out->timeout = info.le.timeout;
			}
		}
		bt_conn_unref(conn);
	}
}

static void dump_history(void)
{
	/* Copy the transition ring before logging; never hold a spinlock across
	 * LOG or sleep. No recognizer/barrier is registered by this observer.
	 * ログ前にcopyし、LOG/sleep中はlockしない。認識器/barrierの追加はしない。 */
	static struct contact_transition transition_copy[HISTORY_COUNT];
	k_spinlock_key_t key = k_spin_lock(&transition_lock);
	unsigned int saved_count = transition_count;
	unsigned int start = (transition_head + HISTORY_COUNT - saved_count) % HISTORY_COUNT;
	for (unsigned int i = 0; i < saved_count; i++) {
		transition_copy[i] = transitions[(start + i) % HISTORY_COUNT];
	}
	k_spin_unlock(&transition_lock, key);
	LOG_INF("history begin: %u samples; timestamps are sample uptime; soc is cached", count);
	unsigned int first = (head + HISTORY_COUNT - count) % HISTORY_COUNT;
	for (unsigned int i = 0; i < count; i++) {
		const struct snapshot *s = &history[(first + i) % HISTORY_COUNT];
		LOG_INF("history t=%lld vbus=%u rdy=%d irq=%u raw=%u rel=%u fingers=%u queue=%u ble=%d state=%d ep=%d ci=%u lat=%u to=%u soc=%u",
			s->ms, s->vbus, s->ready, s->irqs, s->frames, s->pointer, s->fingers,
			s->queue, s->connected, s->conn_state, s->transport,
			s->interval, s->latency, s->timeout, s->cached_soc);
		LOG_INF("flow t=%lld raw-move=%u changes=%u pos=%d/%d routed=%u routed-move=%u routed-fingers=%u markers=%u xy=%u wheel=%u",
			s->ms, s->raw_motion, s->changes, s->x, s->y, s->routed,
			s->routed_motion, s->routed_fingers, s->resets, s->xy, s->wheel);
		/* Pace logging without blocking the driver/input/system workqueue. */
		LOG_INF("sensor t=%lld observed-at=%lld age-ms=%lld last-mode=%u info=0x%04x alp-prox=%u alp-ati-error=%u mode-reports=%u/%u/%u/%u/%u reserved=%u/%u/%u",
			s->ms, s->power.sampled_ms, s->power.sampled_ms ? s->ms-s->power.sampled_ms : -1LL,
			s->power.info_flags&7, s->power.info_flags, !!(s->power.info_flags&BIT(8)), !!(s->power.info_flags&BIT(5)),
			s->power.mode_reports[0],s->power.mode_reports[1],s->power.mode_reports[2],s->power.mode_reports[3],s->power.mode_reports[4],
			s->power.mode_reports[5],s->power.mode_reports[6],s->power.mode_reports[7]);
		k_sleep(K_MSEC(50));
	}
	LOG_INF("contacts begin: %u retained count-change/reset frames", saved_count);
	for (unsigned int i = 0; i < saved_count; i++) {
		const struct contact_transition *s = &transition_copy[i];
		LOG_INF("contact t=%lld vbus=%u count=%u->%u f1=%d/%d f2=%d/%d valid2=%u reset=%u raw=%u xy=%u",
			s->ms, s->vbus, s->before, s->after, s->x, s->y,
			s->second_x, s->second_y, s->second_valid, s->reset, s->raw, s->xy);
		k_sleep(K_MSEC(50));
	}
	LOG_INF("contacts end");
	LOG_INF("history end");
}

static void history_thread(void *a, void *b, void *c)
{
	ARG_UNUSED(a); ARG_UNUSED(b); ARG_UNUSED(c);
	bool previously_open = false;
	for (;;) {
		collect(&history[head]);
		head = (head + 1) % HISTORY_COUNT;
		count = MIN(count + 1, HISTORY_COUNT);
		uint32_t dtr = 0;
		bool open = nrf_power_usbregstatus_vbusdet_get(NRF_POWER) &&
			    uart_line_ctrl_get(console, UART_LINE_CTRL_DTR, &dtr) == 0 && dtr;
		if (open && !previously_open) {
			dump_history();
		}
		previously_open = open;
		k_sleep(K_MSEC(SAMPLE_MS));
	}
}

K_THREAD_STACK_DEFINE(history_stack, 2048);
static struct k_thread history_thread_data;

static int history_init(void)
{
	if (!gpio_is_ready_dt(&rdy) || !device_is_ready(console)) {
		return -ENODEV;
	}
	/* Reuse the driver's existing IRQ configuration; only add an observer. */
	gpio_init_callback(&rdy_callback, on_rdy, BIT(rdy.pin));
	int rc = gpio_add_callback(rdy.port, &rdy_callback);
	if (rc != 0) {
		return rc;
	}
	k_work_init_delayable(&queue_probe, on_queue_probe);
	k_work_schedule(&queue_probe, K_MSEC(SAMPLE_MS));
	k_thread_create(&history_thread_data, history_stack, K_THREAD_STACK_SIZEOF(history_stack),
			history_thread, NULL, NULL, NULL, 10, 0, K_SECONDS(2));
	k_thread_name_set(&history_thread_data, "ble-idle-history");
	LOG_INF("RAM history enabled: 96 samples, 5s interval; reconnect serial to dump");
	return 0;
}
SYS_INIT(history_init, APPLICATION, 99);
