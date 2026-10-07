static struct pointer_data data;
static struct pointer_config cfg;
static struct device dev = { &cfg, &data };
static void event(uint8_t type, uint16_t code, int32_t value, bool sync) {
    struct input_event e = {type, code, value, sync}; pointer_handle_input(&dev, &e);
}
static void frame(int32_t x, int32_t y, int fingers) {
    event(INPUT_EV_MSC, TRAVELERS_PAD_INPUT_MSC_ROUTED_FRAME_SEQ, 1, false);
    event(INPUT_EV_ABS, INPUT_ABS_X, x, false);
    event(INPUT_EV_ABS, INPUT_ABS_Y, y, false);
    event(INPUT_EV_ABS, INPUT_ABS_Z, fingers, false);
    event(INPUT_EV_KEY, 0, fingers != 0, true);
}
static void reset(void) { data = (struct pointer_data){0}; motion_count = scroll_count = 0; }
int main(void) {
    /* Reproduce the logged signed sentinel jumps without allowing any output. */
    for (int i = 0; i < 100; i++) {
        reset(); frame(1333, 368, 1); assert(motion_count == 0);
        frame(1334, 370, 1); assert(last_dx == 1 && last_dy == 2);
        int saved = motion_count;
        frame(UINT16_MAX, UINT16_MAX, 1); assert(motion_count == saved && !data.have_position);
        frame(1622, 429, 1); assert(motion_count == saved && data.have_position);
        frame(UINT16_MAX, 429, 1); assert(motion_count == saved);
        frame(UINT16_MAX, UINT16_MAX, 0); assert(!data.have_position && !data.scroll_session);
        frame(50, 60, 1); assert(motion_count == saved);
        frame(53, 64, 1); assert(last_dx == 3 && last_dy == 4);
    }
    reset(); frame(100, 100, 1);
    frame(-1, 100, 1); assert(motion_count == 0);
    frame(100, 65536, 1); assert(motion_count == 0);
    frame(100, 100, 1); assert(motion_count == 0);
    /* Unexpected sync before contact count must never reuse prior fingers. */
    event(INPUT_EV_MSC, TRAVELERS_PAD_INPUT_MSC_ROUTED_FRAME_SEQ, 2, false);
    event(INPUT_EV_ABS, INPUT_ABS_X, UINT16_MAX, false);
    event(INPUT_EV_ABS, INPUT_ABS_Y, UINT16_MAX, true);
    assert(motion_count == 0 && !data.have_position);
    frame(500, 600, 1); assert(motion_count == 0);
    frame(501, 601, 1); assert(last_dx == 1 && last_dy == 1);
    reset(); frame(100, 100, 1); frame(200, 200, 2);
    assert(motion_count == 0 && scroll_count == 0 && data.scroll_session);
    frame(202, 203, 2); assert(scroll_count == 1);
    frame(UINT16_MAX, UINT16_MAX, 2); assert(scroll_count == 1 && data.scroll_session);
    frame(300, 300, 2); assert(scroll_count == 1 && motion_count == 0);
    frame(301, 302, 2); assert(scroll_count == 2 && last_dx == 1 && last_dy == 2);
    frame(301, 302, 1); assert(scroll_count == 2 && motion_count == 0);
    frame(UINT16_MAX, UINT16_MAX, 0); assert(!data.scroll_session);
    frame(5, 5, 1); assert(motion_count == 0);
    event(INPUT_EV_ABS, INPUT_ABS_RZ, 1, false);
    event(INPUT_EV_KEY, 0, 0, true); assert(!data.have_position && data.previous_fingers == 0);
    frame(200, 200, 1); assert(motion_count == 0);
    puts("PASS: sentinel/release/partial frames cannot jump; 100 cycles; pointer/scroll rebasing and reset");
    return 0;
}
