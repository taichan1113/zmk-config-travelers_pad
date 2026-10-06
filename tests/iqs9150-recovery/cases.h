
static struct iqs9150_config cfg={.rdy_watch_ms=250};
static struct iqs9150_data data;
static struct device dev={&cfg,&data};
static void setup(enum iqs9150_state state) {data=(struct iqs9150_data){.dev=&dev,.state=state};now=0;active=1;info_flags=0;reads=reports=terminations=reset_writes=comms_requests=0;}
static void run(void) {data.work.work.pending=0;iqs9150_work_handler(&data.work.work);}
int main(void){
 setup(IQS9150_RECOVER_RESET);active=0;run();assert(reset_writes==0 && data.state==IQS9150_RECOVER_RESET);
 active=1;run();assert(reset_writes==1 && last_low==0 && last_high==2 && data.state==IQS9150_RECOVER_WAIT_RESET);
 info_flags=IQS9150_INFO_SHOW_RESET;run();assert(data.state==IQS9150_ACK_RESET);
 setup(IQS9150_RECOVER_RESET);run();
 for(int i=1;i<=3;i++){now=i*2000;active=0;run();if(i<3){assert(data.state==IQS9150_RECOVER_RESET);active=1;run();}else assert(data.state==IQS9150_ERROR);}
 assert(reset_writes==3);
 setup(IQS9150_RUNNING);active=0;now=600000;run();iqs9150_rdy_watch_handler(&data.rdy_watch_work.work);assert(reports==0 && reset_writes==0 && comms_requests==0 && !data.work.work.pending && data.state==IQS9150_RUNNING);
 active=1;iqs9150_rdy_watch_handler(&data.rdy_watch_work.work);assert(data.work.work.pending && data.rdy_rescues==1);run();assert(reports==1);
 setup(IQS9150_RECOVER_WAIT_RESET);active=0;now=2000;iqs9150_rdy_watch_handler(&data.rdy_watch_work.work);assert(data.work.work.pending);
 setup(IQS9150_READ_COMMS_CONFIG);config_word=0x461e;run();assert(!data.terminate_comms_window && terminations==0);
 setup(IQS9150_READ_COMMS_CONFIG);config_word=0x465e;run();assert(data.terminate_comms_window && terminations==1);
 setup(IQS9150_READ_COMMS_CONFIG);active=0;run();assert(reads==0);
 setup(IQS9150_WAIT_ATI);active=0;run();assert(reads==0);
 setup(IQS9150_RUNNING);data.work.work.pending=1;iqs9150_rdy_watch_handler(&data.rdy_watch_work.work);assert(data.rdy_rescues==0);
 /* Warm Event Mode startup: request, wait RDY, identify, request next window,
  * read config, then explicitly reset before ACK and reconfiguration. */
 setup(IQS9150_WAIT_PRODUCT);active=0;config_word=0x471e;run();
 assert(comms_requests==1 && reads==0 && data.state==IQS9150_WAIT_PRODUCT);
 active=1;run();assert(reads==1 && data.state==IQS9150_READ_COMMS_CONFIG);
 active=0;run();assert(comms_requests==2 && reads==1);
 active=1;run();assert(data.state==IQS9150_RECOVER_RESET && !data.terminate_comms_window);
 active=0;run();assert(comms_requests==3 && reset_writes==0);
 active=1;run();assert(reset_writes==1 && data.state==IQS9150_RECOVER_WAIT_RESET);
 info_flags=IQS9150_INFO_SHOW_RESET;run();assert(data.state==IQS9150_ACK_RESET);
 setup(IQS9150_WAIT_PRODUCT);active=0;
 for (int i=0;i<=IQS9150_COMMS_REQUEST_RETRY_MAX;i++)run();
 assert(data.state==IQS9150_ERROR && comms_requests==IQS9150_COMMS_REQUEST_RETRY_MAX && reads==0);
#ifdef IQS9150_MODE_TIMEOUT_FIRST_REG
 uint8_t source[26], payload[26];memset(source,0xa5,sizeof(source));
 cfg.diagnostic_disable_mode_timeouts=false;
 iqs9150_init_payload(&cfg,0x11a2,source,payload,sizeof(source));assert(memcmp(source,payload,sizeof(source))==0);
 cfg.diagnostic_disable_mode_timeouts=true;
 iqs9150_init_payload(&cfg,0x11a2,source,payload,sizeof(source));
 for(int i=0;i<26;i++)assert(payload[i]==(i>=10 && i<20?0:0xa5));
 iqs9150_init_payload(&cfg,0x11ab,source,payload,2);assert(payload[0]==0xa5 && payload[1]==0);
 iqs9150_init_payload(&cfg,0x11b5,source,payload,2);assert(payload[0]==0 && payload[1]==0xa5);
 setup(IQS9150_WRITE_CONFIG);data.block_index=ARRAY_SIZE(ev02_blocks);run();assert(data.state==IQS9150_VERIFY_MODE_TIMEOUTS);
 active=0;run();assert(reads==0 && comms_requests==1);active=1;run();assert(data.state==IQS9150_START_ATI && reads==1);
 setup(IQS9150_VERIFY_MODE_TIMEOUTS);diagnostic_timeout_mismatch=true;run();assert(data.state==IQS9150_ERROR);
 diagnostic_timeout_mismatch=false;cfg.diagnostic_disable_mode_timeouts=false;
 setup(IQS9150_WRITE_CONFIG);data.block_index=ARRAY_SIZE(ev02_blocks);run();assert(data.state==IQS9150_START_ATI);
 puts("PASS: diagnostic timeout patch boundaries, unchanged default, RDY-gated readback and mismatch failure");
#endif
 puts("PASS: actual driver state machine: warm-start request/RDY/reset, bounded requests, reset 0x0200, active-low gating, reset timeout, idle silence, level rescue, termination bit, pending delay");
}
