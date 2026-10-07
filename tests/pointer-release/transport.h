static void reset_scroll_state(struct pointer_data *data) {
    data->scroll_axis = SCROLL_AXIS_UNDECIDED;
    data->scroll_probe_x = data->scroll_probe_y = 0;
}
static void report_motion(const struct device *dev, const struct pointer_data *data,
                          int16_t dx, int16_t dy) {
    (void)dev; (void)data;
    motion_count++; last_dx = dx; last_dy = dy;
}
static void process_scroll_delta(const struct device *dev, const struct pointer_config *cfg,
                                 int16_t dx, int16_t dy) {
    (void)dev; (void)cfg;
    scroll_count++; last_dx = dx; last_dy = dy;
}
