/*
 * IQS9150 EV02 bring-up driver.
 *
 * STEP1B: Apply the Azoteq GUI-generated initialization data, run ATI, then
 * enable Event Mode.  Pointer reporting is intentionally not implemented yet.
 */

#define DT_DRV_COMPAT azoteq_iqs9150

#include <errno.h>
#include <string.h>

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include "iqs9150_ev02_init.h"

LOG_MODULE_REGISTER(iqs9150_step1a, CONFIG_IQS9150_STEP1A_LOG_LEVEL);

#define IQS9150_PRODUCT_NUMBER_REG       0x1000
#define IQS9150_RELATIVE_X_REG            0x1014
#define IQS9150_INFO_FLAGS_REG            0x1020
#define IQS9150_SYSTEM_CONTROL_REG        0x11BC
#define IQS9150_CONFIG_SETTINGS_REG       0x11BE

#define IQS9150_PRODUCT_NUMBER            0x076A
#define IQS9150_REPORT_LEN                 24
#define IQS9150_WRITE_PAYLOAD_MAX          30
#define IQS9150_ATI_POLL_INTERVAL_MS        20

#define IQS9150_SYSTEM_CONTROL_ACK_RESET  BIT(7)
#define IQS9150_SYSTEM_CONTROL_ALP_REATI  BIT(6)
#define IQS9150_SYSTEM_CONTROL_TP_REATI   BIT(5)

#define IQS9150_INFO_TP_ATI_ERROR          BIT(3)
#define IQS9150_INFO_TP_REATI_OCCURRED     BIT(4)
#define IQS9150_INFO_SHOW_RESET             BIT(7)
#define IQS9150_ACK_RETRY_MAX               3

#define IQS9150_REPORT_GESTURE_X_OFFSET      4
#define IQS9150_REPORT_GESTURE_Y_OFFSET      6
#define IQS9150_REPORT_1F_GESTURE_OFFSET     8
#define IQS9150_REPORT_2F_GESTURE_OFFSET    10

#define IQS9150_1F_TAP_MASK                 (BIT(0) | BIT(1) | BIT(2))
#define IQS9150_2F_TAP_MASK                 (BIT(0) | BIT(1) | BIT(2))
#define IQS9150_2F_VERTICAL_SCROLL          BIT(6)
#define IQS9150_2F_HORIZONTAL_SCROLL        BIT(7)

#define IQS9150_SCROLL_DIVISOR               4
#define IQS9150_SOFTWARE_TAP_MAX_MS        350
#define IQS9150_SOFTWARE_TAP_MAX_MOVE       75

struct iqs9150_block {
	uint16_t reg;
	const uint8_t *data;
	size_t len;
};

enum iqs9150_state {
	IQS9150_ACK_RESET,
	IQS9150_VERIFY_RESET_ACK,
	IQS9150_WRITE_CONFIG,
	IQS9150_START_ATI,
	IQS9150_WAIT_ATI,
	IQS9150_ENABLE_EVENT_MODE,
	IQS9150_RUNNING,
	IQS9150_ERROR,
};

struct iqs9150_config {
	struct i2c_dt_spec i2c;
	struct gpio_dt_spec rdy_gpio;
};

struct iqs9150_data {
	const struct device *dev;
	struct gpio_callback rdy_callback;
	struct k_work_delayable work;
	enum iqs9150_state state;
	uint8_t ack_attempts;
	uint8_t touch_max_fingers;
	bool touch_moved;
	int64_t touch_started_ms;
	uint16_t touch_start_x;
	uint16_t touch_start_y;
	size_t block_index;
	size_t block_offset;
};

static const uint8_t ev02_115c[] = {0};
static const uint8_t ev02_1176[] = {0xFF, 0xFF, 0x03, 0x00};
static const uint8_t ev02_117a[] = {
	0x21, 0x4B, 0x87, 0x5E, 0x87, 0x5E, 0x87, 0x5C, 0x87, 0x5E,
	0x87, 0x5E, 0x87, 0x5E, 0x87, 0x5E, 0x87, 0x5E, 0x87, 0x5C,
	0x87, 0x5C, 0x87, 0x5C, 0x87, 0x5E, 0x87, 0x5E,
};
static const uint8_t ev02_1196[] = {
	0xFA, 0x00, 0x2C, 0x01, 0x32, 0x00, 0x32, 0x00, 0xE8, 0x03, 0x32, 0x14,
};
static const uint8_t ev02_11a2[] = {
	0x0A, 0x00, 0x32, 0x00, 0x32, 0x00, 0x32, 0x00, 0x64, 0x00, 0x0A, 0x00,
	0x3C, 0x00, 0x05, 0x00, 0x28, 0x00, 0xDC, 0x05, 0x05, 0x08, 0x64, 0x00,
	0x14, 0x00,
};
static const uint8_t ev02_11be[] = {
	CONFIG_SETTINGS_0, CONFIG_SETTINGS_1, OTHER_SETTINGS_0, OTHER_SETTINGS_1,
};
static const uint8_t ev02_11c2[] = {0xFF, 0xFF, 0xFF, 0xD3, 0x00, 0x00, 0x00, 0xFC, 0xFF, 0x0F};
static const uint8_t ev02_11cc[] = {0x1A, 0x14, 0x08, 0x08, 0x02, 0x02, 0x32, 0x32};
static const uint8_t ev02_11d4[] = {0x46, 0x04, 0xB4, 0x64};
static const uint8_t ev02_11d8[] = {0x28, 0x02, 0x02, 0x10, 0x07, 0x07, 0x00, 0x44, 0x40, 0x4B};
static const uint8_t ev02_11e2[] = {
	0x28, 0x1A, 0x12, 0x07, 0xD0, 0x07, 0xBD, 0x04, 0x06, 0x00,
	0x7C, 0x00, 0x07, 0x80, 0x14, 0x03, 0x14, 0x14, 0x03, 0x14,
};
static const uint8_t ev02_11f6[] = {
	0x1F, 0xFF, 0xFF, 0x00, 0x96, 0x00, 0x96, 0x00, 0x32, 0x00, 0x2C, 0x01,
	0x96, 0x00, 0x2C, 0x01, 0x96, 0x00, 0x64, 0x00, 0x32, 0x00, 0x25, 0x25,
	0x64, 0x00, 0x0A, 0x00, 0x08, 0x00, 0x05, 0x00, 0x64, 0x00,
};
static const uint8_t ev02_1218[] = {
	0x19, 0x0C, 0x18, 0x0B, 0x17, 0x0A, 0x16, 0x09, 0x15, 0x08, 0x14, 0x13,
	0x0E, 0x02, 0x0F, 0x03, 0x10, 0x04, 0x11, 0x07, 0x05, 0x12, 0x06, 0x01,
	0x0D, 0x00, 0x2B, 0x2A, 0x29, 0x28, 0x27, 0x26, 0x25, 0x24, 0x23, 0x21,
	0x22, 0x20, 0x1B, 0x1C, 0x1A, 0x1E, 0x1D, 0x1F, 0x00, 0x00,
};
static const uint8_t ev02_zero_88[88];
static const uint8_t ev02_zero_506[506];
static const uint8_t ev02_zero_252[252];
static const uint8_t ev02_2000[] = {0x01, 0x00, 0x03, 0x03, 0x14, 0x00};

static const struct iqs9150_block ev02_blocks[] = {
	{0x115C, ev02_115c, 26},
	{0x1176, ev02_1176, sizeof(ev02_1176)},
	{0x117A, ev02_117a, sizeof(ev02_117a)},
	{0x1196, ev02_1196, sizeof(ev02_1196)},
	{0x11A2, ev02_11a2, sizeof(ev02_11a2)},
	{0x11BE, ev02_11be, sizeof(ev02_11be)},
	{0x11C2, ev02_11c2, sizeof(ev02_11c2)},
	{0x11CC, ev02_11cc, sizeof(ev02_11cc)},
	{0x11D4, ev02_11d4, sizeof(ev02_11d4)},
	{0x11D8, ev02_11d8, sizeof(ev02_11d8)},
	{0x11E2, ev02_11e2, sizeof(ev02_11e2)},
	{0x11F6, ev02_11f6, sizeof(ev02_11f6)},
	{0x1218, ev02_1218, sizeof(ev02_1218)},
	{0x1246, ev02_zero_88, sizeof(ev02_zero_88)},
	{0x129E, ev02_zero_88, sizeof(ev02_zero_88)},
	{0x12F6, ev02_zero_506, sizeof(ev02_zero_506)},
	{0x14F0, ev02_zero_252, sizeof(ev02_zero_252)},
	{0x2000, ev02_2000, sizeof(ev02_2000)},
};

static int read_reg(const struct device *dev, uint16_t reg, uint8_t *data, size_t len)
{
	const struct iqs9150_config *cfg = dev->config;
	uint8_t address[] = {reg & 0xff, reg >> 8};

	return i2c_write_read_dt(&cfg->i2c, address, sizeof(address), data, len);
}

static int write_reg(const struct device *dev, uint16_t reg, const uint8_t *data, size_t len)
{
	const struct iqs9150_config *cfg = dev->config;
	uint8_t buffer[2 + IQS9150_WRITE_PAYLOAD_MAX];

	if (len > IQS9150_WRITE_PAYLOAD_MAX) {
		return -EINVAL;
	}

	buffer[0] = reg & 0xff;
	buffer[1] = reg >> 8;
	memcpy(&buffer[2], data, len);
	return i2c_write_dt(&cfg->i2c, buffer, len + 2);
}

static uint16_t le16(const uint8_t *data)
{
	return (uint16_t)data[0] | ((uint16_t)data[1] << 8);
}

static void report_tap_click(const struct device *dev, uint16_t button)
{
	/* Each state must be synchronized separately.  Combining press and release
	 * in one input frame leaves the final HID report released, so the host never
	 * observes a click. */
	int rc = input_report_key(dev, button, 1, true, K_NO_WAIT);

	if (rc == 0) {
		rc = input_report_key(dev, button, 0, true, K_NO_WAIT);
	}
	if (rc != 0) {
		LOG_ERR("tap click report failed: %d", rc);
	}
}

static int16_t scale_scroll_delta(int16_t delta)
{
	int16_t scaled = delta / IQS9150_SCROLL_DIVISOR;

	if (scaled == 0 && delta != 0) {
		return delta > 0 ? 1 : -1;
	}
	return scaled;
}

static void report_software_tap(struct iqs9150_data *data, const uint8_t *report)
{
	uint8_t fingers = le16(&report[14]) & 0x0f;
	uint16_t x = le16(&report[16]);
	uint16_t y = le16(&report[18]);
	int64_t now = k_uptime_get();
	int delta_x;
	int delta_y;

	if (fingers != 0) {
		if (data->touch_max_fingers == 0) {
			data->touch_started_ms = now;
			data->touch_start_x = x;
			data->touch_start_y = y;
			data->touch_moved = false;
		}
		data->touch_max_fingers = MAX(data->touch_max_fingers, fingers);
		delta_x = (int)x - (int)data->touch_start_x;
		delta_y = (int)y - (int)data->touch_start_y;
		if (delta_x > IQS9150_SOFTWARE_TAP_MAX_MOVE ||
		    delta_x < -IQS9150_SOFTWARE_TAP_MAX_MOVE ||
		    delta_y > IQS9150_SOFTWARE_TAP_MAX_MOVE ||
		    delta_y < -IQS9150_SOFTWARE_TAP_MAX_MOVE) {
			data->touch_moved = true;
		}
		return;
	}

	if (data->touch_max_fingers != 0 && !data->touch_moved &&
	    now - data->touch_started_ms <= IQS9150_SOFTWARE_TAP_MAX_MS) {
		if (data->touch_max_fingers == 1) {
			LOG_INF("software 1-finger tap (%lld ms)", now - data->touch_started_ms);
			report_tap_click(data->dev, INPUT_BTN_0);
		} else if (data->touch_max_fingers == 2) {
			LOG_INF("software 2-finger tap (%lld ms)", now - data->touch_started_ms);
			report_tap_click(data->dev, INPUT_BTN_1);
		}
	} else if (data->touch_max_fingers != 0) {
		LOG_INF("tap rejected: fingers=%u duration=%lld ms moved=%d",
			data->touch_max_fingers, now - data->touch_started_ms, data->touch_moved);
	}
	data->touch_max_fingers = 0;
}

static void log_report(const struct device *dev)
{
	struct iqs9150_data *data = dev->data;
	uint8_t report[IQS9150_REPORT_LEN];
	int rc = read_reg(dev, IQS9150_RELATIVE_X_REG, report, sizeof(report));
	uint8_t gestures_1f;
	uint8_t gestures_2f;
	uint8_t fingers;
	bool scrolling;

	if (rc != 0) {
		LOG_ERR("report read failed: %d", rc);
		return;
	}

	LOG_DBG("status=0x%04x tp=0x%04x fingers=%u rel=(%d,%d) abs1=(%u,%u) strength=%u area=%u",
		le16(&report[12]), le16(&report[14]), le16(&report[14]) & 0x0f,
		(int16_t)le16(&report[0]), (int16_t)le16(&report[2]), le16(&report[16]),
		le16(&report[18]), le16(&report[20]), le16(&report[22]));

	gestures_1f = report[IQS9150_REPORT_1F_GESTURE_OFFSET];
	gestures_2f = report[IQS9150_REPORT_2F_GESTURE_OFFSET];
	fingers = le16(&report[14]) & 0x0f;
	/* Keep scrolling after a pause: the ASIC scroll-state bit can clear while
	 * two fingers remain down, but the reported relative deltas stay valid. */
	scrolling = fingers >= 2;
	if ((gestures_1f & IQS9150_1F_TAP_MASK) || (gestures_2f & IQS9150_2F_TAP_MASK)) {
		LOG_INF("hardware tap gesture: 1f=0x%02x 2f=0x%02x", gestures_1f,
			gestures_2f);
	}

	/* input-listener turns relative and wheel events into ZMK mouse HID. */
	if (scrolling) {
		int16_t gesture_x = le16(&report[IQS9150_REPORT_GESTURE_X_OFFSET]);
		int16_t gesture_y = le16(&report[IQS9150_REPORT_GESTURE_Y_OFFSET]);
		int16_t scroll_x = gesture_x != 0 ? gesture_x : le16(&report[0]);
		int16_t scroll_y = gesture_y != 0 ? gesture_y : le16(&report[2]);

		if (scroll_x != 0) {
			rc = input_report_rel(dev, INPUT_REL_HWHEEL, scale_scroll_delta(scroll_x),
					      scroll_y == 0, K_NO_WAIT);
			if (rc != 0) {
				LOG_ERR("horizontal scroll report failed: %d", rc);
			}
		}
		if (scroll_y != 0) {
			rc = input_report_rel(dev, INPUT_REL_WHEEL, -scale_scroll_delta(scroll_y), true,
					      K_NO_WAIT);
			if (rc != 0) {
				LOG_ERR("vertical scroll report failed: %d", rc);
			}
		}
	} else {
		if (report[0] != 0 || report[1] != 0) {
			rc = input_report_rel(dev, INPUT_REL_X, (int16_t)le16(&report[0]),
					      report[2] == 0 && report[3] == 0, K_NO_WAIT);
			if (rc != 0) {
				LOG_ERR("pointer X report failed: %d", rc);
			}
		}
		if (report[2] != 0 || report[3] != 0) {
			rc = input_report_rel(dev, INPUT_REL_Y, (int16_t)le16(&report[2]), true,
					      K_NO_WAIT);
			if (rc != 0) {
				LOG_ERR("pointer Y report failed: %d", rc);
			}
		}
	}

	/* The EV02 reports finger count and coordinates reliably, so use them for
	 * taps as well as retaining the ASIC gesture flags as diagnostics. */
	report_software_tap(data, report);
}

static void iqs9150_fail(struct iqs9150_data *data, const char *operation, int rc)
{
	data->state = IQS9150_ERROR;
	LOG_ERR("STEP1B %s failed: %d", operation, rc);
}

static void iqs9150_work_handler(struct k_work *work)
{
	struct k_work_delayable *delayable = k_work_delayable_from_work(work);
	struct iqs9150_data *data = CONTAINER_OF(delayable, struct iqs9150_data, work);
	const struct device *dev = data->dev;
	uint8_t value[4];
	int rc;

	switch (data->state) {
	case IQS9150_ACK_RESET:
		/* A read followed by a separate write inserts STOP and closes the RDY
		 * communication window. At power-up the documented System Control
		 * value is 0x0002, so acknowledge Show Reset with one complete
		 * 16-bit System Control write (0x0082). */
		value[0] = IQS9150_SYSTEM_CONTROL_ACK_RESET | BIT(1);
		value[1] = 0;
		rc = write_reg(dev, IQS9150_SYSTEM_CONTROL_REG, value, 2);
		if (rc != 0) {
			iqs9150_fail(data, "reset ACK", rc);
			return;
		}
		data->ack_attempts++;
		data->state = IQS9150_VERIFY_RESET_ACK;
		LOG_INF("STEP1B reset ACK requested; verifying on next RDY");
		return;

	case IQS9150_VERIFY_RESET_ACK:
		rc = read_reg(dev, IQS9150_INFO_FLAGS_REG, value, 2);
		if (rc != 0) {
			iqs9150_fail(data, "reset ACK verification", rc);
			return;
		}
		if (value[0] & IQS9150_INFO_SHOW_RESET) {
			if (data->ack_attempts >= IQS9150_ACK_RETRY_MAX) {
				LOG_ERR("STEP1B Show Reset remains set; Info Flags=0x%02x%02x",
					value[1], value[0]);
				iqs9150_fail(data, "reset ACK", -EIO);
				return;
			}
			data->state = IQS9150_ACK_RESET;
			LOG_WRN("STEP1B Show Reset still set; retrying reset ACK");
			return;
		}
		data->state = IQS9150_WRITE_CONFIG;
		LOG_INF("STEP1B reset acknowledged; writing EV02 initialization data");
		return;

	case IQS9150_WRITE_CONFIG: {
		const struct iqs9150_block *block;
		size_t chunk;

		if (data->block_index == ARRAY_SIZE(ev02_blocks)) {
			data->state = IQS9150_START_ATI;
			LOG_INF("STEP1B initialization data written; starting ATI");
			return;
		}

		block = &ev02_blocks[data->block_index];
		chunk = MIN(IQS9150_WRITE_PAYLOAD_MAX, block->len - data->block_offset);
		rc = write_reg(dev, block->reg + data->block_offset,
			       block->data + data->block_offset, chunk);
		if (rc != 0) {
			iqs9150_fail(data, "initialization data write", rc);
			return;
		}

		data->block_offset += chunk;
		if (data->block_offset == block->len) {
			data->block_index++;
			data->block_offset = 0;
		}
		return;
	}

	case IQS9150_START_ATI:
		/* IQS9150 needs only the Trackpad re-ATI action here.  Keep this as a
		 * complete 16-bit write during the RDY window; requesting ALP re-ATI
		 * simultaneously can introduce an unrelated calibration failure. */
		value[0] = IQS9150_SYSTEM_CONTROL_TP_REATI;
		value[1] = 0;
		rc = write_reg(dev, IQS9150_SYSTEM_CONTROL_REG, value, 2);
		if (rc != 0) {
			iqs9150_fail(data, "ATI start", rc);
			return;
		}
		data->state = IQS9150_WAIT_ATI;
		LOG_INF("STEP1B ATI requested; waiting for completion");
		return;

	case IQS9150_WAIT_ATI:
		/* This handler is entered from the RDY falling-edge callback. Do not
		 * poll I2C during ATI: RDY may be low while conversions are running. */
		rc = read_reg(dev, IQS9150_INFO_FLAGS_REG, value, 2);
		if (rc != 0) {
			iqs9150_fail(data, "ATI result read", rc);
			return;
		}
		if (value[0] & IQS9150_INFO_TP_ATI_ERROR) {
			LOG_ERR("STEP1B ATI completed with error; Info Flags=0x%02x%02x",
				value[1], value[0]);
			iqs9150_fail(data, "ATI (trackpad ATI error flag set)", -EIO);
			return;
		}

		data->state = IQS9150_ENABLE_EVENT_MODE;
		LOG_INF("STEP1B ATI complete; enabling Event Mode");
		k_work_schedule(&data->work, K_NO_WAIT);
		return;

	case IQS9150_ENABLE_EVENT_MODE:
		value[0] = CONFIG_SETTINGS_0;
		value[1] = CONFIG_SETTINGS_1 | BIT(0);
		value[2] = OTHER_SETTINGS_0;
		value[3] = OTHER_SETTINGS_1;
		rc = write_reg(dev, IQS9150_CONFIG_SETTINGS_REG, value, sizeof(value));
		if (rc != 0) {
			iqs9150_fail(data, "Event Mode enable", rc);
			return;
		}
		data->state = IQS9150_RUNNING;
		LOG_INF("STEP1B complete: Event Mode active; touch the pad to produce reports");
		return;

	case IQS9150_RUNNING:
		log_report(dev);
		return;

	case IQS9150_ERROR:
		return;
	}
}

static void iqs9150_rdy_callback(const struct device *port, struct gpio_callback *callback,
				 uint32_t pins)
{
	struct iqs9150_data *data = CONTAINER_OF(callback, struct iqs9150_data, rdy_callback);

	ARG_UNUSED(port);
	ARG_UNUSED(pins);
	k_work_schedule(&data->work, K_NO_WAIT);
}

static int iqs9150_init(const struct device *dev)
{
	const struct iqs9150_config *cfg = dev->config;
	struct iqs9150_data *data = dev->data;
	uint8_t product[2];
	int rc;

	if (!i2c_is_ready_dt(&cfg->i2c) || !gpio_is_ready_dt(&cfg->rdy_gpio)) {
		LOG_ERR("I2C or RDY GPIO is not ready");
		return -ENODEV;
	}

	rc = gpio_pin_configure_dt(&cfg->rdy_gpio, GPIO_INPUT);
	if (rc != 0) {
		LOG_ERR("RDY GPIO configuration failed: %d", rc);
		return rc;
	}

	rc = read_reg(dev, IQS9150_PRODUCT_NUMBER_REG, product, sizeof(product));
	if (rc != 0) {
		LOG_ERR("product number read failed: %d", rc);
		return rc;
	}
	if (le16(product) != IQS9150_PRODUCT_NUMBER) {
		LOG_ERR("unexpected product number: 0x%04x", le16(product));
		return -ENODEV;
	}

	data->dev = dev;
	data->state = IQS9150_ACK_RESET;
	k_work_init_delayable(&data->work, iqs9150_work_handler);
	gpio_init_callback(&data->rdy_callback, iqs9150_rdy_callback, BIT(cfg->rdy_gpio.pin));
	rc = gpio_add_callback(cfg->rdy_gpio.port, &data->rdy_callback);
	if (rc != 0) {
		LOG_ERR("RDY callback add failed: %d", rc);
		return rc;
	}
	rc = gpio_pin_interrupt_configure_dt(&cfg->rdy_gpio, GPIO_INT_EDGE_TO_ACTIVE);
	if (rc != 0) {
		LOG_ERR("RDY interrupt configuration failed: %d", rc);
		return rc;
	}

	LOG_INF("IQS9150 product 0x%04x found at I2C 0x%02x", le16(product), cfg->i2c.addr);
	if (gpio_pin_get_dt(&cfg->rdy_gpio) == 0) {
		k_work_schedule(&data->work, K_NO_WAIT);
	}
	return 0;
}

#define IQS9150_DEFINE(inst) \
	static struct iqs9150_data iqs9150_data_##inst; \
	static const struct iqs9150_config iqs9150_config_##inst = { \
		.i2c = I2C_DT_SPEC_INST_GET(inst), \
		.rdy_gpio = GPIO_DT_SPEC_INST_GET(inst, rdy_gpios), \
	}; \
	DEVICE_DT_INST_DEFINE(inst, iqs9150_init, NULL, \
			      &iqs9150_data_##inst, &iqs9150_config_##inst, \
			      POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEVICE, NULL);

DT_INST_FOREACH_STATUS_OKAY(IQS9150_DEFINE)
