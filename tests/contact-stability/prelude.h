#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <travelers_pad/contact_session.h>
#include <travelers_pad/contact_stability.h>
#include <travelers_pad/multitouch.h>
#define BIT(n) (1u << (n))
#define BIT_MASK(n) (BIT(n) - 1)
#define K_NO_WAIT 0
#define K_FOREVER -1
#define K_MSEC(x) (x)
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define CONTAINER_OF(ptr,type,member) ((type *)((char *)(ptr)-offsetof(type,member)))
struct k_work { void (*handler)(struct k_work *); };
struct k_work_delayable { struct k_work work; bool pending; int64_t delay; };
struct k_mutex { int depth; };
static void k_mutex_init(struct k_mutex *m) { m->depth=0; }
static void k_mutex_lock(struct k_mutex *m, int wait) { (void)wait;m->depth++; }
static void k_mutex_unlock(struct k_mutex *m) { assert(m->depth>0);m->depth--; }
static struct k_work_delayable *k_work_delayable_from_work(struct k_work *w) {
    return CONTAINER_OF(w,struct k_work_delayable,work);
}
static void k_work_init_delayable(struct k_work_delayable *w,void (*fn)(struct k_work *)) {
    *w=(struct k_work_delayable){.work={fn}};
}
static void k_work_reschedule(struct k_work_delayable *w,int64_t delay) {w->pending=true;w->delay=delay;}
static void k_work_cancel_delayable(struct k_work_delayable *w) {w->pending=false;}
#define LOG_INF(...) ((void)0)
#define LOG_WRN(...) ((void)0)
#define LOG_ERR(...) ((void)0)
#define INPUT_EV_ABS 3
#define INPUT_EV_KEY 1
#define INPUT_EV_MSC 4
#define INPUT_ABS_X 0
#define INPUT_ABS_Y 1
#define INPUT_ABS_Z 2
#define INPUT_ABS_RX 3
#define INPUT_ABS_RY 4
#define INPUT_ABS_RZ 5
#define INPUT_ABS_RUDDER 7
#define INPUT_ABS_MT_SLOT 0x2f
#define INPUT_BTN_TOUCH 0x14a
struct device { void *data; const void *config; };
struct input_event { uint8_t type; uint16_t code; int32_t value; bool sync; };
struct iqs9150_contact { uint16_t x, y; bool valid; };
static int64_t clock_ms;
static int64_t k_uptime_get(void) { return clock_ms; }
static uint16_t le16(const uint8_t *p) { return p[0] | ((uint16_t)p[1] << 8); }
static struct travelers_pad_session_snapshot resolved;
static unsigned int cancels, output_count;
static bool reject_wake;
static struct input_event output[128];
static void travelers_pad_arbiter_begin_frame(const struct travelers_pad_session_snapshot *s) { resolved = *s; }
static void travelers_pad_arbiter_cancel_all(const char *why) { (void)why; cancels++; }
static void travelers_pad_output_cancel_all(const char *why) { (void)why; cancels++; }
static int input_report(const struct device *dev, uint8_t type, uint16_t code, int32_t value, bool sync, int wait)
{
    (void)dev; (void)wait;
    if (type==INPUT_EV_MSC && code==0x7f01 && reject_wake) return -1;
    assert(output_count < 128);
    output[output_count++] = (struct input_event){type, code, value, sync};
    return 0;
}
static int input_report_abs(const struct device *dev, uint16_t code, int32_t value, bool sync, int wait) { return input_report(dev, INPUT_EV_ABS, code, value, sync, wait); }
static int input_report_key(const struct device *dev, uint16_t code, int32_t value, bool sync, int wait) { return input_report(dev, INPUT_EV_KEY, code, value, sync, wait); }
