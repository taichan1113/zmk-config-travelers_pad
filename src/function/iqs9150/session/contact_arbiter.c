/*
 * Contact arbitration core / 接触仲裁core.
 *
 * Hybrid stage: accept proposals emitted for one logical input frame, resolve
 * them independently of callback order, and compare the result with the
 * legacy owner. Zephyr queues input callbacks, so resolution waits until every
 * registered recognizer acknowledges that frame. Primary semantic output is
 * token-gated, while the session manager still retains legacy shared claims.
 *
 * hybrid段階: 一つの論理input frameに対して出たproposalを収集し、callback順に
 * 依存せず解決してlegacy ownerと比較する。Zephyr input callbackはqueue方式のため、
 * 全registered recognizerのframe通知を待つ。primary semantic outputはtoken gateし、
 * session managerのshared claimはまだ旧経路として残す。
 */

#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include <travelers_pad/contact_arbiter.h>

LOG_MODULE_REGISTER(travelers_pad_arbiter, LOG_LEVEL_INF);

#define TRAVELERS_PAD_ARBITER_MAX_PROPOSALS 8
#define TRAVELERS_PAD_ARBITER_MAX_IN_FLIGHT_FRAMES 16
#define TRAVELERS_PAD_ARBITER_MAX_OUTPUT_RECEIPTS 4
#define TRAVELERS_PAD_ARBITER_MAX_DECISIONS 16

struct arbiter_output_receipt {
	enum travelers_pad_recognizer_id recognizer;
	enum travelers_pad_owner requested_owner;
	uint32_t capability;
	const char *producer;
};

struct arbiter_decision {
	struct travelers_pad_owner_token token;
	uint32_t frame_seq;
	enum travelers_pad_recognizer_id recognizer;
	bool valid;
	bool output_verified;
	bool authoritative_output_verified;
};

struct arbiter_frame {
	struct travelers_pad_session_snapshot snapshot;
	struct travelers_pad_arbiter_proposal proposals[TRAVELERS_PAD_ARBITER_MAX_PROPOSALS];
	struct arbiter_output_receipt output_receipts[TRAVELERS_PAD_ARBITER_MAX_OUTPUT_RECEIPTS];
	uint32_t observed_mask;
	enum travelers_pad_owner legacy_owner;
	uint8_t proposal_count;
	uint8_t output_receipt_count;
	bool active;
};

/* A claim must be visible before the queued listener fan-out has completed:
 * the router uses it only to suppress an incompatible raw stream. It is not a
 * grant and cannot authorize semantic output. The resolver replaces it with a
 * committed token after every recognizer has observed the frame.
 * claimはqueued listener fan-out完了前にも見える必要がある。routerはこれを
 * 非互換な生ストリームの抑止だけに使う。これはgrantではなくsemantic出力を許可
 * できない。全recognizerがframeを観測後、resolverが確定tokenへ置換する。 */
struct arbiter_provisional_owner {
	struct travelers_pad_owner_token token;
	enum travelers_pad_recognizer_id recognizer;
};

struct arbiter_state {
	struct arbiter_frame frames[TRAVELERS_PAD_ARBITER_MAX_IN_FLIGHT_FRAMES];
	struct arbiter_decision decisions[TRAVELERS_PAD_ARBITER_MAX_DECISIONS];
	struct travelers_pad_owner_token token;
	struct arbiter_provisional_owner provisional;
	struct {
		uint32_t session_id;
		enum travelers_pad_owner owner;
	} ending;
	uint32_t registered_mask;
	struct {
		travelers_pad_arbiter_resolve_handler_t handler;
		void *context;
	} handlers[TRAVELERS_PAD_RECOGNIZER_COUNT];
	uint8_t next_decision;
};

static struct arbiter_state state;

static const char *owner_name(enum travelers_pad_owner owner)
{
	switch (owner) {
	case TRAVELERS_PAD_OWNER_POINTER_PROVISIONAL:
		return "pointer";
	case TRAVELERS_PAD_OWNER_DRAG:
		return "drag";
	case TRAVELERS_PAD_OWNER_SCROLL:
		return "scroll";
	case TRAVELERS_PAD_OWNER_MULTI_GESTURE:
		return "gesture";
	case TRAVELERS_PAD_OWNER_EDGE_DIAL:
		return "edge-dial";
	case TRAVELERS_PAD_OWNER_TIP_TAP:
		return "tip-tap";
	case TRAVELERS_PAD_OWNER_NONE:
	default:
		return "none";
	}
}

/* This table declares policy, rather than accidentally inheriting the order in
 * which Zephyr input callbacks were linked. Higher values win a same-frame
 * conflict. It is authoritative for primary token-gated output while legacy
 * session ownership is migrated separately.
 * この表はZephyr input callbackのリンク順ではなく方針を明示する。同一frameの
 * 競合では大きい値が勝つ。primary出力に対しては正であり、legacy session所有権は
 * 別途移行する。 */
static uint8_t owner_priority(enum travelers_pad_owner owner)
{
	switch (owner) {
	case TRAVELERS_PAD_OWNER_EDGE_DIAL:
		return 100;
	case TRAVELERS_PAD_OWNER_DRAG:
		return 90;
	case TRAVELERS_PAD_OWNER_MULTI_GESTURE:
		return 80;
	case TRAVELERS_PAD_OWNER_SCROLL:
		return 70;
	case TRAVELERS_PAD_OWNER_TIP_TAP:
		return 60;
	case TRAVELERS_PAD_OWNER_POINTER_PROVISIONAL:
		return 10;
	case TRAVELERS_PAD_OWNER_NONE:
	default:
		return 0;
	}
}

bool travelers_pad_arbiter_register(enum travelers_pad_recognizer_id recognizer)
{
	uint32_t bit;

	if (recognizer >= TRAVELERS_PAD_RECOGNIZER_COUNT) {
		return false;
	}
	bit = 1U << recognizer;
	if (state.registered_mask & bit) {
		LOG_WRN("shadow recognizer=%u registered twice", recognizer);
		return false;
	}
	state.registered_mask |= bit;
	LOG_INF("shadow recognizer=%u registered", recognizer);
	return true;
}

bool travelers_pad_arbiter_bind_resolve_handler(
	enum travelers_pad_recognizer_id recognizer,
	travelers_pad_arbiter_resolve_handler_t handler, void *context)
{
	if (recognizer >= TRAVELERS_PAD_RECOGNIZER_COUNT || handler == NULL ||
	    (state.registered_mask & (1U << recognizer)) == 0U ||
	    state.handlers[recognizer].handler != NULL) {
		LOG_WRN("resolver handler rejected recognizer=%u", recognizer);
		return false;
	}
	state.handlers[recognizer].handler = handler;
	state.handlers[recognizer].context = context;
	LOG_INF("resolver handler bound recognizer=%u", recognizer);
	return true;
}

static struct arbiter_frame *frame_for(uint32_t frame_seq)
{
	for (uint8_t i = 0; i < ARRAY_SIZE(state.frames); i++) {
		if (state.frames[i].active && state.frames[i].snapshot.frame_seq == frame_seq) {
			return &state.frames[i];
		}
	}
	return NULL;
}

static struct arbiter_decision *decision_for(uint32_t frame_seq)
{
	for (uint8_t i = 0; i < ARRAY_SIZE(state.decisions); i++) {
		if (state.decisions[i].valid && state.decisions[i].frame_seq == frame_seq) {
			return &state.decisions[i];
		}
	}
	return NULL;
}

static bool proposal_wins_over(enum travelers_pad_owner candidate_owner,
			       enum travelers_pad_recognizer_id candidate_recognizer,
			       enum travelers_pad_owner current_owner,
			       enum travelers_pad_recognizer_id current_recognizer)
{
	return owner_priority(candidate_owner) > owner_priority(current_owner) ||
	       (owner_priority(candidate_owner) == owner_priority(current_owner) &&
		candidate_recognizer < current_recognizer);
}

static void note_provisional_claim(const struct arbiter_frame *frame,
				   const struct travelers_pad_arbiter_proposal *proposal)
{
	struct arbiter_provisional_owner *provisional = &state.provisional;

	if (proposal->intent != TRAVELERS_PAD_PROPOSAL_CLAIM ||
	    (state.token.valid && state.token.session_id == frame->snapshot.session_id)) {
		return;
	}
	if (provisional->token.valid && provisional->token.session_id == frame->snapshot.session_id &&
	    !proposal_wins_over(proposal->requested_owner, proposal->recognizer,
				provisional->token.owner, provisional->recognizer)) {
		return;
	}
	*provisional = (struct arbiter_provisional_owner){
		.token = {
			.session_id = frame->snapshot.session_id,
			.owner_generation = 0,
			.owner = proposal->requested_owner,
			.capabilities = proposal->capabilities,
			.kind = TRAVELERS_PAD_TOKEN_PROVISIONAL,
			.valid = true,
		},
		.recognizer = proposal->recognizer,
	};
	LOG_INF("arbiter provisional session=%u frame=%u owner=%s recognizer=%u",
		frame->snapshot.session_id, frame->snapshot.frame_seq,
		owner_name(proposal->requested_owner), proposal->recognizer);
}

void travelers_pad_arbiter_begin_frame(const struct travelers_pad_session_snapshot *snapshot)
{
	struct arbiter_frame *frame = NULL;

	if (snapshot == NULL || snapshot->session_id == 0) {
		return;
	}
	for (uint8_t i = 0; i < ARRAY_SIZE(state.frames); i++) {
		if (!state.frames[i].active) {
			frame = &state.frames[i];
			break;
		}
	}
	if (frame == NULL) {
		LOG_ERR("shadow frame queue overflow seq=%u", snapshot->frame_seq);
		return;
	}
	*frame = (struct arbiter_frame){
		.snapshot = *snapshot,
		.legacy_owner = snapshot->owner,
		.active = true,
	};
}

static bool frame_accepts_recognizer(const struct arbiter_frame *frame,
					  enum travelers_pad_recognizer_id recognizer)
{
	return frame != NULL && (state.registered_mask & (1U << recognizer)) != 0U;
}

uint32_t travelers_pad_arbiter_next_frame(enum travelers_pad_recognizer_id recognizer)
{
	const struct arbiter_frame *next = NULL;
	uint32_t bit;

	if (recognizer >= TRAVELERS_PAD_RECOGNIZER_COUNT) {
		return 0;
	}
	bit = 1U << recognizer;
	if ((state.registered_mask & bit) == 0U) {
		return 0;
	}
	for (uint8_t i = 0; i < ARRAY_SIZE(state.frames); i++) {
		const struct arbiter_frame *frame = &state.frames[i];

		if (!frame->active || (frame->observed_mask & bit) != 0U) {
			continue;
		}
		if (next == NULL || frame->snapshot.frame_seq < next->snapshot.frame_seq) {
			next = frame;
		}
	}
	return next == NULL ? 0 : next->snapshot.frame_seq;
}

bool travelers_pad_arbiter_submit(const struct travelers_pad_arbiter_proposal *proposal)
{
	struct arbiter_frame *frame;

	if (proposal == NULL ||
	    proposal->recognizer >= TRAVELERS_PAD_RECOGNIZER_COUNT ||
	    proposal->requested_owner == TRAVELERS_PAD_OWNER_NONE) {
		return false;
	}
	frame = frame_for(proposal->frame_seq);
	if (!frame_accepts_recognizer(frame, proposal->recognizer)) {
		LOG_WRN("shadow proposal from unregistered recognizer=%u", proposal->recognizer);
		return false;
	}
	if (frame->proposal_count >= ARRAY_SIZE(frame->proposals)) {
		LOG_ERR("shadow proposal queue overflow session=%u", frame->snapshot.session_id);
		return false;
	}
	frame->proposals[frame->proposal_count++] = *proposal;
	note_provisional_claim(frame, proposal);
	return true;
}

static const struct travelers_pad_arbiter_proposal *select_winner(const struct arbiter_frame *frame)
{
	const struct travelers_pad_arbiter_proposal *winner = NULL;

	for (uint8_t i = 0; i < frame->proposal_count; i++) {
		const struct travelers_pad_arbiter_proposal *proposal = &frame->proposals[i];

		if (proposal->intent != TRAVELERS_PAD_PROPOSAL_CLAIM) {
			continue;
		}
		if (winner == NULL || proposal_wins_over(proposal->requested_owner,
							 proposal->recognizer,
							 winner->requested_owner,
							 winner->recognizer)) {
			/* The final recognizer-ID comparison is deterministic only. Primary
			 * claims are intentionally assigned distinct policy priorities. */
			winner = proposal;
		}
	}
	return winner;
}

static bool decision_allows(const struct travelers_pad_owner_token *token,
				    enum travelers_pad_recognizer_id winner,
				    const struct arbiter_output_receipt *receipt)
{
	return token != NULL && token->valid && winner == receipt->recognizer &&
	       token->owner == receipt->requested_owner &&
	       (token->capabilities & receipt->capability) == receipt->capability;
}

static void validate_receipts(const struct arbiter_frame *frame,
			      enum travelers_pad_recognizer_id winner)
{
	bool accepted = true;

	for (uint8_t i = 0; i < frame->output_receipt_count; i++) {
		const struct arbiter_output_receipt *receipt = &frame->output_receipts[i];

		if (!decision_allows(&state.token, winner, receipt)) {
			accepted = false;
			LOG_ERR("shadow output would reject producer=%s session=%u frame=%u owner=%s caps=0x%x",
				receipt->producer, frame->snapshot.session_id, frame->snapshot.frame_seq,
				owner_name(receipt->requested_owner), receipt->capability);
		}
	}
	if (frame->output_receipt_count != 0) {
		LOG_INF("shadow output preflight session=%u frame=%u winner=%s outputs=%u %s",
			frame->snapshot.session_id, frame->snapshot.frame_seq,
			owner_name(state.token.owner), frame->output_receipt_count,
			accepted ? "accepted" : "rejected");
	}
}

static void resolve_frame(struct arbiter_frame *frame)
{
	const struct travelers_pad_arbiter_proposal *winner;
	bool new_token = false;

	/* A new resolved session invalidates any completed session token. The
	 * asynchronous queue can hold several frames, so this must happen at
	 * resolution time rather than when the producer enqueues a frame. */
	if (state.token.valid && state.token.session_id != frame->snapshot.session_id) {
		state.token.valid = false;
	}
	if (state.provisional.token.valid &&
	    state.provisional.token.session_id != frame->snapshot.session_id) {
		state.provisional.token.valid = false;
	}
	winner = select_winner(frame);
	if (winner != NULL && !state.token.valid) {
		state.token = (struct travelers_pad_owner_token){
			.session_id = frame->snapshot.session_id,
			.owner_generation = 1,
			.owner = winner->requested_owner,
			.capabilities = winner->capabilities,
			.kind = TRAVELERS_PAD_TOKEN_COMMITTED,
			.valid = true,
		};
		state.decisions[state.next_decision++ % ARRAY_SIZE(state.decisions)] =
			(struct arbiter_decision){
				.token = state.token,
				.frame_seq = frame->snapshot.frame_seq,
				.recognizer = winner->recognizer,
				.valid = true,
			};
		new_token = true;
		if (state.provisional.token.valid &&
		    state.provisional.token.session_id == frame->snapshot.session_id) {
			state.provisional.token.valid = false;
		}
	}

	if (new_token) {
		LOG_INF("shadow session=%u frame=%u winner=%s recognizer=%u caps=0x%x",
			frame->snapshot.session_id, frame->snapshot.frame_seq,
			owner_name(state.token.owner), winner->recognizer, state.token.capabilities);
		if (frame->legacy_owner != state.token.owner) {
			LOG_INF("shadow session=%u differs: legacy=%s shadow=%s",
				frame->snapshot.session_id, owner_name(frame->legacy_owner),
				owner_name(state.token.owner));
		}
		/* A first semantic action was staged by the winning recognizer while this
		 * frame was unresolved. Dispatch it only after the immutable decision is
		 * in the ledger. / 未解決frame中に勝者recognizerが保留した初回semantic
		 * actionを、不変decisionをledgerへ記録した後だけdispatchする。 */
		if (state.handlers[winner->recognizer].handler != NULL) {
			state.handlers[winner->recognizer].handler(
				state.handlers[winner->recognizer].context, &state.token,
				frame->snapshot.frame_seq);
		}
	}
	if (frame->output_receipt_count != 0) {
		validate_receipts(frame, winner == NULL ? TRAVELERS_PAD_RECOGNIZER_COUNT :
						       winner->recognizer);
	}

	/* A terminal frame may still carry a proposal or legacy action. Preserve the
	 * result until this comparison is complete, then invalidate it so no later
	 * work can mistake it for a live-session token. */
	if (frame->snapshot.contact_count == 0) {
		state.ending.session_id = frame->snapshot.session_id;
		state.ending.owner = state.token.valid ? state.token.owner :
			(state.provisional.token.valid ? state.provisional.token.owner :
			 TRAVELERS_PAD_OWNER_NONE);
		state.token.valid = false;
		state.provisional.token.valid = false;
	}
}

enum travelers_pad_owner travelers_pad_arbiter_owner_for_frame(uint32_t frame_seq)
{
	const struct arbiter_frame *frame = frame_for(frame_seq);
	uint32_t session_id;

	if (frame == NULL) {
		return TRAVELERS_PAD_OWNER_NONE;
	}
	session_id = frame->snapshot.session_id;
	if (state.token.valid && state.token.session_id == session_id) {
		return state.token.owner;
	}
	if (state.provisional.token.valid && state.provisional.token.session_id == session_id) {
		return state.provisional.token.owner;
	}
	if (frame->snapshot.contact_count == 0 && state.ending.session_id == session_id) {
		return state.ending.owner;
	}
	return TRAVELERS_PAD_OWNER_NONE;
}

bool travelers_pad_arbiter_authorize_output(uint32_t ownership_frame_seq,
					    enum travelers_pad_recognizer_id recognizer,
					    enum travelers_pad_owner requested_owner,
					    uint32_t capability, const char *producer)
{
	struct arbiter_decision *decision = decision_for(ownership_frame_seq);
	struct arbiter_output_receipt receipt = {
		.recognizer = recognizer,
		.requested_owner = requested_owner,
		.capability = capability,
		.producer = producer,
	};

	if (decision == NULL ||
	    !decision_allows(&decision->token, decision->recognizer, &receipt)) {
		LOG_ERR("arbiter output rejected producer=%s ownership-frame=%u owner=%s caps=0x%x",
			producer, ownership_frame_seq, owner_name(requested_owner), capability);
		return false;
	}
	if (!decision->authoritative_output_verified) {
		decision->authoritative_output_verified = true;
		LOG_INF("arbiter output authorized ownership-frame=%u winner=%s producer=%s",
			ownership_frame_seq, owner_name(decision->token.owner), producer);
	}
	return true;
}

bool travelers_pad_arbiter_authorize_auxiliary_output(
	enum travelers_pad_auxiliary_id auxiliary, enum travelers_pad_owner required_owner,
	uint32_t capability, const char *producer)
{
	ARG_UNUSED(producer);

	/* Auxiliary output has no independent owner. The only authorization is the
	 * live primary token, so a delayed timer cannot continue after release,
	 * cancellation, or a higher-priority ownership hand-off. */
	return auxiliary < TRAVELERS_PAD_AUXILIARY_COUNT && state.token.valid &&
	       state.token.owner == required_owner &&
	       (state.token.capabilities & capability) == capability;
}

void travelers_pad_arbiter_observed(enum travelers_pad_recognizer_id recognizer, uint32_t frame_seq)
{
	struct arbiter_frame *frame = frame_for(frame_seq);

	if (!frame_accepts_recognizer(frame, recognizer)) {
		return;
	}
	frame->observed_mask |= 1U << recognizer;
	if ((frame->observed_mask & state.registered_mask) != state.registered_mask) {
		return;
	}
	resolve_frame(frame);
	frame->active = false;
}

void travelers_pad_arbiter_record_legacy_claim(uint32_t frame_seq,
						enum travelers_pad_owner legacy_owner)
{
	struct arbiter_frame *frame = frame_for(frame_seq);

	if (frame != NULL) {
		frame->legacy_owner = legacy_owner;
	}
}

bool travelers_pad_arbiter_shadow_output(uint32_t ownership_frame_seq,
					 enum travelers_pad_recognizer_id recognizer,
					 enum travelers_pad_owner requested_owner,
					 uint32_t capability,
					 const char *producer)
{
	struct arbiter_frame *frame = frame_for(ownership_frame_seq);
	struct arbiter_output_receipt receipt = {
		.recognizer = recognizer,
		.requested_owner = requested_owner,
		.capability = capability,
		.producer = producer,
	};
	struct arbiter_decision *decision;

	if (frame != NULL) {
		if (frame->output_receipt_count >= ARRAY_SIZE(frame->output_receipts)) {
			LOG_ERR("shadow output receipt overflow session=%u frame=%u",
				frame->snapshot.session_id, ownership_frame_seq);
			return true;
		}
		frame->output_receipts[frame->output_receipt_count++] = receipt;
		return true;
	}

	decision = decision_for(ownership_frame_seq);
	if (decision == NULL ||
	    !decision_allows(&decision->token, decision->recognizer, &receipt)) {
		LOG_ERR("shadow output would reject producer=%s ownership-frame=%u owner=%s caps=0x%x",
			producer, ownership_frame_seq, owner_name(requested_owner), capability);
	} else if (!decision->output_verified) {
		decision->output_verified = true;
		LOG_INF("shadow output preflight ownership-frame=%u winner=%s producer=%s accepted",
			ownership_frame_seq, owner_name(decision->token.owner), producer);
	}
	/* Preflight only: output remains unchanged until all verified gates switch
	 * together to authoritative mode. / 事前監査のみ: 検証済みgateを一括で
	 * authoritative modeへ移すまでは出力を変更しない。 */
	return true;
}

bool travelers_pad_arbiter_shadow_gate(const char *producer,
					const struct travelers_pad_owner_token *token,
					uint32_t capability)
{
	bool valid = token != NULL && token->valid &&
		     (token->capabilities & capability) == capability;

	if (!valid) {
		LOG_WRN("shadow gate would reject producer=%s capability=0x%x", producer, capability);
	}
	/* Shadow mode intentionally never changes established output. */
	return true;
}

const struct travelers_pad_owner_token *travelers_pad_arbiter_shadow_token(void)
{
	return state.token.valid ? &state.token : NULL;
}
