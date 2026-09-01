/* Shared edge-dial ownership between Travelers Pad input function modules. */
#pragma once

#include <zephyr/sys/util.h>

#if IS_ENABLED(CONFIG_TRAVELERS_PAD_IQS9150_EDGE_DIAL_KSCAN)
bool travelers_pad_edge_dial_is_active(void);
#else
static inline bool travelers_pad_edge_dial_is_active(void)
{
	return false;
}
#endif
