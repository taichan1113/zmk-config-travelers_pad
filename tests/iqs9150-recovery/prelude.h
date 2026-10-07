#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#if __has_include(<iqs9150/diagnostics.h>)
#include <iqs9150/diagnostics.h>
struct k_spinlock {int unused;};
typedef int k_spinlock_key_t;
static int k_spin_lock(struct k_spinlock *lock) {(void)lock;return 0;}
static void k_spin_unlock(struct k_spinlock *lock,int key) {(void)lock;(void)key;}
#endif
#define BIT(n) (1U<<(n))
#define BIT_MASK(n) (BIT(n)-1)
#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))
#define MIN(a,b) ((a)<(b)?(a):(b))
#define CONTAINER_OF(p,t,m) ((t*)((char*)(p)-offsetof(t,m)))
#define K_NO_WAIT 0
#define K_MSEC(x) (x)
#define LOG_INF(...) ((void)0)
#define LOG_WRN(...) ((void)0)
#define LOG_ERR(...) ((void)0)
#define TRAVELERS_PAD_MAX_CONTACTS 7
#define CONFIG_SETTINGS_0 0x1e
#define CONFIG_SETTINGS_1 0x46
#define OTHER_SETTINGS_0 0xa8
#define OTHER_SETTINGS_1 0x80
typedef int atomic_t;
struct device {const void *config;void *data;};
struct i2c_dt_spec {int dummy;};
struct gpio_dt_spec {int dummy;};
struct gpio_callback {int dummy;};
struct k_work {int pending;};
struct k_work_delayable {struct k_work work;};
static int64_t now;
static int active=1;
static uint16_t info_flags, config_word=0x461e;
static int reset_writes, reads, reports, terminations;
static int comms_requests;
static bool diagnostic_timeout_mismatch;
static bool power_setup_empty;
static uint16_t power_setup_other;
static uint8_t last_low,last_high;
static int64_t k_uptime_get(void) {return now;}
static int gpio_pin_get_dt(const struct gpio_dt_spec *p) {(void)p;return active;}
static struct k_work_delayable *k_work_delayable_from_work(struct k_work *w) {return (void*)w;}
static int k_work_schedule(struct k_work_delayable *w,int delay) {(void)delay;w->work.pending=1;return 1;}
static int k_work_reschedule(struct k_work_delayable *w,int delay) {return k_work_schedule(w,delay);}
static bool k_work_delayable_is_pending(struct k_work_delayable *w) {return w->work.pending;}
static int atomic_get(atomic_t *p) {return *p;}
