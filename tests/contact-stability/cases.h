static void sensor_coordinate(uint8_t *report, unsigned int slot, uint16_t x, uint16_t y)
{
    unsigned int offset = IQS9150_REPORT_FINGER_OFFSET(slot);
    report[offset] = x; report[offset + 1] = x >> 8;
    report[offset + 2] = y; report[offset + 3] = y >> 8;
}
static void event(const struct device *dev, uint8_t type, uint16_t code, int value, bool sync)
{
    struct input_event e = {type, code, value, sync}; session_input(dev, &e);
}
static int last_value(uint8_t type, uint16_t code)
{
    for (int i = output_count - 1; i >= 0; i--) if (output[i].type == type && output[i].code == code) return output[i].value;
    assert(false); return 0;
}
static void frame(const struct device *dev, uint8_t mask, int64_t ms, bool reset)
{
    clock_ms = ms; output_count = 0;
    for (unsigned int slot = 0; slot < 7; slot++) {
        event(dev, INPUT_EV_ABS, INPUT_ABS_MT_SLOT, slot, false);
        event(dev, INPUT_EV_ABS, TRAVELERS_PAD_INPUT_ABS_MT_TRACKING_ID, mask & BIT(slot) ? (int)slot : -1, false);
        if (mask & BIT(slot)) {
            event(dev, INPUT_EV_ABS, TRAVELERS_PAD_INPUT_ABS_MT_POSITION_X, 100 + slot * 200, false);
            event(dev, INPUT_EV_ABS, TRAVELERS_PAD_INPUT_ABS_MT_POSITION_Y, 300 + slot * 100, false);
        }
    }
    if (reset) event(dev, INPUT_EV_ABS, INPUT_ABS_RZ, 1, false);
    event(dev, INPUT_EV_ABS, INPUT_ABS_Z, travelers_pad_contact_count(mask), false);
    event(dev, INPUT_EV_KEY, INPUT_BTN_TOUCH, mask != 0, true);
    assert(output[output_count - 1].sync);
}
int main(void)
{
    uint8_t report[72]; struct iqs9150_contact contacts[7];
    memset(report, 255, sizeof(report));
    sensor_coordinate(report, 6, 321, 654);
    assert(decode_contact_slots(report, 1 | BIT(14), contacts) == 1);
    assert(contacts[6].valid && !contacts[0].valid);
    assert(contact_representative(contacts, 0, false) == 6);
    assert(decode_contact_slots(report, BIT(14), contacts) == 0);
    assert(decode_contact_slots(report, 1, contacts) == 0); // confidence rejected
    sensor_coordinate(report, 1, 700, 900);
    assert(decode_contact_slots(report, 2 | BIT(9) | BIT(14), contacts) == 2);
    assert(contact_representative(contacts, 6, true) == 6); // sticky identity
    sensor_coordinate(report, 6, UINT16_MAX, UINT16_MAX);
    assert(decode_contact_slots(report, 1 | BIT(9) | BIT(14), contacts) == 1);
    assert(contact_representative(contacts, 6, true) == 1);

    struct session_data data = {0}; struct session_config cfg = {35};
    struct device dev = {&data, &cfg};
    frame(&dev, 1, 0, false); assert(resolved.contact_count == 1);
    uint32_t session = resolved.session_id;
    frame(&dev, 3, 100, false); assert(resolved.contact_count == 1);
    frame(&dev, 3, 125, false); assert(resolved.contact_count == 1);
    frame(&dev, 1, 126, false); assert(resolved.contact_count == 1);
    assert(resolved.session_id == session && resolved.peak_contact_count == 1);
    assert(data.emitted_mask == 1); // no ghost slot leaked
    for (int burst = 0; burst < 100; burst++) {
        int64_t ms = 200 + burst * 100;
        frame(&dev, 3, ms, false); frame(&dev, 3, ms + 26, false);
        frame(&dev, 1, ms + 27, false);
        assert(data.emitted_mask == 1 && resolved.peak_contact_count == 1);
    }
    frame(&dev, 3, 10300, false); frame(&dev, 3, 10335, false);
    assert(resolved.contact_count == 2 && data.emitted_mask == 3);
    frame(&dev, 2, 10336, false); // surviving physical slot 1, no session reset
    assert(resolved.contact_count == 1 && resolved.session_id == session);
    assert(last_value(INPUT_EV_ABS, INPUT_ABS_X) == 300);
    assert(last_value(INPUT_EV_MSC, TRAVELERS_PAD_INPUT_MSC_CONTACT_REFERENCE_CHANGED) == 1);
    frame(&dev, 2, 10337, false);
    assert(last_value(INPUT_EV_MSC, TRAVELERS_PAD_INPUT_MSC_CONTACT_REFERENCE_CHANGED) == 0);
    frame(&dev, 0, 10400, false); assert(resolved.contact_count == 0 && !resolved.cancelled);
    frame(&dev, 3, 10401, false); assert(resolved.contact_count == 2); // initial 2F immediate
    frame(&dev, 7, 10402, false); assert(resolved.contact_count == 3); // 3F immediate
    frame(&dev, 0, 10403, true); assert(resolved.cancelled && cancels == 2);
    frame(&dev, 1, 10404, false); assert(!resolved.cancelled && resolved.contact_count == 1);
    cfg.extra_contact_confirm_ms = 0;
    frame(&dev, 3, 10405, false); assert(resolved.contact_count == 2); // configurable bypass
    puts("PASS: production sparse-slot decoder and session stabilization: 100 bursts, 35ms confirmation, immediate initial multi-touch/release, reference changes, reset, disabled filter");
    return 0;
}
