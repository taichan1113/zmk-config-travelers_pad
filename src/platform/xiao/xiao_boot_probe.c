/*
 * XIAO UF2-to-application USB startup diagnostic.
 *
 * Red LED behaviour after copying the diagnostic UF2:
 *   - flashing: the ZMK application is running, but USB is not configured.
 *   - off: USB reached CONFIGURED.
 * No red LED activity means the application did not reach this code.
 */

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/usb/usb_device.h>

#include <zmk/usb.h>

LOG_MODULE_REGISTER(xiao_boot_probe, LOG_LEVEL_INF);

#define BOOT_PROBE_PERIOD_MS 250

static const struct gpio_dt_spec probe_led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static struct k_work_delayable probe_work;
static bool probe_led_on;

static void boot_probe_set_led(bool on)
{
	/* The XIAO RGB LEDs are active-low. gpio_pin_set_dt() uses the DTS flags. */
	(void)gpio_pin_set_dt(&probe_led, on ? 1 : 0);
	probe_led_on = on;
}

static void boot_probe_work_handler(struct k_work *work)
{
	ARG_UNUSED(work);

	if (zmk_usb_get_status() == USB_DC_CONFIGURED) {
		boot_probe_set_led(false);
		LOG_INF("USB configured; boot probe complete");
		return;
	}

	/* Keep a visible heartbeat until the host configures the application USB
	 * device. This remains observable even when the CDC port never appears. */
	boot_probe_set_led(!probe_led_on);
	k_work_schedule(&probe_work, K_MSEC(BOOT_PROBE_PERIOD_MS));
}

static int xiao_boot_probe_init(void)
{
	int rc;

	if (!gpio_is_ready_dt(&probe_led)) {
		LOG_ERR("boot probe LED is not ready");
		return -ENODEV;
	}
	rc = gpio_pin_configure_dt(&probe_led, GPIO_OUTPUT_INACTIVE);
	if (rc != 0) {
		LOG_ERR("boot probe LED configuration failed: %d", rc);
		return rc;
	}

	k_work_init_delayable(&probe_work, boot_probe_work_handler);
	LOG_INF("application reached XIAO USB boot probe; status=%d", zmk_usb_get_status());
	k_work_schedule(&probe_work, K_NO_WAIT);
	return 0;
}

SYS_INIT(xiao_boot_probe_init, APPLICATION, 99);
