#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#define BIT(n) (1u << (n))
#define INPUT_EV_MSC 4
#define INPUT_EV_ABS 3
#define INPUT_EV_KEY 1
#define INPUT_ABS_X 0
#define INPUT_ABS_Y 1
#define INPUT_ABS_Z 2
#define INPUT_ABS_RZ 5
#define TRAVELERS_PAD_INPUT_MSC_ROUTED_FRAME_SEQ 100
#define LOG_DBG(...) ((void)0)
#define LOG_WRN(...) ((void)0)
struct device { const void *config; void *data; };
struct input_event { uint8_t type; uint16_t code; int32_t value; bool sync; };
static int motion_count, scroll_count;
static int16_t last_dx, last_dy;
