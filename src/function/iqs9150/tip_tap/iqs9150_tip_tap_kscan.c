/* Two-finger Tip-Tap recognizer / 2本指Tip-Tap認識器. */
#define DT_DRV_COMPAT zmk_travelers_pad_iqs9150_tip_tap_kscan
#include <zephyr/device.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>
#include <zephyr/drivers/kscan.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <travelers_pad/contact_session.h>
#include <travelers_pad/multitouch.h>

LOG_MODULE_REGISTER(travelers_pad_tip_tap, LOG_LEVEL_INF);

struct cfg { uint16_t max_ms, slop, separation; bool invert; };
struct data {
	kscan_callback_t callback; uint16_t x[TRAVELERS_PAD_MAX_CONTACTS], y[TRAVELERS_PAD_MAX_CONTACTS];
	int32_t id[TRAVELERS_PAD_MAX_CONTACTS]; bool valid[TRAVELERS_PAD_MAX_CONTACTS]; uint8_t slot, fingers, previous;
	uint8_t anchor, second; int32_t anchor_id, second_id; uint16_t ax, ay, sx, sy; int64_t second_start;
	bool candidate, fired, reset;
};
static uint16_t absi(int value) { return value < 0 ? -value : value; }
static void reset(struct data *d) { d->candidate = false; d->fired = false; d->anchor_id = -1; d->second_id = -1; }
static void process(const struct device *dev)
{
	struct data *d=dev->data; const struct cfg *c=dev->config; int64_t now=k_uptime_get();
	if (d->reset) { reset(d); d->reset=false; return; }
	if (d->fingers == 1 && d->previous == 0) {
		for (uint8_t i=0;i<TRAVELERS_PAD_MAX_CONTACTS;i++) if (d->valid[i]) { d->anchor=i; d->anchor_id=d->id[i]; d->ax=d->x[i]; d->ay=d->y[i]; break; }
	}
	if (d->fingers == 2 && d->previous < 2 && d->anchor_id >= 0) {
		for (uint8_t i=0;i<TRAVELERS_PAD_MAX_CONTACTS;i++) if (d->valid[i] && i!=d->anchor) { d->second=i; d->second_id=d->id[i]; d->sx=d->x[i]; d->sy=d->y[i]; d->second_start=now; d->candidate=true; break; }
	}
	if (d->candidate && d->fingers == 2 && (!d->valid[d->anchor] || d->id[d->anchor]!=d->anchor_id ||
		absi((int)d->x[d->anchor]-d->ax)>c->slop || absi((int)d->y[d->anchor]-d->ay)>c->slop ||
		absi((int)d->x[d->second]-d->sx)>c->slop || absi((int)d->y[d->second]-d->sy)>c->slop)) d->candidate=false;
	if (d->candidate && !d->fired && d->previous == 2 && d->fingers == 1 && d->valid[d->anchor] && d->id[d->anchor]==d->anchor_id && now-d->second_start<=c->max_ms && absi((int)d->sx-d->ax)>=c->separation && travelers_pad_session_claim(TRAVELERS_PAD_OWNER_TIP_TAP)) {
		bool left=d->sx<d->ax; if (c->invert) left=!left; d->fired=true;
		LOG_INF("Tip-Tap %s anchor=%d second=%d duration=%lld", left?"Left":"Right", d->anchor_id,d->second_id,now-d->second_start);
		if (d->callback) { d->callback(dev,0,left?0:1,true); d->callback(dev,0,left?0:1,false); }
	}
	if (d->fingers==0) reset(d); d->previous=d->fingers;
}
static int configure(const struct device *dev, kscan_callback_t cb) { if(!cb) return -EINVAL; ((struct data*)dev->data)->callback=cb; return 0; }
static const struct kscan_driver_api api={.config=configure};
#define INST(n) \
	static struct data data_##n={.anchor_id=-1,.second_id=-1}; \
	static const struct cfg cfg_##n={.max_ms=DT_INST_PROP_OR(n,second_contact_timeout_ms,180),.slop=DT_INST_PROP_OR(n,contact_slop,75),.separation=DT_INST_PROP_OR(n,minimum_horizontal_separation,160),.invert=DT_INST_PROP_OR(n,direction_inverted,false)}; \
	static void input_##n(struct input_event *e) { struct data *d=&data_##n; if(e->type==INPUT_EV_ABS) { if(e->code==INPUT_ABS_MT_SLOT)d->slot=e->value; else if(e->code==TRAVELERS_PAD_INPUT_ABS_MT_TRACKING_ID && d->slot<TRAVELERS_PAD_MAX_CONTACTS){d->valid[d->slot]=e->value>=0;d->id[d->slot]=e->value;} else if(e->code==TRAVELERS_PAD_INPUT_ABS_MT_POSITION_X && d->slot<TRAVELERS_PAD_MAX_CONTACTS)d->x[d->slot]=e->value; else if(e->code==TRAVELERS_PAD_INPUT_ABS_MT_POSITION_Y && d->slot<TRAVELERS_PAD_MAX_CONTACTS)d->y[d->slot]=e->value; else if(e->code==INPUT_ABS_Z)d->fingers=e->value; else if(e->code==INPUT_ABS_RZ && e->value)d->reset=true;} if(e->sync)process(DEVICE_DT_INST_GET(n)); } \
	INPUT_CALLBACK_DEFINE(DEVICE_DT_GET(DT_INST_PHANDLE(n,input_device)),input_##n); \
	DEVICE_DT_INST_DEFINE(n,NULL,NULL,&data_##n,&cfg_##n,POST_KERNEL,CONFIG_TRAVELERS_PAD_IQS9150_FUNCTION_INIT_PRIORITY,&api);
DT_INST_FOREACH_STATUS_OKAY(INST)
