#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#ifndef CONFIG_TRAVELERS_PAD_CLICK_PRESS_MS
#define CONFIG_TRAVELERS_PAD_CLICK_PRESS_MS 5
#endif
#define CONFIG_TRAVELERS_PAD_CLICK_GAP_MS 5
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define ARG_UNUSED(x) ((void)(x))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define K_NO_WAIT 0
#define K_MSEC(x) (x)
#define INPUT_EV_KEY 1
#define TRAVELERS_PAD_CAP_BUTTON 1
#define LOG_ERR(...) ((void)0)
#define LOG_WRN(...) ((void)0)
#define LOG_INF(...) do { if (0) printf(__VA_ARGS__); } while (0)
struct device { int unused; };
enum travelers_pad_recognizer_id { REC };
enum travelers_pad_owner { OWNER };
struct travelers_pad_terminal_action_grant { uint32_t grant_generation; };
struct travelers_pad_arbiter_output_guard { int unused; };
struct k_work { int unused; };
struct k_work_delayable { struct k_work work; };
struct k_spinlock { int unused; };
typedef int k_spinlock_key_t;
static int64_t now, scheduled;
static bool authorized = true;
static int failure, event_count;
static struct { int64_t t; int value; uint16_t button; } events[128];
#define K_WORK_DELAYABLE_DEFINE(name, handler) struct k_work_delayable name
static int64_t k_uptime_get(void) { return now; }
static int k_work_reschedule(struct k_work_delayable *w, int64_t delay) {
    (void)w; scheduled = now + delay; return 0;
}
static k_spinlock_key_t k_spin_lock(struct k_spinlock *l) { (void)l; return 0; }
static void k_spin_unlock(struct k_spinlock *l, k_spinlock_key_t k) { (void)l; (void)k; }
static int input_report(const struct device *d, int type, uint16_t button,
                        int value, bool sync, int wait) {
    (void)d; (void)type; (void)sync; (void)wait;
    if (failure) return -ENOSPC;
    assert(event_count < 128);
    events[event_count].t = now; events[event_count].value = value;
    events[event_count++].button = button; return 0;
}
static bool travelers_pad_arbiter_begin_terminal_action_output(
    struct travelers_pad_terminal_action_grant *g, enum travelers_pad_recognizer_id r,
    enum travelers_pad_owner o, int cap, const char *p,
    struct travelers_pad_arbiter_output_guard *guard) {
    (void)g; (void)r; (void)o; (void)cap; (void)p; (void)guard; return authorized;
}
static void travelers_pad_arbiter_end_output(struct travelers_pad_arbiter_output_guard *g) { (void)g; }
static void retry_pending_button_ups(const char *p) { (void)p; }
static bool travelers_pad_arbiter_begin_primary_output(uint32_t frame,
    enum travelers_pad_recognizer_id r, enum travelers_pad_owner o, uint32_t cap,
    const char *p, struct travelers_pad_arbiter_output_guard *g) {
    (void)frame; (void)r; (void)o; (void)cap; (void)p; (void)g; return authorized;
}
static int emit_guarded(const struct device *d, uint8_t type, uint16_t code,
    int32_t value, bool sync, const char *p,
    struct travelers_pad_arbiter_output_guard *g, bool observe) {
    (void)p; (void)g; (void)observe; return input_report(d, type, code, value, sync, 0);
}
