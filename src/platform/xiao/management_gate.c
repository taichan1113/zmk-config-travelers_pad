/* SPDX-License-Identifier: MIT
 * Keyboard-only output policy. No raw-frame or ownership mutation.
 * キーボード固有の出力制御。生frame・所有権は変更しない。 */
#define DT_DRV_COMPAT zmk_travelers_pad_management_gate
#include <zephyr/device.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>
#include <drivers/input_processor.h>
#include <zmk/keymap.h>

struct gate_config { uint8_t layer; bool grid_only; };
struct gate_data { uint16_t delivered_buttons; };

static int gate_event(const struct device *dev, struct input_event *event,
                      uint32_t p1, uint32_t p2,
                      struct zmk_input_processor_state *state)
{
 const struct gate_config *cfg = dev->config;
 struct gate_data *data = dev->data;
 bool managing = zmk_keymap_layer_active(cfg->layer);
 ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(state);
 if (cfg->grid_only) return managing ? ZMK_INPUT_PROC_CONTINUE : ZMK_INPUT_PROC_STOP;
 /* Pass releases for presses previously delivered even across a layer change.
  * レイヤー切替をまたいでも、送信済み押下の解放を欠落させない。 */
 if (event->type == INPUT_EV_KEY && event->code >= INPUT_BTN_0 &&
     event->code < INPUT_BTN_0 + 16) {
  uint16_t bit = BIT(event->code - INPUT_BTN_0);
  if (event->value) {
   if (managing) return ZMK_INPUT_PROC_STOP;
   data->delivered_buttons |= bit;
  } else {
   bool delivered = data->delivered_buttons & bit;
   data->delivered_buttons &= ~bit;
   if (!delivered) return ZMK_INPUT_PROC_STOP;
  }
  return ZMK_INPUT_PROC_CONTINUE;
 }
 return managing ? ZMK_INPUT_PROC_STOP : ZMK_INPUT_PROC_CONTINUE;
}
static struct zmk_input_processor_driver_api gate_api = {.handle_event = gate_event};
#define GATE_INST(n) \
 static struct gate_data gate_data_##n; \
 static const struct gate_config gate_config_##n = { \
  .layer = DT_INST_PROP(n, layer), .grid_only = DT_INST_PROP(n, grid_only)}; \
 DEVICE_DT_INST_DEFINE(n, NULL, NULL, &gate_data_##n, &gate_config_##n, \
                      POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &gate_api);
DT_INST_FOREACH_STATUS_OKAY(GATE_INST)
