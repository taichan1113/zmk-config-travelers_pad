static struct device dev;
static uint32_t serial;
static void tick(int64_t t) { now = t; click_work_handler(&click_work.work); }
static int queue(uint16_t button, uint8_t count) {
    struct travelers_pad_terminal_action_grant g = { ++serial };
    return travelers_pad_output_terminal_click(&dev, &g, REC, OWNER, button, count, "test");
}
static void reset(void) {
    for (size_t i = 0; i < ARRAY_SIZE(held_buttons); i++) held_buttons[i] = (struct held_button_record){0};
    queued_clicks = click_head = 0; click_deadline = now = scheduled = 0;
    event_count = failure = 0; authorized = true;
}
int main(void) {
    const int p=CONFIG_TRAVELERS_PAD_CLICK_PRESS_MS, g=CONFIG_TRAVELERS_PAD_CLICK_GAP_MS;
    reset(); assert(queue(1, 2) == 0);
    tick(0); assert(event_count == 1 && events[0].value == 1);
    tick(p-1); assert(event_count == 1 && scheduled == p);
    tick(p); assert(event_count == 2 && events[1].value == 0);
    tick(p+g-1); assert(event_count == 2);
    tick(p+g); assert(event_count == 3 && events[2].value == 1);
    tick(2*p+g); assert(event_count == 4 && events[3].value == 0);
    tick(2*p+2*g); assert(queued_clicks == 0 && !button_busy(1));
    reset(); assert(queue(3, 1) == 0); tick(0);
    /* Actual live drag API takes over; delayed release cannot undo it. */
    assert(travelers_pad_output_primary(&dev, 999, REC, OWNER,
        TRAVELERS_PAD_CAP_BUTTON, INPUT_EV_KEY, 3, 1, true, "drag") == 0);
    tick(p); tick(p+g); assert(event_count == 3 && button_busy(3));
    assert(held_button_exists(&dev, 999, REC, OWNER, 3));
    reset(); assert(queue(3, 1) == 0); tick(0); failure = 1;
    assert(travelers_pad_output_primary(&dev, 999, REC, OWNER,
        TRAVELERS_PAD_CAP_BUTTON, INPUT_EV_KEY, 3, 1, true, "drag") == -EAGAIN);
    assert(!held_button_exists(&dev, 999, REC, OWNER, 3));
    failure = 0; tick(p); assert(!button_busy(3));
    reset(); assert(queue(1, 2) == 0); cancel_clicks(0, true); tick(0);
    assert(event_count == 0 && queued_clicks == 0);
    reset(); authorized = false; assert(queue(1, 1) == -EACCES); assert(queued_clicks == 0);
    reset(); assert(queue(1, 0) == -EINVAL); assert(queue(1, 3) == -EINVAL);
    for (int i = 0; i < CLICK_QUEUE_SIZE; i++) assert(queue(1, 1) == 0);
    assert(queue(1, 1) == -ENOSPC);
    reset(); assert(queue(1, 1) == 0); failure = 1; tick(0);
    assert(!button_busy(1) && event_count == 0);
    failure = 0; tick(p); failure = 1; tick(2*p);
    assert(button_busy(1) && event_count == 1);
    failure = 0; tick(3*p); assert(!button_busy(1) && event_count == 2);
    reset(); assert(remember_held_button(&dev, 999, REC, OWNER, 1));
    assert(queue(1, 2) == 0); tick(0); assert(event_count == 0 && button_busy(1));
    reset(); assert(queue(1, 1) == 0); assert(queue(1, 1) == 0);
    tick(0); tick(p); tick(p+g); tick(2*p+g); tick(2*p+2*g);
    assert(event_count == 4 && events[2].t - events[1].t >= g);
    assert(events[1].t - events[0].t >= p && events[3].t - events[2].t >= p);
    puts("PASS: terminal click pacing, ordering, authorization, cancel, drag isolation, backpressure");
    return 0;
}
