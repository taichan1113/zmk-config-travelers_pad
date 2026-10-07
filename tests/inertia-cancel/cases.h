static struct scroll_inertia_config cfg = {
    .start_delay_ms = 10, .repeat_ms = 10, .decay_numerator = 9,
    .decay_denominator = 10, .stop_delta = 2, .max_delta = 300,
    .velocity_window_ms = 80, .release_timeout_ms = 120,
    .max_duration_ms = 3000, .stop_speed = 20,
};
static struct scroll_inertia_data data;
static struct device dev = { .config = &cfg, .data = &data };

static void setup(bool time_based, bool horizontal)
{
    cfg.half_life_ms = time_based ? 250 : 0;
    data = (struct scroll_inertia_data){
        .dev = &dev, .post_session_lease = {.valid = true},
        .horizontal_velocity = horizontal ? 30 : 0,
        .vertical_velocity = horizontal ? 0 : 30,
        .horizontal_velocity_fp = horizontal ? TRAVELERS_PAD_INERTIA_ONE : 0,
        .vertical_velocity_fp = horizontal ? 0 : TRAVELERS_PAD_INERTIA_ONE,
        .release_ms = 100, .last_tick_ms = 100,
    };
    now = 110;
    output_calls = 0;
    output_result = 0;
    lease_available = true;
}

static void run_tick(void)
{
    data.work.work.pending = false;
    inertia_work_handler(&data.work.work);
}

static void next_scroll(uint16_t axis)
{
    struct input_event contact = {.type = INPUT_EV_ABS, .code = INPUT_ABS_Z, .value = 2};
    inertia_handle_contact(&dev, &contact);
    assert(!data.emitting);
    now += 12;
    struct input_event wheel = {.type = INPUT_EV_REL, .code = axis, .value = 55};
    inertia_handle_wheel(&dev, &wheel);
    assert(axis == INPUT_REL_WHEEL ? data.vertical_velocity == 55 : data.horizontal_velocity == 55);
    if (cfg.half_life_ms) assert(data.have_wheel);

    now += 12;
    contact.value = 0;
    inertia_handle_contact(&dev, &contact);
    struct travelers_pad_session_snapshot snapshot = {.contact_count = 0, .session_id = 42};
    struct travelers_pad_owner_token token = {.valid = true, .owner = TRAVELERS_PAD_OWNER_SCROLL};
    inertia_deliver_resolved_frame(&data, &snapshot, &token);
    assert(data.post_session_pending && data.work.work.pending);
    output_result = 0;
    int before = output_calls;
    now += cfg.start_delay_ms;
    run_tick();
    assert(output_calls > before && !data.emitting && data.work.work.pending);
}

int main(void)
{
    for (int mode = 0; mode < 2; mode++) {
        for (int horizontal = 0; horizontal < 2; horizontal++) {
            for (int repeat = 0; repeat < 64; repeat++) {
                setup(mode, horizontal);
                output_result = -EACCES;
                run_tick();
                assert(output_calls == 1);
                assert(!data.emitting && !data.have_wheel && !data.post_session_lease.valid);
                assert(!data.work.work.pending);
                next_scroll(horizontal ? INPUT_REL_HWHEEL : INPUT_REL_WHEEL);
                stop_inertia(&dev);
                assert(!data.emitting && !data.work.work.pending);
            }
            setup(mode, horizontal);
            output_result = -EIO;
            run_tick();
            assert(output_calls == 1 && !data.emitting);
            setup(mode, horizontal);
            data.post_session_lease.valid = false;
            data.post_session_pending = true;
            lease_available = false;
            run_tick();
            assert(!data.emitting && output_calls == 0 && !data.work.work.pending);
        }
    }
    setup(true, false);
    data.emitting = true;
    clear_inertia_state(&data);
    assert(!data.emitting);
    puts("PASS: production inertia cancellation, horizontal/vertical and legacy/time-based, 256 cancel/restart cycles, output error and missing lease");
    return 0;
}
