#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#define CONFIG_TRAVELERS_PAD_GESTURE_PRESS_MS 50
#define TRAVELERS_PAD_CAP_BEHAVIOR_ACTION 8
#define K_NO_WAIT 0
#define K_MSEC(x) (x)
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define ARG_UNUSED(x) ((void)(x))
#define LOG_INF(...) do { if(0) printf(__VA_ARGS__); } while(0)
#define LOG_WRN(...) ((void)0)
struct device { int dummy; };
typedef void (*kscan_callback_t)(const struct device *, uint32_t, uint32_t, bool);
enum travelers_pad_recognizer_id { REC, TRAVELERS_PAD_RECOGNIZER_EDGE_DIAL };
enum travelers_pad_owner { OWNER };
struct k_work { int dummy; };
struct k_work_delayable { struct k_work work; };
struct k_spinlock { int dummy; };
typedef int k_spinlock_key_t;
#define K_WORK_DELAYABLE_DEFINE(name, handler) struct k_work_delayable name
static int64_t now, scheduled;
static bool authorized=true;
static int64_t k_uptime_get(void) { return now; }
static k_spinlock_key_t k_spin_lock(struct k_spinlock *l) { (void)l; return 0; }
static void k_spin_unlock(struct k_spinlock *l,k_spinlock_key_t k) { (void)l;(void)k; }
static void k_work_reschedule(struct k_work_delayable *w,int64_t delay) { (void)w;scheduled=now+delay; }
static void retry_pending_button_ups(const char *p) { (void)p; }
static bool travelers_pad_arbiter_authorize_output(uint32_t f,
    enum travelers_pad_recognizer_id r,enum travelers_pad_owner o,int cap,const char *p) {
    (void)f;(void)r;(void)o;(void)cap;(void)p;return authorized;
}
