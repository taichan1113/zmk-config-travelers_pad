#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <travelers_pad/inertia_math.h>

#define CONTAINER_OF(p, type, member) ((type *)((char *)(p) - offsetof(type, member)))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CLAMP(v, lo, hi) ((v) < (lo) ? (lo) : (v) > (hi) ? (hi) : (v))
#define K_MSEC(ms) (ms)
#define LOG_INF(...) ((void)0)
#define LOG_DBG(...) ((void)0)
#define LOG_ERR(...) ((void)0)
#define INPUT_EV_ABS 3
#define INPUT_EV_REL 2
#define INPUT_ABS_Z 2
#define INPUT_REL_WHEEL 8
#define INPUT_REL_HWHEEL 6
#define TRAVELERS_PAD_OWNER_SCROLL 2
#define TRAVELERS_PAD_CAP_SCROLL_DELTA 2

struct device { const void *config; void *data; };
struct input_event { uint8_t type; uint16_t code; int32_t value; bool sync; };
struct k_work { bool pending; };
struct k_work_delayable { struct k_work work; };
struct k_work_sync { int unused; };
struct travelers_pad_wheel_observation { int unused; };
struct travelers_pad_owner_token {
    bool valid;
    uint32_t session_id;
    uint32_t owner_generation;
    int owner;
};
struct travelers_pad_session_snapshot {
    uint8_t contact_count;
    bool cancelled;
    uint32_t session_id;
};

static int64_t now;
static int output_result, output_calls;
static bool lease_available = true;
static int64_t k_uptime_get(void) { return now; }
static int k_work_schedule(struct k_work_delayable *w, int delay)
{
    (void)delay;
    w->work.pending = true;
    return 1;
}
static int k_work_cancel_delayable_sync(struct k_work_delayable *w, struct k_work_sync *sync)
{
    (void)sync;
    w->work.pending = false;
    return 0;
}
static bool travelers_pad_arbiter_acquire_post_session_lease(
    uint32_t session_id, int owner, uint32_t caps, struct travelers_pad_owner_token *token)
{
    (void)caps;
    if (!lease_available) return false;
    *token = (struct travelers_pad_owner_token){
        .valid = true, .session_id = session_id, .owner = owner,
    };
    return true;
}
static int travelers_pad_output_post_session(
    const struct device *dev, const struct travelers_pad_owner_token *token,
    int owner, uint32_t caps, uint8_t type, uint16_t code, int32_t value,
    bool sync, const char *producer)
{
    (void)dev; (void)token; (void)owner; (void)caps; (void)type;
    (void)code; (void)value; (void)sync; (void)producer;
    output_calls++;
    return output_result;
}
