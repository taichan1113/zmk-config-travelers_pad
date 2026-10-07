
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
#ifdef IQS9150_IDLE_TIMEOUT_REG
 cfg.diagnostic_disable_lp_entry=true;
 iqs9150_init_payload(&cfg,0x11a2,source,payload,sizeof(source));
 for(int i=0;i<26;i++)assert(payload[i]==(i==14 || i==15?0:0xa5));
 iqs9150_init_payload(&cfg,0x11af,source,payload,2);assert(payload[0]==0xa5 && payload[1]==0);
 iqs9150_init_payload(&cfg,0x11b1,source,payload,2);assert(payload[0]==0 && payload[1]==0xa5);
 setup(IQS9150_WRITE_CONFIG);data.block_index=ARRAY_SIZE(ev02_blocks);run();assert(data.state==IQS9150_VERIFY_MODE_TIMEOUTS);
 active=0;run();assert(reads==0 && comms_requests==1);active=1;run();assert(reads==1 && data.state==IQS9150_START_ATI);
 setup(IQS9150_VERIFY_MODE_TIMEOUTS);diagnostic_timeout_mismatch=true;run();assert(data.state==IQS9150_ERROR);
 diagnostic_timeout_mismatch=false;cfg.diagnostic_disable_lp_entry=false;
 puts("PASS: idle-only patch preserves four other timeouts; RDY-gated two-byte verification and failure");
#endif
 setup(IQS9150_WRITE_CONFIG);data.block_index=ARRAY_SIZE(ev02_blocks);run();assert(data.state==IQS9150_START_ATI);
#ifdef IQS9150_LP1_TIMEOUT_REG
 for(int limit=IQS9150_POWER_LIMIT_ACTIVE;limit<=IQS9150_POWER_LIMIT_LP1;limit++) {
  cfg.power_mode_limit=limit;
  iqs9150_init_payload(&cfg,0x11a2,source,payload,sizeof(source));
  int first=limit==1?10:limit==2?14:16, end=limit==1?20:first+2;
  for(int i=0;i<26;i++)assert(payload[i]==(i>=first && i<end?0:0xa5));
  uint16_t reg=iqs9150_timeout_patch_reg(&cfg);
  iqs9150_init_payload(&cfg,reg-1,source,payload,2);assert(payload[0]==0xa5 && payload[1]==0);
  iqs9150_init_payload(&cfg,reg+(limit==1?10:2)-1,source,payload,2);assert(payload[0]==0 && payload[1]==0xa5);
  setup(IQS9150_WRITE_CONFIG);data.block_index=ARRAY_SIZE(ev02_blocks);run();assert(data.state==IQS9150_VERIFY_MODE_TIMEOUTS);
  active=0;run();assert(reads==0 && comms_requests==1);active=1;run();assert(data.state==IQS9150_START_ATI);
  setup(IQS9150_VERIFY_MODE_TIMEOUTS);diagnostic_timeout_mismatch=true;run();assert(data.state==IQS9150_ERROR);diagnostic_timeout_mismatch=false;
 }
 cfg.diagnostic_disable_mode_timeouts=true;assert(iqs9150_power_limit(&cfg)==IQS9150_POWER_LIMIT_ACTIVE);
 cfg.diagnostic_disable_mode_timeouts=false;cfg.diagnostic_disable_lp_entry=true;assert(iqs9150_power_limit(&cfg)==IQS9150_POWER_LIMIT_IDLE);
 cfg.diagnostic_disable_lp_entry=false;cfg.power_mode_limit=IQS9150_POWER_LIMIT_NONE;
 iqs9150_init_payload(&cfg,0x11a2,source,payload,sizeof(source));assert(memcmp(source,payload,sizeof(source))==0);
 puts("PASS: all power limits, chunk boundaries, unchanged none, readback errors and legacy precedence");
#endif
#if __has_include(<iqs9150/diagnostics.h>)
 cfg.power_diagnostics=true;
 setup(IQS9150_WRITE_CONFIG);data.block_index=ARRAY_SIZE(ev02_blocks);run();assert(data.state==IQS9150_READ_POWER_SETUP);
 active=0;run();assert(reads==0 && comms_requests==1);active=1;run();assert(reads==1 && data.power_setup_phase==1);
 run();assert(reads==2 && data.state==IQS9150_START_ATI);
 setup(IQS9150_READ_POWER_SETUP);power_setup_empty=true;run();assert(data.state==IQS9150_ERROR);power_setup_empty=false;
 setup(IQS9150_RUNNING);
 for(int mode=0;mode<8;mode++){now=100+mode;iqs9150_observe_power(&data,0x120|mode);}
 iqs9150_observe_power(&data,0xeeee);
 struct iqs9150_power_observation cached;
 assert(iqs9150_get_power_observation(&dev,&cached)==0 && reads==0 && comms_requests==0);
 assert(cached.sampled_ms==107 && cached.info_flags==0x127);
 for(int mode=0;mode<8;mode++)assert(cached.mode_reports[mode]==1);
 assert(iqs9150_get_power_observation(NULL,&cached)==-EINVAL);
 assert(iqs9150_get_power_observation(&dev,NULL)==-EINVAL);
 cfg.power_diagnostics=false;
 puts("PASS: power setup RDY gating/empty rejection and cached modes/ALP flags without I2C requests");
#endif
 puts("PASS: diagnostic timeout patch boundaries, unchanged default, RDY-gated readback and mismatch failure");
#endif
 puts("PASS: actual driver state machine: warm-start request/RDY/reset, bounded requests, reset 0x0200, active-low gating, reset timeout, idle silence, level rescue, termination bit, pending delay");
}
