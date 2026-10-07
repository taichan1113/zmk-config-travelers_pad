#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#define TRAVELERS_PAD_MAX_CONTACTS 7
#define MAX(a,b) ((a)>(b)?(a):(b))
#define ARG_UNUSED(x) ((void)(x))
#define LOG_INF(...) ((void)0)
#define LOG_WRN(...) ((void)0)
#define TRAVELERS_PAD_RECOGNIZER_MULTIFINGER_GESTURE 1
#define TRAVELERS_PAD_PROPOSAL_CLAIM 1
#define TRAVELERS_PAD_CAP_BEHAVIOR_ACTION 1
enum travelers_pad_owner {TRAVELERS_PAD_OWNER_NONE,TRAVELERS_PAD_OWNER_DRAG,TRAVELERS_PAD_OWNER_SCROLL,TRAVELERS_PAD_OWNER_MULTI_GESTURE};
struct device {const void *config; void *data;};
typedef void (*kscan_callback_t)(const struct device *,uint32_t,uint32_t,bool);
struct kscan_driver_api {int (*config)(const struct device *,kscan_callback_t);};
struct travelers_pad_owner_token {int dummy;};
struct travelers_pad_arbiter_proposal {int frame_seq,recognizer,intent,requested_owner,capabilities,contact_count,evidence;};
static enum travelers_pad_owner owner;
static int actions,last_row,last_column;
static int64_t clock_ms;
static int64_t k_uptime_get(void) {return clock_ms;}
static enum travelers_pad_owner travelers_pad_arbiter_owner_for_frame(uint32_t frame) {(void)frame;return owner;}
static bool travelers_pad_arbiter_submit(const struct travelers_pad_arbiter_proposal *p) {owner=p->requested_owner;return true;}
static int travelers_pad_output_behavior_primary(const struct device *dev,kscan_callback_t cb,uint32_t frame,int rec,enum travelers_pad_owner o,int row,int col,const char *s) {(void)dev;(void)cb;(void)frame;(void)rec;(void)s;if(owner!=o)return -1;actions++;last_row=row;last_column=col;return 0;}
