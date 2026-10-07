
static const uint8_t block_bytes[4]={0x1e,0x46,0xa8,0x80};
static const struct iqs9150_block ev02_blocks[]={{0x11be,block_bytes,4}};
#ifdef IQS9150_ALP_ENABLE_BYTE_REG
static const uint8_t ev02_11c2[]={0xff,0xff,0xff,0xd3};
#endif
static int i2c_write_dt(const struct i2c_dt_spec *spec, const uint8_t *bytes, size_t len) {
 (void)spec;assert(len==1 && bytes[0]==0xff);comms_requests++;return 0;
}
static uint16_t le16(const uint8_t *v){return v[0]|((uint16_t)v[1]<<8);}
static int read_reg(const struct device *dev,uint16_t reg,uint8_t *buf,size_t len) {
 (void)dev;memset(buf,0,len);reads++;
#if __has_include(<iqs9150/diagnostics.h>)
 if((reg==0x11a2 || reg==0x11be) && len==26) {
  if(reg==0x11be){buf[2]=power_setup_other;buf[3]=power_setup_other>>8;
   buf[4]=power_setup_alp;buf[5]=power_setup_alp>>8;buf[6]=power_setup_alp>>16;buf[7]=power_setup_alp>>24;}
  if(power_setup_empty)memset(buf,0xee,len);return 0;
 }
#endif
#ifdef IQS9150_MODE_TIMEOUT_FIRST_REG
 if(reg==IQS9150_MODE_TIMEOUT_FIRST_REG) {assert(len==10);buf[9]=diagnostic_timeout_mismatch;return 0;}
#ifdef IQS9150_IDLE_TIMEOUT_REG
 if(reg==IQS9150_IDLE_TIMEOUT_REG) {assert(len==2);buf[1]=diagnostic_timeout_mismatch;return 0;}
#endif
#ifdef IQS9150_LP1_TIMEOUT_REG
 if(reg==IQS9150_LP1_TIMEOUT_REG) {assert(len==2);buf[1]=diagnostic_timeout_mismatch;return 0;}
#endif
#endif
 uint16_t val= reg==IQS9150_INFO_FLAGS_REG?info_flags:reg==IQS9150_CONFIG_SETTINGS_REG?config_word:IQS9150_PRODUCT_NUMBER;
 buf[0]=val&255;buf[1]=val>>8;return 0;
}
static int write_reg(const struct device *dev,uint16_t reg,const uint8_t *buf,size_t len) {
 (void)dev;if(reg==IQS9150_CONFIG_SETTINGS_REG){event_write_len=len;event_write_word=le16(buf);}
 if(reg==IQS9150_SYSTEM_CONTROL_REG) {last_low=buf[0];last_high=buf[1];if(buf[0]==0 && buf[1]==2)reset_writes++;}return 0;
}
static int terminate_comms_if_needed(const struct iqs9150_data *d) {if(d->terminate_comms_window)terminations++;return 0;}
static void report_contact_frame(const struct device *dev) {(void)dev;reports++;}
static void iqs9150_start_runtime_recovery(struct iqs9150_data *d,const char *reason,bool reset) {(void)reason;d->state=reset?IQS9150_ACK_RESET:IQS9150_RECOVER_RESET;d->reset_request_attempts=0;}
