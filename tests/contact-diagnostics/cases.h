int main(void) {
    struct iqs9150_data d={0};
    struct iqs9150_contact c[7];
    for (int i=0;i<7;i++) c[i]=(struct iqs9150_contact){UINT16_MAX,UINT16_MAX,false};
    observe_contact_report(&d,0,0,0,c); assert(logs==1);
    now_ms=5000;observe_contact_report(&d,0,0,0,c);assert(logs==1);
    c[0]=(struct iqs9150_contact){1000,500,true};
    observe_contact_report(&d,0,0x101,1,c); assert(logs==2);
    now_ms=5999;observe_contact_report(&d,0,0x101,1,c);assert(logs==2);
    now_ms=6000;observe_contact_report(&d,0,0x101,1,c);assert(logs==3);
    /* Raw second contact, but missing confidence: decoded remains one. */
    c[1]=(struct iqs9150_contact){500,700,false};
    now_ms=6010;observe_contact_report(&d,0,0x102,1,c);assert(logs==4);
    assert(strstr(last_log,"raw=2 decoded=1 mask=0x01 confidence=0x01 coordinates=0x03"));
    c[1].valid=true;
    observe_contact_report(&d,0,0x302,2,c);assert(logs==5);
    assert(strstr(last_log,"raw=2 decoded=2 mask=0x03"));
    /* Confidence stays set but coordinates become invalid. */
    c[1].x=UINT16_MAX;c[1].valid=false;
    observe_contact_report(&d,0,0x302,1,c);assert(logs==6);
    assert(strstr(last_log,"confidence=0x03 coordinates=0x01"));
    /* Last slot is represented in all masks, not only the first two. */
    c[6]=(struct iqs9150_contact){900,900,true};
    observe_contact_report(&d,0,0x4102,2,c);assert(logs==7);
    assert(strstr(last_log,"mask=0x41"));
    puts("PASS: raw/decoded counts, confidence/coordinate/slot masks, sparse slots, idle silence, bounded heartbeat");
    return 0;
}
