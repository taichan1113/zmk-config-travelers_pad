/*
 * IQS9150 EV02 Step 1A diagnostic driver.
 *
 * This file deliberately contains no configuration writes: its job is to
 * verify the existing EV02 configuration without disturbing it.
 */

#define DT_DRV_COMPAT azoteq_iqs9150

#include <errno.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

LOG_MODULE_REGISTER(iqs9150_step1a, CONFIG_IQS9150_STEP1A_LOG_LEVEL);

#define IQS9150_PRODUCT_INFO_REG 0x1000
#define IQS9150_PRODUCT_INFO_LEN 20
#define IQS9150_PRODUCT_NUMBER 0x076A

#define IQS9150_REPORT_REG 0x1014
/* Relative/gesture data, flags, then the first touch's absolute data. */
#define IQS9150_REPORT_LEN 24

struct iqs9150_config {
    struct i2c_dt_spec i2c;
    struct gpio_dt_spec rdy_gpio;
};

struct iqs9150_data {
    const struct device *dev;
    struct gpio_callback rdy_callback;
    struct k_work report_work;
};

static uint16_t get_le16(const uint8_t *value)
{
    return (uint16_t)value[0] | ((uint16_t)value[1] << 8);
}

static int iqs9150_read(const struct i2c_dt_spec *i2c, uint16_t reg,
                        uint8_t *buffer, size_t length)
{
    /* IQS9150 register addresses are 16-bit, byte-addressable little-endian. */
    uint8_t address[] = { reg & 0xff, reg >> 8 };

    return i2c_write_read_dt(i2c, address, sizeof(address), buffer, length);
}

static void iqs9150_log_report(struct iqs9150_data *data)
{
    const struct iqs9150_config *config = data->dev->config;
    uint8_t report[IQS9150_REPORT_LEN];
    int ret = iqs9150_read(&config->i2c, IQS9150_REPORT_REG, report,
                           sizeof(report));

    if (ret < 0) {
        LOG_ERR("report read failed: %d", ret);
        return;
    }

    uint16_t info_flags = get_le16(&report[12]);
    uint16_t trackpad_flags = get_le16(&report[14]);
    uint8_t fingers = trackpad_flags & 0x0f;

    LOG_INF("status=0x%04x tp=0x%04x fingers=%u rel=(%d,%d) gest=(%d,%d) "
            "abs1=(%u,%u) strength=%u area=%u", info_flags, trackpad_flags,
            fingers, (int16_t)get_le16(&report[0]), (int16_t)get_le16(&report[2]),
            (int16_t)get_le16(&report[4]), (int16_t)get_le16(&report[6]),
            get_le16(&report[16]), get_le16(&report[18]),
            get_le16(&report[20]), get_le16(&report[22]));
}

static void iqs9150_report_work_handler(struct k_work *work)
{
    struct iqs9150_data *data = CONTAINER_OF(work, struct iqs9150_data,
                                              report_work);

    iqs9150_log_report(data);
}

static void iqs9150_rdy_isr(const struct device *port,
                             struct gpio_callback *callback, uint32_t pins)
{
    ARG_UNUSED(port);
    ARG_UNUSED(pins);

    struct iqs9150_data *data = CONTAINER_OF(callback, struct iqs9150_data,
                                              rdy_callback);
    k_work_submit(&data->report_work);
}

static int iqs9150_init(const struct device *dev)
{
    const struct iqs9150_config *config = dev->config;
    struct iqs9150_data *data = dev->data;
    uint8_t product[IQS9150_PRODUCT_INFO_LEN];
    int ret;

    if (!i2c_is_ready_dt(&config->i2c)) {
        LOG_ERR("I2C controller is not ready");
        return -ENODEV;
    }
    if (!gpio_is_ready_dt(&config->rdy_gpio)) {
        LOG_ERR("RDY GPIO controller is not ready");
        return -ENODEV;
    }

    /* A forced read is permitted by the datasheet; it may clock-stretch until
     * a communication window is available. No configuration is modified. */
    ret = iqs9150_read(&config->i2c, IQS9150_PRODUCT_INFO_REG, product,
                       sizeof(product));
    if (ret < 0) {
        LOG_ERR("product information read failed: %d", ret);
        return ret;
    }
    if (get_le16(&product[0]) != IQS9150_PRODUCT_NUMBER) {
        LOG_ERR("unexpected product number: 0x%04x (expected 0x%04x)",
                get_le16(&product[0]), IQS9150_PRODUCT_NUMBER);
        return -ENODEV;
    }

    LOG_INF("IQS9150 detected: product=0x%04x version=%u.%u library=0x%04x "
            "v%u.%u", get_le16(&product[0]), get_le16(&product[2]),
            get_le16(&product[4]), get_le16(&product[10]),
            get_le16(&product[12]), get_le16(&product[14]));

    data->dev = dev;
    k_work_init(&data->report_work, iqs9150_report_work_handler);
    ret = gpio_pin_configure_dt(&config->rdy_gpio, GPIO_INPUT);
    if (ret < 0) {
        LOG_ERR("RDY GPIO configuration failed: %d", ret);
        return ret;
    }
    gpio_init_callback(&data->rdy_callback, iqs9150_rdy_isr,
                       BIT(config->rdy_gpio.pin));
    ret = gpio_add_callback(config->rdy_gpio.port, &data->rdy_callback);
    if (ret < 0) {
        LOG_ERR("RDY callback registration failed: %d", ret);
        return ret;
    }
    ret = gpio_pin_interrupt_configure_dt(&config->rdy_gpio,
                                          GPIO_INT_EDGE_FALLING);
    if (ret < 0) {
        LOG_ERR("RDY interrupt configuration failed: %d", ret);
        return ret;
    }

    LOG_INF("IQS9150 Step 1A ready; waiting for RDY");
    if (gpio_pin_get_dt(&config->rdy_gpio) == 0) {
        k_work_submit(&data->report_work);
    }
    return 0;
}

#define IQS9150_DEFINE(inst)                                                   \
    static struct iqs9150_data iqs9150_data_##inst;                            \
    static const struct iqs9150_config iqs9150_config_##inst = {               \
        .i2c = I2C_DT_SPEC_INST_GET(inst),                                      \
        .rdy_gpio = GPIO_DT_SPEC_INST_GET(inst, rdy_gpios),                     \
    };                                                                          \
    DEVICE_DT_INST_DEFINE(inst, iqs9150_init, NULL, &iqs9150_data_##inst,       \
                          &iqs9150_config_##inst, POST_KERNEL,                 \
                          CONFIG_KERNEL_INIT_PRIORITY_DEVICE, NULL);

DT_INST_FOREACH_STATUS_OKAY(IQS9150_DEFINE)
