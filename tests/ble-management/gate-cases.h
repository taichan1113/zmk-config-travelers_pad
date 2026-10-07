int main(void) {
 struct gate_config cfg={.layer=1};
 struct gate_data data={0};
 struct device dev={&cfg,&data};
 struct input_event e={INPUT_EV_KEY,INPUT_BTN_0,1};
 active=false;assert(gate_event(&dev,&e,0,0,0)==0);
 active=true;e.value=0;assert(gate_event(&dev,&e,0,0,0)==0);
 e.value=1;assert(gate_event(&dev,&e,0,0,0)==1);
 active=false;e.value=0;assert(gate_event(&dev,&e,0,0,0)==1);
 e.type=INPUT_EV_REL;active=true;assert(gate_event(&dev,&e,0,0,0)==1);
 active=false;assert(gate_event(&dev,&e,0,0,0)==0);
 cfg.grid_only=true;assert(gate_event(&dev,&e,0,0,0)==1);
 active=true;assert(gate_event(&dev,&e,0,0,0)==0);
 puts("PASS: actual keyboard gate: normal output, management suppression, paired releases across layer transitions, grid isolation");
}
