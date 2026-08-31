/* Shared drag ownership between Travelers Pad input function modules. */
#pragma once

#include <zephyr/sys/util.h>

#if IS_ENABLED(CONFIG_TRAVELERS_PAD_IQS9150_TAP_DRAG)
bool travelers_pad_tap_drag_is_active(void);
bool travelers_pad_tap_drag_is_tracking(void);
#else
static inline bool travelers_pad_tap_drag_is_active(void)
{
	return false;
}
static inline bool travelers_pad_tap_drag_is_tracking(void)
{
	return false;
}
#endif
