/*
 * Travelers Pad contact-arbiter vocabulary.
 *
 * This API is intentionally separate from the legacy shared-claim API. During
 * shadow migration recognizers submit immutable proposals, the resolver
 * selects a hypothetical owner, and the legacy output path remains
 * authoritative. A later stage makes the issued token mandatory at the
 * semantic output gate.
 *
 * Travelers Pad 接触arbiter用の共通語彙。
 * legacy shared-claim APIとは意図的に分離する。shadow移行中はrecognizerが
 * 不変proposalを提出し、resolverが仮のownerを選ぶが、既存出力経路を正とする。
 * 後段で発行tokenをsemantic output gateの必須条件にする。
 */
#ifndef TRAVELERS_PAD_CONTACT_ARBITER_H_
#define TRAVELERS_PAD_CONTACT_ARBITER_H_

#include <stdbool.h>
#include <stdint.h>

#include <travelers_pad/contact_session.h>

enum travelers_pad_recognizer_id {
	TRAVELERS_PAD_RECOGNIZER_ROUTER,
	TRAVELERS_PAD_RECOGNIZER_TAP_DRAG,
	TRAVELERS_PAD_RECOGNIZER_MULTIFINGER_GESTURE,
	TRAVELERS_PAD_RECOGNIZER_EDGE_DIAL,
	TRAVELERS_PAD_RECOGNIZER_TIP_TAP,
	TRAVELERS_PAD_RECOGNIZER_COUNT,
};

enum travelers_pad_proposal_intent {
	/* A recognizer is tracking a possible action but requests no ownership. */
	TRAVELERS_PAD_PROPOSAL_CANDIDATE,
	/* The recognizer requests exclusive session ownership. */
	TRAVELERS_PAD_PROPOSAL_CLAIM,
};

enum travelers_pad_output_capability {
	TRAVELERS_PAD_CAP_NONE = 0,
	TRAVELERS_PAD_CAP_POINTER_DELTA = (1U << 0),
	TRAVELERS_PAD_CAP_SCROLL_DELTA = (1U << 1),
	TRAVELERS_PAD_CAP_BUTTON = (1U << 2),
	TRAVELERS_PAD_CAP_BEHAVIOR_ACTION = (1U << 3),
};

enum travelers_pad_token_kind {
	TRAVELERS_PAD_TOKEN_NONE,
	TRAVELERS_PAD_TOKEN_PROVISIONAL,
	TRAVELERS_PAD_TOKEN_COMMITTED,
	TRAVELERS_PAD_TOKEN_POST_SESSION,
};

/* Evidence is recognizer-local and only used for trace diagnostics in the
 * shadow phase. It must never affect callback ordering. */
struct travelers_pad_arbiter_proposal {
	enum travelers_pad_recognizer_id recognizer;
	enum travelers_pad_proposal_intent intent;
	enum travelers_pad_owner requested_owner;
	uint32_t capabilities;
	uint8_t contact_count;
	uint32_t evidence;
};

struct travelers_pad_owner_token {
	uint32_t session_id;
	uint32_t owner_generation;
	enum travelers_pad_owner owner;
	uint32_t capabilities;
	enum travelers_pad_token_kind kind;
	bool valid;
};

/* Recognizers register from their DT-backed init function. Therefore a node
 * with status = "disabled" has no registry entry and cannot alter ordering.
 * recognizerはDT-backed initから登録する。従ってstatus = "disabled"のnodeは
 * registryへ入らず、他機能の順序を変えない。 */
bool travelers_pad_arbiter_register(enum travelers_pad_recognizer_id recognizer);

/* Called by the session manager around one synchronously forwarded frame. */
void travelers_pad_arbiter_begin_frame(const struct travelers_pad_session_snapshot *snapshot);
bool travelers_pad_arbiter_submit(const struct travelers_pad_arbiter_proposal *proposal);
void travelers_pad_arbiter_resolve_frame(enum travelers_pad_owner legacy_owner);

/* The semantic gate is observational until the authoritative migration. It
 * reports invalid tokens but deliberately permits output in this phase.
 * semantic gateはauthoritative移行まで観測専用である。不正tokenを記録するが、
 * 現段階では意図的に出力を許可する。 */
bool travelers_pad_arbiter_shadow_gate(const char *producer,
					const struct travelers_pad_owner_token *token,
					uint32_t capability);
const struct travelers_pad_owner_token *travelers_pad_arbiter_shadow_token(void);

#endif
