#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#define TRAVELERS_PAD_MAX_CONTACTS 7
#define BIT(n) (1u << (n))
#define IQS9150_TP_FINGER_COUNT_MASK 15
#define IQS9150_TP_FINGER_CONFIDENCE(n) BIT(8+(n))
struct iqs9150_contact { uint16_t x,y; bool valid; };
struct iqs9150_data {
    bool contact_diagnostic_seen;
    uint8_t contact_diagnostic_raw_count, contact_diagnostic_mask;
    uint8_t contact_diagnostic_confidence, contact_diagnostic_coordinates;
    int64_t contact_diagnostic_ms;
    uint32_t reports_read;
};
static int64_t now_ms;
static int logs;
static char last_log[512];
static int64_t k_uptime_get(void) { return now_ms; }
static void capture_log(const char *fmt, ...) {
    va_list args; va_start(args,fmt);
    vsnprintf(last_log,sizeof(last_log),fmt,args); va_end(args); logs++;
}
#define LOG_INF(...) capture_log(__VA_ARGS__)
