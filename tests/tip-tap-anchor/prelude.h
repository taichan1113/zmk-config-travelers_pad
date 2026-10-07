#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#ifndef CONFIG_TRAVELERS_PAD_TIP_TAP_DIAGNOSTICS
#define CONFIG_TRAVELERS_PAD_TIP_TAP_DIAGNOSTICS 1
#endif
#define TRAVELERS_PAD_MAX_CONTACTS 7
#define ARG_UNUSED(x) ((void)(x))
static int trace_logs, candidate_logs, unarmed_logs, interrupted_logs;
static void capture_log(const char *format, ...) {
    if (strstr(format,"Tip-Tap trace")) trace_logs++;
    if (strstr(format,"Tip-Tap candidate")) candidate_logs++;
    if (strstr(format,"Tip-Tap unarmed")) unarmed_logs++;
    if (strstr(format,"Tip-Tap interrupted")) interrupted_logs++;
}
#define LOG_INF(...) capture_log(__VA_ARGS__)
#define LOG_WRN(...) ((void)0)
#define TRAVELERS_PAD_RECOGNIZER_TIP_TAP 1
#define TRAVELERS_PAD_PROPOSAL_CLAIM 1
#define TRAVELERS_PAD_CAP_BEHAVIOR_ACTION 1
enum travelers_pad_owner { TRAVELERS_PAD_OWNER_NONE, TRAVELERS_PAD_OWNER_TIP_TAP,
    TRAVELERS_PAD_OWNER_SCROLL };
struct device { const void *config; void *data; };
typedef void (*kscan_callback_t)(const struct device *,uint32_t,uint32_t,bool);
struct travelers_pad_owner_token { int dummy; };
struct travelers_pad_arbiter_proposal { int frame_seq,recognizer,intent,
    requested_owner,capabilities,contact_count,evidence; };
static enum travelers_pad_owner owner;
static int proposals, actions, column;
static int64_t clock_ms;
static int64_t k_uptime_get(void) { return clock_ms; }
static enum travelers_pad_owner travelers_pad_arbiter_owner_for_frame(uint32_t f) {
    (void)f; return owner;
}
static bool travelers_pad_arbiter_submit(const struct travelers_pad_arbiter_proposal *p) {
    proposals++; owner=p->requested_owner; return true;
}
static int travelers_pad_output_behavior_primary(const struct device *d, kscan_callback_t cb,
    uint32_t f,int rec,enum travelers_pad_owner o,int row,int col,const char *s) {
    (void)d;(void)cb;(void)f;(void)rec;(void)row;(void)s;
    if(owner!=o) return -1;
    actions++;column=col;return 0;
}
