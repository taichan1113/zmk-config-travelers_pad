/*
 * Travelers Pad contact output broker implementation.
 *
 * Authorization and transport deliberately live together: a future producer
 * cannot accidentally check one token and emit from an unrelated delayed path.
 *
 * Travelers Pad 接触出力brokerの実装。
 * authorizationとtransportを意図的に同居させる。将来のproducerがあるtokenを検査して
 * 別の遅延経路から出力してしまうことを防ぐためである。
 */

#include <errno.h>

#include <zephyr/input/input.h>
#include <zephyr/logging/log.h>
#include <zephyr/spinlock.h>
#include <zephyr/sys/util.h>

#include <travelers_pad/contact_output_broker.h>

LOG_MODULE_REGISTER(travelers_pad_output_broker, LOG_LEVEL_INF);

#define TRAVELERS_PAD_OUTPUT_MAX_HELD_BUTTONS 4

/* The broker owns this small ledger rather than relying on recognizer-local
 * state alone. A terminal release is therefore permitted only for the exact
 * trackpad press that passed through this transport. This prevents a future
 * stale path from releasing the same host button held by another producer.
 * recognizer局所stateだけに頼らず、brokerがこの小さなledgerを所有する。terminal
 * releaseはこのtransportを通った正確なtrackpad pressにだけ許可される。これにより将来の
 * stale経路が、別producerが保持する同じhost buttonをreleaseすることを防ぐ。 */
struct held_button_record {
	const struct device *dev;
	uint32_t ownership_frame_seq;
	enum travelers_pad_recognizer_id recognizer;
	enum travelers_pad_owner owner;
	uint16_t button;
	bool held;
};

static struct held_button_record held_buttons[TRAVELERS_PAD_OUTPUT_MAX_HELD_BUTTONS];
static struct k_spinlock held_buttons_lock;

static bool held_button_matches(const struct held_button_record *record, const struct device *dev,
				uint32_t ownership_frame_seq,
				enum travelers_pad_recognizer_id recognizer,
				enum travelers_pad_owner owner, uint16_t button)
{
	return record->held && record->dev == dev &&
	       record->ownership_frame_seq == ownership_frame_seq &&
	       record->recognizer == recognizer && record->owner == owner &&
	       record->button == button;
}

static bool held_button_exists(const struct device *dev, uint32_t ownership_frame_seq,
			       enum travelers_pad_recognizer_id recognizer,
			       enum travelers_pad_owner owner, uint16_t button)
{
	k_spinlock_key_t key = k_spin_lock(&held_buttons_lock);
	bool found = false;

	for (size_t index = 0; index < ARRAY_SIZE(held_buttons); index++) {
		if (held_button_matches(&held_buttons[index], dev, ownership_frame_seq, recognizer,
					owner, button)) {
			found = true;
			break;
		}
	}
	k_spin_unlock(&held_buttons_lock, key);
	return found;
}

static bool remember_held_button(const struct device *dev, uint32_t ownership_frame_seq,
				enum travelers_pad_recognizer_id recognizer,
				enum travelers_pad_owner owner, uint16_t button)
{
	k_spinlock_key_t key = k_spin_lock(&held_buttons_lock);
	struct held_button_record *empty = NULL;

	for (size_t index = 0; index < ARRAY_SIZE(held_buttons); index++) {
		if (held_button_matches(&held_buttons[index], dev, ownership_frame_seq, recognizer,
					owner, button)) {
			k_spin_unlock(&held_buttons_lock, key);
			return false;
		}
		if (!held_buttons[index].held && empty == NULL) {
			empty = &held_buttons[index];
		}
	}
	if (empty != NULL) {
		*empty = (struct held_button_record){
			.dev = dev,
			.ownership_frame_seq = ownership_frame_seq,
			.recognizer = recognizer,
			.owner = owner,
			.button = button,
			.held = true,
		};
	}
	k_spin_unlock(&held_buttons_lock, key);
	return empty != NULL;
}

static bool take_held_button(const struct device *dev, uint32_t ownership_frame_seq,
			     enum travelers_pad_recognizer_id recognizer,
			     enum travelers_pad_owner owner, uint16_t button)
{
	k_spinlock_key_t key = k_spin_lock(&held_buttons_lock);
	bool found = false;

	for (size_t index = 0; index < ARRAY_SIZE(held_buttons); index++) {
		if (held_button_matches(&held_buttons[index], dev, ownership_frame_seq, recognizer,
					owner, button)) {
			held_buttons[index].held = false;
			found = true;
			break;
		}
	}
	k_spin_unlock(&held_buttons_lock, key);
	return found;
}

static int emit(const struct device *dev, uint8_t type, uint16_t code, int32_t value, bool sync,
		const char *producer)
{
	int rc = input_report(dev, type, code, value, sync, K_NO_WAIT);

	if (rc != 0) {
		LOG_ERR("%s output report failed: %d", producer, rc);
	}
	return rc;
}

int travelers_pad_output_primary(const struct device *dev, uint32_t ownership_frame_seq,
				enum travelers_pad_recognizer_id recognizer,
				enum travelers_pad_owner owner, uint32_t capability,
				uint8_t type, uint16_t code, int32_t value, bool sync,
				const char *producer)
{
	int rc;

	if (!travelers_pad_arbiter_authorize_output(ownership_frame_seq, recognizer, owner,
					     capability, producer)) {
		return -EACCES;
	}
	if (capability == TRAVELERS_PAD_CAP_BUTTON && type == INPUT_EV_KEY && value != 0 &&
	    held_button_exists(dev, ownership_frame_seq, recognizer, owner, code)) {
		LOG_WRN("%s duplicate held button denied frame=%u button=%u", producer,
			ownership_frame_seq, code);
		return -EALREADY;
	}
	if (capability == TRAVELERS_PAD_CAP_BUTTON && type == INPUT_EV_KEY && value != 0 &&
	    !remember_held_button(dev, ownership_frame_seq, recognizer, owner, code)) {
		LOG_ERR("%s held-button ledger full frame=%u button=%u", producer,
			ownership_frame_seq, code);
		return -ENOMEM;
	}
	rc = emit(dev, type, code, value, sync, producer);
	if (rc != 0 && capability == TRAVELERS_PAD_CAP_BUTTON && type == INPUT_EV_KEY && value != 0) {
		(void)take_held_button(dev, ownership_frame_seq, recognizer, owner, code);
	}
	return rc;
}

int travelers_pad_output_auxiliary(const struct device *dev,
				  enum travelers_pad_auxiliary_id auxiliary,
				  enum travelers_pad_owner owner, uint32_t capability,
				  uint8_t type, uint16_t code, int32_t value, bool sync,
				  const char *producer)
{
	if (!travelers_pad_arbiter_authorize_auxiliary_output(auxiliary, owner, capability, producer)) {
		return -EACCES;
	}
	return emit(dev, type, code, value, sync, producer);
}

int travelers_pad_output_post_session(const struct device *dev,
				    const struct travelers_pad_owner_token *lease,
				    enum travelers_pad_owner owner, uint32_t capability,
				    uint8_t type, uint16_t code, int32_t value, bool sync,
				    const char *producer)
{
	if (!travelers_pad_arbiter_authorize_post_session_output(lease, owner, capability, producer)) {
		return -EACCES;
	}
	return emit(dev, type, code, value, sync, producer);
}

int travelers_pad_output_terminal(const struct device *dev, uint32_t ownership_frame_seq,
				 enum travelers_pad_recognizer_id recognizer,
				 enum travelers_pad_owner owner, uint32_t capability,
				 uint8_t type, uint16_t code, int32_t value, bool sync,
				 const char *producer)
{
	if (capability == TRAVELERS_PAD_CAP_BUTTON && type == INPUT_EV_KEY && value == 0 &&
	    !take_held_button(dev, ownership_frame_seq, recognizer, owner, code)) {
		LOG_WRN("%s terminal button release denied: no matching held press frame=%u button=%u",
			producer, ownership_frame_seq, code);
		return -EACCES;
	}
	(void)travelers_pad_arbiter_note_terminal_output(ownership_frame_seq, recognizer, owner,
						 capability, producer);
	int rc = emit(dev, type, code, value, sync, producer);

	if (rc != 0 && capability == TRAVELERS_PAD_CAP_BUTTON && type == INPUT_EV_KEY && value == 0) {
		(void)remember_held_button(dev, ownership_frame_seq, recognizer, owner, code);
	}
	return rc;
}

int travelers_pad_output_terminal_click(
	const struct device *dev, struct travelers_pad_terminal_action_grant *grant,
	enum travelers_pad_recognizer_id recognizer, enum travelers_pad_owner owner,
	uint16_t button, uint8_t click_count, const char *producer)
{
	int rc;

	if (click_count == 0 ||
	    !travelers_pad_arbiter_consume_terminal_action(
		grant, recognizer, owner, TRAVELERS_PAD_CAP_BUTTON, producer)) {
		return -EACCES;
	}
	for (uint8_t index = 0; index < click_count; index++) {
		rc = emit(dev, INPUT_EV_KEY, button, 1, true, producer);
		if (rc != 0) {
			return rc;
		}
		rc = emit(dev, INPUT_EV_KEY, button, 0, true, producer);
		if (rc != 0) {
			return rc;
		}
	}
	return 0;
}

int travelers_pad_output_behavior_primary(
	const struct device *dev, kscan_callback_t callback, uint32_t ownership_frame_seq,
	enum travelers_pad_recognizer_id recognizer, enum travelers_pad_owner owner,
	uint16_t row, uint16_t column, const char *producer)
{
	if (callback == NULL) {
		LOG_WRN("%s ignored: KSCAN callback is not configured", producer);
		return -ENODEV;
	}
	if (!travelers_pad_arbiter_authorize_output(
		ownership_frame_seq, recognizer, owner, TRAVELERS_PAD_CAP_BEHAVIOR_ACTION,
		producer)) {
		return -EACCES;
	}

	callback(dev, row, column, true);
	callback(dev, row, column, false);
	return 0;
}

int travelers_pad_output_routed_pointer(const struct device *dev, uint8_t type, uint16_t code,
				 int32_t value, bool sync, const char *producer)
{
	return emit(dev, type, code, value, sync, producer);
}
