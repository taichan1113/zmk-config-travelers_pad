
static struct iqs9150_config cfg={.rdy_watch_ms=250};
static struct iqs9150_data data;
static struct device dev={&cfg,&data};
static void setup(enum iqs9150_state state) {data=(struct iqs9150_data){.dev=&dev,.state=state};now=0;active=1;info_flags=0;reads=reports=terminations=reset_writes=0;}
static void run(void) {data.work.work.pending=0;iqs9150_work_handler(&data.work.work);}
int main(void){
 setup(IQS9150_RECOVER_RESET);active=0;run();assert(reset_writes==0 && data.state==IQS9150_RECOVER_RESET);
 active=1;run();assert(reset_writes==1 && last_low==0 && last_high==2 && data.state==IQS9150_RECOVER_WAIT_RESET);
 info_flags=IQS9150_INFO_SHOW_RESET;run();assert(data.state==IQS9150_ACK_RESET);
 setup(IQS9150_RECOVER_RESET);run();
 for(int i=1;i<=3;i++){now=i*2000;active=0;run();if(i<3){assert(data.state==IQS9150_RECOVER_RESET);active=1;run();}else assert(data.state==IQS9150_ERROR);}
 assert(reset_writes==3);
 setup(IQS9150_RUNNING);active=0;now=600000;run();iqs9150_rdy_watch_handler(&data.rdy_watch_work.work);assert(reports==0 && reset_writes==0 && !data.work.work.pending && data.state==IQS9150_RUNNING);
 active=1;iqs9150_rdy_watch_handler(&data.rdy_watch_work.work);assert(data.work.work.pending && data.rdy_rescues==1);run();assert(reports==1);
 setup(IQS9150_RECOVER_WAIT_RESET);active=0;now=2000;iqs9150_rdy_watch_handler(&data.rdy_watch_work.work);assert(data.work.work.pending);
 setup(IQS9150_READ_COMMS_CONFIG);config_word=0x461e;run();assert(!data.terminate_comms_window && terminations==0);
 setup(IQS9150_READ_COMMS_CONFIG);config_word=0x465e;run();assert(data.terminate_comms_window && terminations==1);
 setup(IQS9150_READ_COMMS_CONFIG);active=0;run();assert(reads==0);
 setup(IQS9150_WAIT_ATI);active=0;run();assert(reads==0);
 setup(IQS9150_RUNNING);data.work.work.pending=1;iqs9150_rdy_watch_handler(&data.rdy_watch_work.work);assert(data.rdy_rescues==0);
 puts("PASS: actual driver state machine: reset 0x0200, active-low logical gating, reset success/timeout, idle silence, stranded RDY rescue, correct termination bit, pending delay preserved");
}

