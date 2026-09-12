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

#include <travelers_pad/contact_output_broker.h>

LOG_MODULE_REGISTER(travelers_pad_output_broker, LOG_LEVEL_INF);

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
	if (!travelers_pad_arbiter_authorize_output(ownership_frame_seq, recognizer, owner,
					     capability, producer)) {
		return -EACCES;
	}
	return emit(dev, type, code, value, sync, producer);
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
	(void)travelers_pad_arbiter_note_terminal_output(ownership_frame_seq, recognizer, owner,
						 capability, producer);
	return emit(dev, type, code, value, sync, producer);
}

int travelers_pad_output_terminal_tap(const struct device *dev, uint8_t type, uint16_t code,
				      int32_t value, bool sync, const char *producer)
{
	return emit(dev, type, code, value, sync, producer);
}

int travelers_pad_output_routed_pointer(const struct device *dev, uint8_t type, uint16_t code,
				 int32_t value, bool sync, const char *producer)
{
	return emit(dev, type, code, value, sync, producer);
}
