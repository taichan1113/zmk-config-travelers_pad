static struct cfg config = {.max_ms=180,.slop=75,.separation=160,.anchor_min_hold_ms=250};
static struct data state;
static struct device dev = {&config,&state};
static void fresh(void) {
    state=(struct data){.dev=&dev,.anchor_id=-1,.second_id=-1};
    owner=TRAVELERS_PAD_OWNER_NONE;proposals=actions=column=0;clock_ms=0;
}
static void contacts(int64_t t,bool anchor,bool second,bool third) {
    clock_ms=t;state.arbiter_frame_seq++;
    state.valid[0]=anchor;state.valid[1]=second;state.valid[2]=third;
    state.id[0]=10;state.id[1]=20;state.id[2]=30;
    state.x[0]=1000;state.y[0]=500;state.x[1]=500;state.y[1]=500;
    state.x[2]=1500;state.y[2]=500;
    state.fingers=anchor+second+third;process(&dev);
}
int main(void) {
    for(int i=0;i<100;i++) {
        fresh();contacts(0,1,0,0);contacts(30,1,1,0);contacts(100,1,0,0);
        contacts(120,0,0,0);assert(proposals==0 && actions==0);
    }
    fresh();contacts(0,1,0,0);contacts(249,1,1,0);contacts(300,1,0,0);
    assert(proposals==0);contacts(400,1,1,0);contacts(450,1,0,0);
    assert(proposals==0); /* Failed 2F time cannot age an anchor. */
    fresh();contacts(0,1,0,0);contacts(250,1,1,0);contacts(300,1,0,0);
    assert(proposals==1 && actions==0 && state.pending_action);
    tip_tap_resolve_action(&state,NULL,state.pending_frame_seq);assert(actions==1 && column==0);
    contacts(310,1,1,0);contacts(360,1,0,0);assert(proposals==1 && actions==2);
    contacts(370,1,1,0);contacts(420,1,0,0);assert(proposals==1 && actions==3);
    contacts(430,0,0,0);assert(state.anchor_id==-1 && state.previous==0 && !state.tip_owned);
    fresh();contacts(0,1,1,0);contacts(100,1,0,0);assert(proposals==0);
    fresh();contacts(0,1,0,0);contacts(250,1,1,0);contacts(260,1,1,1);
    contacts(280,1,1,0);contacts(300,1,0,0);assert(proposals==0);
    fresh();contacts(0,1,0,0);clock_ms=200;state.x[0]=1100;process(&dev);
    contacts(250,1,1,0);contacts(300,1,0,0);assert(proposals==0);
    fresh();contacts(0,1,0,0);contacts(250,1,1,0);
    state.id[0]=99;clock_ms=270;process(&dev);contacts(300,1,0,0);assert(proposals==0);
    fresh();contacts(0,1,0,0);contacts(250,1,1,0);owner=TRAVELERS_PAD_OWNER_SCROLL;
    contacts(300,1,0,0);assert(proposals==0 && actions==0);
    fresh();contacts(0,1,0,0);state.reset=true;clock_ms=100;process(&dev);
    assert(state.previous==0 && state.anchor_id==-1);
    config.anchor_min_hold_ms=0;fresh();contacts(0,1,0,0);contacts(1,1,1,0);
    contacts(50,1,0,0);assert(proposals==1); /* Explicit compatibility mode. */
    puts("PASS: 100 staggered taps rejected as Tip-Tap; 249/250ms boundary; repeats, motion, IDs, 3F, reset, competing owner");
    return 0;
}
