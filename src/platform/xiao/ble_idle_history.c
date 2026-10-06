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

LOG_MODULE_REGISTER(ble_idle_history, LOG_LEVEL_INF);

#define HISTORY_COUNT 96
#define SAMPLE_MS 5000
static const struct gpio_dt_spec rdy = GPIO_DT_SPEC_GET(DT_NODELABEL(iqs9150), rdy_gpios);
static const struct device *const console = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
static atomic_t irq_count, raw_frames, pointer_events, contacts, queue_ticks;
static struct gpio_callback rdy_callback;
static struct k_work_delayable queue_probe;

struct snapshot {
	int64_t ms;
	uint32_t irqs, frames, pointer, queue;
	int8_t ready, connected, conn_state, transport;
	uint8_t vbus, fingers, cached_soc;
	uint16_t interval, latency, timeout;
};
static struct snapshot history[HISTORY_COUNT];
static unsigned int head, count;

static void on_rdy(const struct device *port, struct gpio_callback *cb, uint32_t pins)
{
	ARG_UNUSED(port); ARG_UNUSED(cb); ARG_UNUSED(pins);
	atomic_inc(&irq_count);
}

static void on_raw(struct input_event *event)
{
	if (event->type == INPUT_EV_ABS && event->code == INPUT_ABS_Z) {
		atomic_set(&contacts, event->value);
	}
	if (event->sync) {
		atomic_inc(&raw_frames);
	}
}
INPUT_CALLBACK_DEFINE(DEVICE_DT_GET(DT_NODELABEL(iqs9150)), on_raw);

static void on_pointer(struct input_event *event)
{
	if (event->type == INPUT_EV_REL && event->value != 0) {
		atomic_inc(&pointer_events);
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
	};
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
	LOG_INF("history begin: %u samples; timestamps are sample uptime; soc is cached", count);
	unsigned int first = (head + HISTORY_COUNT - count) % HISTORY_COUNT;
	for (unsigned int i = 0; i < count; i++) {
		const struct snapshot *s = &history[(first + i) % HISTORY_COUNT];
		LOG_INF("history t=%lld vbus=%u rdy=%d irq=%u raw=%u rel=%u fingers=%u queue=%u ble=%d state=%d ep=%d ci=%u lat=%u to=%u soc=%u",
			s->ms, s->vbus, s->ready, s->irqs, s->frames, s->pointer, s->fingers,
			s->queue, s->connected, s->conn_state, s->transport,
			s->interval, s->latency, s->timeout, s->cached_soc);
		/* Pace logging without blocking the driver/input/system workqueue. */
		k_sleep(K_MSEC(50));
	}
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
