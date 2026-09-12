/*
 * Travelers Pad contact output broker.
 *
 * All host-facing relative motion, wheel, and button reports produced by a
 * contact-session feature pass through this interface.  The broker is the
 * single place that checks the arbiter token or post-session lease immediately
 * before emitting an input event.  Internal normalized-contact forwarding is
 * intentionally outside this interface.
 *
 * Travelers Pad 接触出力broker。
 * 接触session機能が生成するホスト向けの相対移動、ホイール、ボタンreportは、すべて
 * このinterfaceを通す。brokerはinput eventを出す直前にarbiter tokenまたは
 * post-session leaseを検査する唯一の場所である。正規化接触frameの内部配送は意図的に
 * 対象外とする。
 */
#ifndef TRAVELERS_PAD_CONTACT_OUTPUT_BROKER_H_
#define TRAVELERS_PAD_CONTACT_OUTPUT_BROKER_H_

#include <stdbool.h>
#include <stdint.h>

#include <zephyr/device.h>

#include <travelers_pad/contact_arbiter.h>

/* A primary owner output is emitted only while its exact arbiter token is
 * current. / primary ownerの出力は、完全に一致するarbiter tokenが現行の間だけ送る。 */
int travelers_pad_output_primary(const struct device *dev, uint32_t ownership_frame_seq,
				enum travelers_pad_recognizer_id recognizer,
				enum travelers_pad_owner owner, uint32_t capability,
				uint8_t type, uint16_t code, int32_t value, bool sync,
				const char *producer);

/* An auxiliary output is emitted only while the delegated active owner is
 * current. / auxiliary出力は委譲されたactive ownerが現行の間だけ送る。 */
int travelers_pad_output_auxiliary(const struct device *dev,
				  enum travelers_pad_auxiliary_id auxiliary,
				  enum travelers_pad_owner owner, uint32_t capability,
				  uint8_t type, uint16_t code, int32_t value, bool sync,
				  const char *producer);

/* A post-session output is emitted only while its copied, revocable lease is
 * current. / post-session出力は、コピーされた取消可能leaseが現行の間だけ送る。 */
int travelers_pad_output_post_session(const struct device *dev,
				    const struct travelers_pad_owner_token *lease,
				    enum travelers_pad_owner owner, uint32_t capability,
				    uint8_t type, uint16_t code, int32_t value, bool sync,
				    const char *producer);

/* Releases must remain fail-safe. Record the terminal path for diagnostics,
 * then always emit it even if the previous token was revoked.
 * releaseはfail-safeを維持する。診断用にterminal経路を記録した後、前のtokenが
 * 失効済みでも必ず送る。 */
int travelers_pad_output_terminal(const struct device *dev, uint32_t ownership_frame_seq,
				 enum travelers_pad_recognizer_id recognizer,
				 enum travelers_pad_owner owner, uint32_t capability,
				 uint8_t type, uint16_t code, int32_t value, bool sync,
				 const char *producer);

/* A tap click has no long-lived owner token: it is a terminal semantic action
 * after the session classifier completes. It still uses this broker so no
 * host-facing report bypasses the central transport point.
 * tap clickには長寿命owner tokenがない。session classifier完了後のterminal
 * semantic actionだが、ホスト向けreportが中央transportを迂回しないようbrokerを通す。 */
int travelers_pad_output_terminal_tap(const struct device *dev, uint8_t type, uint16_t code,
				      int32_t value, bool sync, const char *producer);

/* Canonical pointer output is already selected by the router before it reaches
 * the pointer adapter. It has no independent recognizer claim, but uses the
 * same transport point so every host-facing REL event is centralized.
 * canonical pointer出力はpointer adapter到達前にrouterで選別済みである。独立した
 * recognizer claimは持たないが、全ホスト向けREL eventを集約するため同じtransportを使う。 */
int travelers_pad_output_routed_pointer(const struct device *dev, uint8_t type, uint16_t code,
				 int32_t value, bool sync, const char *producer);

#endif
