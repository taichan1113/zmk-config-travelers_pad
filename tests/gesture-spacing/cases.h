static struct device dev;
static struct { bool down; int64_t t; uint32_t row,col; } events[128];
static int count;
static void cb(const struct device *d,uint32_t row,uint32_t col,bool down) {
    (void)d;assert(count<128);
    events[count].down=down;events[count].t=now;events[count].row=row;events[count++].col=col;
}
static void tick(int64_t t) { now=t;behavior_work_handler(&behavior_work.work); }
static void reset(void) { behavior_count=behavior_head=0;behavior_deadline=now=scheduled=0;count=0;authorized=true; }
static int queue(int col) { return travelers_pad_output_behavior_primary(&dev,cb,1,REC,OWNER,0,col,"test"); }
int main(void) {
    reset();assert(queue(1)==0);tick(0);assert(count==1 && events[0].down);
    tick(49);assert(count==1 && scheduled==50);
    tick(50);assert(count==2 && !events[1].down && events[1].t-events[0].t==50);
    reset();assert(queue(1)==0);assert(queue(1)==0);tick(0);tick(50);tick(54);
    assert(count==2);tick(55);tick(105);assert(count==4 && events[2].down && !events[3].down);
    reset();assert(queue(1)==0);cancel_behavior_actions();tick(0);assert(count==0);
    reset();assert(queue(1)==0);assert(queue(2)==0);tick(0);cancel_behavior_actions();tick(1);
    assert(count==2 && !events[1].down);tick(2);assert(count==2 && behavior_count==0);
    reset();authorized=false;assert(queue(1)==-EACCES);assert(behavior_count==0);
    authorized=true;assert(travelers_pad_output_behavior_primary(&dev,NULL,1,REC,OWNER,0,0,"test")==-ENODEV);
    reset();for(int i=0;i<BEHAVIOR_QUEUE_SIZE;i++)assert(queue(i)==0);
    assert(queue(99)==-ENOSPC);cancel_behavior_actions();tick(0);assert(count==0);
    reset();assert(travelers_pad_output_behavior_primary(&dev,cb,1,
        TRAVELERS_PAD_RECOGNIZER_EDGE_DIAL,OWNER,0,0,"edge")==0);
    assert(count==2 && events[0].down && !events[1].down && behavior_count==0);
    puts("PASS: 50ms gesture down/up, same-cell repeats, 5ms gap, reset cancellation, token rejection, bounded queue, edge unchanged");
    return 0;
}
