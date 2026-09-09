/* Shared immutable contact-session vocabulary. */
#ifndef TRAVELERS_PAD_CONTACT_SESSION_H_
#define TRAVELERS_PAD_CONTACT_SESSION_H_

#include <stdbool.h>
#include <stdint.h>

enum travelers_pad_owner {
	TRAVELERS_PAD_OWNER_NONE,
	TRAVELERS_PAD_OWNER_POINTER_PROVISIONAL,
	TRAVELERS_PAD_OWNER_DRAG,
	TRAVELERS_PAD_OWNER_SCROLL,
	TRAVELERS_PAD_OWNER_MULTI_GESTURE,
	TRAVELERS_PAD_OWNER_EDGE_DIAL,
	TRAVELERS_PAD_OWNER_TIP_TAP,
};

enum travelers_pad_session_end_reason {
	TRAVELERS_PAD_SESSION_END_RELEASE,
	TRAVELERS_PAD_SESSION_END_CANCEL,
};

struct travelers_pad_session_snapshot {
	uint32_t session_id;
	uint32_t frame_seq;
	int64_t timestamp_ms;
	uint8_t contact_count;
	uint8_t peak_contact_count;
	enum travelers_pad_owner owner;
	uint32_t owner_generation;
	bool tap_allowed;
	bool cancelled;
};

/* Stage-2 shared claim API. Only the session core owns this state; legacy
 * recognizers use it while their raw subscriptions are migrated. */
bool travelers_pad_session_claim(enum travelers_pad_owner owner);
bool travelers_pad_session_tap_allowed(void);
enum travelers_pad_owner travelers_pad_session_owner(void);

#endif
