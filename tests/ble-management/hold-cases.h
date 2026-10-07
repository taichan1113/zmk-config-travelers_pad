static struct gesture_config cfg={.swipe_min_move=180,.swipe_max_ms=700,.five_finger_radial_move=80,.five_finger_stable_frames=3,.four_finger_hold_ms=1500,.four_finger_hold_slop=60};
static struct gesture_data data;
static struct device dev={&cfg,&data};
static void begin(void) {data=(struct gesture_data){.dev=&dev,.fingers=4,.arbiter_frame_seq=1};owner=TRAVELERS_PAD_OWNER_NONE;actions=0;clock_ms=0;for(int i=0;i<4;i++){data.slot_valid[i]=true;data.slot_tracking_id[i]=i;data.slot_x[i]=400+100*i;data.slot_y[i]=400;}gesture_process_frame(&dev);}
static void tick(int64_t ms) {clock_ms=ms;data.arbiter_frame_seq++;gesture_process_frame(&dev);}
static void lift(void) {data.fingers=0;for(int i=0;i<7;i++)data.slot_valid[i]=false;tick(clock_ms+10);}
int main(void) {
 begin();tick(1499);assert(!data.hold_confirmed && actions==0);tick(1500);assert(data.hold_confirmed && actions==0);lift();assert(actions==1 && last_row==2 && last_column==2);tick(1600);assert(actions==1);
 begin();data.slot_x[0]+=30;tick(1500);assert(data.hold_confirmed);lift();assert(actions==1);
 begin();data.slot_x[0]+=61;tick(1500);assert(!data.hold_confirmed && data.hold_cancelled);lift();assert(actions==0);
 begin();data.fingers=3;data.slot_valid[3]=false;tick(100);data.fingers=4;data.slot_valid[3]=true;tick(1600);assert(!data.hold_confirmed);lift();assert(actions==0);
 begin();data.slot_tracking_id[2]=99;tick(1500);assert(!data.hold_confirmed);lift();assert(actions==0);
 begin();data.sensor_reset=true;tick(1500);assert(!data.hold_confirmed);lift();assert(actions==0);
 begin();owner=TRAVELERS_PAD_OWNER_DRAG;tick(1500);assert(!data.hold_confirmed);lift();assert(actions==0);
 begin();owner=TRAVELERS_PAD_OWNER_SCROLL;tick(1500);assert(!data.hold_confirmed);lift();assert(actions==0);
 begin();for(int i=0;i<4;i++)data.slot_x[i]+=200;tick(200);assert(data.moved && !data.hold_confirmed);lift();assert(actions==1 && last_row==1 && last_column==GESTURE_RIGHT);
 cfg.four_finger_hold_ms=0;begin();tick(2000);lift();assert(actions==0);
 cfg.four_finger_hold_ms=1000;cfg.four_finger_hold_immediate=true;
 begin();tick(999);assert(!data.hold_confirmed && actions==0);
 tick(1000);assert(data.hold_confirmed && data.pending_hold_action && actions==0);
 multifinger_resolve_action(&data,NULL,data.ownership_frame_seq+1);assert(actions==0 && data.pending_hold_action);
 multifinger_resolve_action(&data,NULL,data.ownership_frame_seq);assert(actions==1 && data.fingers==4 && !data.pending_hold_action);
 tick(2000);multifinger_resolve_action(&data,NULL,data.ownership_frame_seq);assert(actions==1);
 lift();assert(actions==1);tick(3000);assert(actions==1);
 begin();tick(1000);owner=TRAVELERS_PAD_OWNER_DRAG;
 multifinger_resolve_action(&data,NULL,data.ownership_frame_seq);assert(actions==0);lift();assert(actions==0);
 begin();tick(500);data.sensor_reset=true;tick(1000);assert(!data.pending_hold_action && actions==0);
 puts("PASS: immediate 1s hold waits for resolver, emits while down once, no release duplicate/retrigger, reset and denied output safe");
 puts("PASS: actual gesture code: 4F timing, slop, identity, contact changes, reset, competing owners, one-shot release, swipe preservation, default-disabled");
}
