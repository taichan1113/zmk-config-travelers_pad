#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#define INPUT_EV_KEY 1
#define INPUT_EV_REL 2
#define INPUT_BTN_0 256
#define ZMK_INPUT_PROC_CONTINUE 0
#define ZMK_INPUT_PROC_STOP 1
#define BIT(n) (1U<<(n))
#define ARG_UNUSED(x) ((void)(x))
struct device {const void *config;void *data;};
struct input_event {int type,code,value;};
struct zmk_input_processor_state {int dummy;};
static bool active;
static bool zmk_keymap_layer_active(int layer) {(void)layer;return active;}
