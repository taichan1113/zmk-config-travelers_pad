/*
 * Contact arbitration core / 接触仲裁core.
 *
 * Accept proposals emitted for one logical input frame and resolve them
 * independently of callback order. Zephyr queues input callbacks, so
 * resolution waits until every registered primary recognizer acknowledges that frame.
 * A frame-routing callback runs only after that complete proposal set has been
 * resolved. Primary semantic output is token-gated; the session manager owns
 * only the immutable lifecycle boundary, never contact ownership.
 *
 * 一つの論理input frameに対して出たproposalを収集し、callback順に依存せず解決する。
 * Zephyr input callbackはqueue方式のため、全registered primary recognizerのframe通知を待つ。
 * その完全なproposal集合を解決後にだけframe-routing callbackを呼ぶ。primary semantic
 * outputはtoken gateし、session managerは不変lifecycle境界だけを保持して接触所有権を持たない。
 */

#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include <travelers_pad/contact_arbiter.h>

LOG_MODULE_REGISTER(travelers_pad_arbiter, LOG_LEVEL_INF);

#define TRAVELERS_PAD_ARBITER_MAX_PROPOSALS 8
#define TRAVELERS_PAD_ARBITER_MAX_IN_FLIGHT_FRAMES 16
#define TRAVELERS_PAD_ARBITER_MAX_OUTPUT_RECEIPTS 4
#define TRAVELERS_PAD_ARBITER_MAX_DECISIONS 16
#define TRAVELERS_PAD_ARBITER_MAX_TERMINAL_ACTIONS 8

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

struct terminal_action_record {
	struct travelers_pad_terminal_action_grant grant;
	bool consumed;
};

struct arbiter_frame {
	struct travelers_pad_session_snapshot snapshot;
	struct travelers_pad_arbiter_proposal proposals[TRAVELERS_PAD_ARBITER_MAX_PROPOSALS];
	struct arbiter_output_receipt output_receipts[TRAVELERS_PAD_ARBITER_MAX_OUTPUT_RECEIPTS];
	uint32_t observed_mask;
	uint8_t proposal_count;
	uint8_t output_receipt_count;
	bool active;
	/* Primary recognizers may decide the frame before asynchronous observers
	 * consume its immutable snapshot. Keep storage alive until every registered
	 * input listener has acknowledged it. / primary recognizerはobserverより先に
	 * frameを裁定できる。全登録input listenerがacknowledgeするまで不変snapshotを保持する。 */
	bool resolved;
	bool terminal_action_open;
	/* A handler must receive a resolved frame only once. Primary handlers run
	 * at primary resolution; observer handlers wait until their own raw input
	 * callback has acknowledged that frame. / handlerは裁定済みframeを一度だけ受ける。
	 * primary handlerはprimary裁定時、observer handlerは自身の生input callbackが同frameを
	 * acknowledgeしてから呼ばれる。 */
	uint32_t frame_handler_mask;
};

struct arbiter_state {
	struct arbiter_frame frames[TRAVELERS_PAD_ARBITER_MAX_IN_FLIGHT_FRAMES];
	struct arbiter_decision decisions[TRAVELERS_PAD_ARBITER_MAX_DECISIONS];
	struct terminal_action_record terminal_actions[TRAVELERS_PAD_ARBITER_MAX_TERMINAL_ACTIONS];
	struct travelers_pad_owner_token token;
	struct travelers_pad_owner_token post_session;
	struct {
		uint32_t session_id;
		enum travelers_pad_owner owner;
	} ending;
	uint32_t cancelled_session_id;
	uint32_t next_owner_generation;
	uint32_t registered_mask;
	uint32_t primary_registered_mask;
	struct {
		travelers_pad_arbiter_resolve_handler_t handler;
		void *context;
	} handlers[TRAVELERS_PAD_RECOGNIZER_COUNT];
	struct {
		travelers_pad_arbiter_frame_handler_t handler;
		void *context;
	} frame_handlers[TRAVELERS_PAD_RECOGNIZER_COUNT];
	uint8_t next_decision;
	uint8_t next_terminal_action;
	uint32_t next_terminal_action_generation;
};

static struct arbiter_state state;

static bool is_primary_recognizer(enum travelers_pad_recognizer_id recognizer)
{
	return recognizer <= TRAVELERS_PAD_RECOGNIZER_TIP_TAP;
}

static const char *owner_name(enum travelers_pad_owner owner)
{
	switch (owner) {
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
 * conflict. It is authoritative for all contact ownership and primary output.
 * この表はZephyr input callbackのリンク順ではなく方針を明示する。同一frameの
 * 競合では大きい値が勝つ。全接触所有権およびprimary出力に対して正である。 */
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
	case TRAVELERS_PAD_OWNER_NONE:
	default:
		return 0;
	}
}

static bool token_has_capabilities(const struct travelers_pad_owner_token *token,
				   enum travelers_pad_owner owner, uint32_t capability)
{
	return token != NULL && token->valid && token->owner == owner &&
	       (token->capabilities & capability) == capability;
}

static void invalidate_decisions_for_session(uint32_t session_id)
{
	for (uint8_t i = 0; i < ARRAY_SIZE(state.decisions); i++) {
		if (state.decisions[i].valid && state.decisions[i].token.session_id == session_id) {
			state.decisions[i].token.valid = false;
			state.decisions[i].valid = false;
		}
	}
}

static void revoke_session(uint32_t session_id, const char *reason)
{
	bool revoked = false;

	if (state.token.valid && state.token.session_id == session_id) {
		state.token.valid = false;
		revoked = true;
	}
	if (state.post_session.valid && state.post_session.session_id == session_id) {
		state.post_session.valid = false;
		revoked = true;
	}
	invalidate_decisions_for_session(session_id);
	if (revoked) {
		LOG_INF("arbiter session=%u leases revoked reason=%s", session_id, reason);
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
		LOG_WRN("arbiter recognizer=%u registered twice", recognizer);
		return false;
	}
	state.registered_mask |= bit;
	if (is_primary_recognizer(recognizer)) {
		state.primary_registered_mask |= bit;
		LOG_INF("arbiter primary recognizer=%u registered", recognizer);
	} else {
		LOG_INF("arbiter observer=%u registered", recognizer);
	}
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

bool travelers_pad_arbiter_bind_frame_handler(
	enum travelers_pad_recognizer_id recognizer,
	travelers_pad_arbiter_frame_handler_t handler, void *context)
{
	if (recognizer >= TRAVELERS_PAD_RECOGNIZER_COUNT || handler == NULL ||
	    (state.registered_mask & (1U << recognizer)) == 0U ||
	    state.frame_handlers[recognizer].handler != NULL) {
		LOG_WRN("frame handler rejected recognizer=%u", recognizer);
		return false;
	}
	state.frame_handlers[recognizer].handler = handler;
	state.frame_handlers[recognizer].context = context;
	LOG_INF("frame handler bound recognizer=%u", recognizer);
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

void travelers_pad_arbiter_begin_frame(const struct travelers_pad_session_snapshot *snapshot)
{
	struct arbiter_frame *frame = NULL;

	if (snapshot == NULL || snapshot->session_id == 0) {
		return;
	}
	/* Cancellation is a lifecycle boundary, not an ordinary zero-contact
	 * release. Revoke every authority for the affected session before queued
	 * recognizers observe the cancellation frame, so delayed work can never
	 * turn a reset into post-session output.
	 * cancellationは通常のzero-contact releaseではなくlifecycle境界である。
	 * queued recognizerがcancel frameを観測する前に当該sessionの全権限を失効し、
	 * 遅延workがresetをpost-session出力へ変換できないようにする。 */
	if (snapshot->cancelled) {
		state.cancelled_session_id = snapshot->session_id;
		revoke_session(snapshot->session_id, "cancel");
		state.ending.session_id = 0;
		state.ending.owner = TRAVELERS_PAD_OWNER_NONE;
	} else if (snapshot->contact_count != 0 && state.post_session.valid &&
		   state.post_session.session_id != snapshot->session_id) {
		/* Any new physical contact revokes the preceding inertial lease. */
		revoke_session(state.post_session.session_id, "new-contact");
	}
	for (uint8_t i = 0; i < ARRAY_SIZE(state.frames); i++) {
		if (!state.frames[i].active) {
			frame = &state.frames[i];
			break;
		}
	}
	if (frame == NULL) {
		LOG_ERR("arbiter frame queue overflow seq=%u", snapshot->frame_seq);
		return;
	}
	*frame = (struct arbiter_frame){
		.snapshot = *snapshot,
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
		LOG_WRN("arbiter proposal from unregistered recognizer=%u", proposal->recognizer);
		return false;
	}
	if (frame->proposal_count >= ARRAY_SIZE(frame->proposals)) {
		LOG_ERR("arbiter proposal queue overflow session=%u", frame->snapshot.session_id);
		return false;
	}
	frame->proposals[frame->proposal_count++] = *proposal;
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
			LOG_ERR("terminal output record would reject producer=%s session=%u frame=%u owner=%s caps=0x%x",
				receipt->producer, frame->snapshot.session_id, frame->snapshot.frame_seq,
				owner_name(receipt->requested_owner), receipt->capability);
		}
	}
	if (frame->output_receipt_count != 0) {
		LOG_INF("terminal output record preflight session=%u frame=%u winner=%s outputs=%u %s",
			frame->snapshot.session_id, frame->snapshot.frame_seq,
			owner_name(state.token.owner), frame->output_receipt_count,
			accepted ? "accepted" : "rejected");
	}
}

static void dispatch_frame_handler(struct arbiter_frame *frame,
				   enum travelers_pad_recognizer_id recognizer)
{
	const struct travelers_pad_owner_token *token = NULL;
	uint32_t bit;

	if (recognizer >= TRAVELERS_PAD_RECOGNIZER_COUNT) {
		return;
	}
	bit = 1U << recognizer;
	if ((frame->frame_handler_mask & bit) != 0U ||
	    state.frame_handlers[recognizer].handler == NULL) {
		return;
	}
	if (state.token.valid && state.token.session_id == frame->snapshot.session_id) {
		token = &state.token;
	}
	frame->frame_handler_mask |= bit;
	state.frame_handlers[recognizer].handler(state.frame_handlers[recognizer].context,
						  &frame->snapshot, token);
}

static void dispatch_frame_handlers(struct arbiter_frame *frame)
{
	/* Primary handlers are intentionally invoked immediately after the final
	 * same-frame decision. This makes router forwarding independent from
	 * callback order. Observers which have already consumed the raw frame can
	 * run now too; later observers are dispatched by arbiter_observed().
	 * primary handlerは同一frameの最終裁定直後に呼ぶため、router転送はcallback順に
	 * 依存しない。すでに生frameを消費済みのobserverもここで実行し、後続observerは
	 * arbiter_observed()からdispatchする。 */
	for (uint8_t recognizer = 0; recognizer < TRAVELERS_PAD_RECOGNIZER_COUNT;
	     recognizer++) {
		uint32_t bit = 1U << recognizer;

		if (is_primary_recognizer(recognizer) || (frame->observed_mask & bit) != 0U) {
			dispatch_frame_handler(frame, recognizer);
		}
	}
}

static void resolve_frame(struct arbiter_frame *frame)
{
	const struct travelers_pad_arbiter_proposal *winner;
	bool new_token = false;
	bool terminal_scroll = false;
	uint32_t terminal_session_id;

	if (frame->snapshot.cancelled ||
	    state.cancelled_session_id == frame->snapshot.session_id) {
		/* Claims arriving from an older queued frame may never resurrect an
		 * owner after this session has been cancelled. */
		revoke_session(frame->snapshot.session_id, "cancelled-frame");
		dispatch_frame_handlers(frame);
		return;
	}

	/* A new resolved session invalidates any completed session token. The
	 * asynchronous queue can hold several frames, so this must happen at
	 * resolution time rather than when the producer enqueues a frame. */
	if (state.token.valid && state.token.session_id != frame->snapshot.session_id) {
		state.token.valid = false;
	}
	winner = select_winner(frame);
	if (winner != NULL && !state.token.valid) {
		state.token = (struct travelers_pad_owner_token){
			.session_id = frame->snapshot.session_id,
			.owner_generation = ++state.next_owner_generation,
			.owner = winner->requested_owner,
			.capabilities = winner->capabilities,
			.kind = TRAVELERS_PAD_TOKEN_COMMITTED,
			.phase = winner->requested_owner == TRAVELERS_PAD_OWNER_DRAG ?
				TRAVELERS_PAD_OWNER_PHASE_RESERVED : TRAVELERS_PAD_OWNER_PHASE_ACTIVE,
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
	}

	if (new_token) {
		LOG_INF("arbiter session=%u frame=%u winner=%s recognizer=%u caps=0x%x",
			frame->snapshot.session_id, frame->snapshot.frame_seq,
			owner_name(state.token.owner), winner->recognizer, state.token.capabilities);
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
	/* Terminal grants are minted only during complete-frame delivery, while the
	 * final owner token (if any) is still live. */
	frame->terminal_action_open = frame->snapshot.contact_count == 0;
	dispatch_frame_handlers(frame);
	frame->terminal_action_open = false;

	/* A terminal frame may still carry a proposal or terminal release record.
	 * Preserve the result until validation is complete, then invalidate it so no later
	 * work can mistake it for a live-session token. */
	if (frame->snapshot.contact_count == 0) {
		terminal_session_id = frame->snapshot.session_id;
		state.ending.session_id = frame->snapshot.session_id;
		state.ending.owner = state.token.valid ? state.token.owner : TRAVELERS_PAD_OWNER_NONE;
		terminal_scroll = state.token.valid &&
			state.token.owner == TRAVELERS_PAD_OWNER_SCROLL && !frame->snapshot.cancelled;
		if (terminal_scroll) {
			state.post_session = (struct travelers_pad_owner_token){
				.session_id = terminal_session_id,
				.owner_generation = ++state.next_owner_generation,
				.owner = TRAVELERS_PAD_OWNER_SCROLL,
				.capabilities = TRAVELERS_PAD_CAP_SCROLL_DELTA,
				.kind = TRAVELERS_PAD_TOKEN_POST_SESSION,
				.phase = TRAVELERS_PAD_OWNER_PHASE_ACTIVE,
				.valid = true,
			};
			LOG_INF("arbiter post-session lease session=%u generation=%u owner=scroll",
				terminal_session_id, state.post_session.owner_generation);
		}
		state.token.valid = false;
		invalidate_decisions_for_session(terminal_session_id);
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
	if (frame->snapshot.contact_count == 0 && state.ending.session_id == session_id) {
		return state.ending.owner;
	}
	return TRAVELERS_PAD_OWNER_NONE;
}

bool travelers_pad_arbiter_get_frame_snapshot(uint32_t frame_seq,
					       struct travelers_pad_session_snapshot *snapshot)
{
	const struct arbiter_frame *frame = frame_for(frame_seq);

	if (frame == NULL || snapshot == NULL) {
		return false;
	}
	*snapshot = frame->snapshot;
	return true;
}

bool travelers_pad_arbiter_issue_terminal_action(
	uint32_t frame_seq, enum travelers_pad_recognizer_id recognizer,
	enum travelers_pad_owner owner, uint32_t capability,
	struct travelers_pad_terminal_action_grant *grant)
{
	struct arbiter_frame *frame = frame_for(frame_seq);
	struct terminal_action_record *record;
	bool owner_allowed;

	if (grant == NULL || recognizer >= TRAVELERS_PAD_RECOGNIZER_COUNT ||
	    capability == TRAVELERS_PAD_CAP_NONE || frame == NULL || !frame->resolved ||
	    !frame->terminal_action_open || frame->snapshot.cancelled ||
	    frame->snapshot.contact_count != 0) {
		return false;
	}
	/* A bare tap may be granted only if no primary recognizer owns the session.
	 * A double-tap candidate is the narrow exception: its own reserved DRAG
	 * token permits its terminal double-click but no other recognizer may use it.
	 * 通常tapはprimary ownerがいない時だけgrantする。唯一の例外は予約済みDRAGで、
	 * 自身のterminal double-clickだけを許可し、他recognizerは利用できない。 */
	if (owner == TRAVELERS_PAD_OWNER_NONE) {
		owner_allowed = !state.token.valid;
	} else {
		owner_allowed = state.token.valid && state.token.session_id == frame->snapshot.session_id &&
			state.token.owner == owner && token_has_capabilities(&state.token, owner, capability);
	}
	if (!owner_allowed) {
		LOG_WRN("terminal action denied session=%u frame=%u recognizer=%u owner=%s",
			frame->snapshot.session_id, frame_seq, recognizer, owner_name(owner));
		return false;
	}

	record = &state.terminal_actions[
		state.next_terminal_action++ % ARRAY_SIZE(state.terminal_actions)];
	*record = (struct terminal_action_record){
		.grant = {
			.session_id = frame->snapshot.session_id,
			.grant_generation = ++state.next_terminal_action_generation,
			.recognizer = recognizer,
			.owner = owner,
			.capability = capability,
			.valid = true,
		},
	};
	*grant = record->grant;
	LOG_INF("terminal action granted session=%u frame=%u generation=%u recognizer=%u owner=%s",
		grant->session_id, frame_seq, grant->grant_generation, recognizer, owner_name(owner));
	return true;
}

bool travelers_pad_arbiter_consume_terminal_action(
	struct travelers_pad_terminal_action_grant *grant,
	enum travelers_pad_recognizer_id recognizer, enum travelers_pad_owner owner,
	uint32_t capability, const char *producer)
{
	if (grant == NULL || !grant->valid || recognizer >= TRAVELERS_PAD_RECOGNIZER_COUNT ||
	    grant->recognizer != recognizer || grant->owner != owner ||
	    (grant->capability & capability) != capability) {
		return false;
	}
	for (uint8_t index = 0; index < ARRAY_SIZE(state.terminal_actions); index++) {
		struct terminal_action_record *record = &state.terminal_actions[index];

		if (!record->grant.valid || record->consumed ||
		    record->grant.grant_generation != grant->grant_generation ||
		    record->grant.session_id != grant->session_id ||
		    record->grant.recognizer != recognizer || record->grant.owner != owner ||
		    (record->grant.capability & capability) != capability) {
			continue;
		}
		record->consumed = true;
		grant->valid = false;
		LOG_INF("terminal action consumed session=%u generation=%u producer=%s",
			grant->session_id, grant->grant_generation, producer);
		return true;
	}
	return false;
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
	    !decision_allows(&decision->token, decision->recognizer, &receipt) ||
	    !state.token.valid || state.token.kind != TRAVELERS_PAD_TOKEN_COMMITTED ||
	    state.token.session_id != decision->token.session_id ||
	    state.token.owner_generation != decision->token.owner_generation) {
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
	       state.token.kind == TRAVELERS_PAD_TOKEN_COMMITTED &&
	       state.token.phase == TRAVELERS_PAD_OWNER_PHASE_ACTIVE &&
	       token_has_capabilities(&state.token, required_owner, capability);
}

bool travelers_pad_arbiter_activate_owner(uint32_t ownership_frame_seq,
					  enum travelers_pad_recognizer_id recognizer,
					  enum travelers_pad_owner requested_owner,
					  const char *producer)
{
	struct arbiter_decision *decision = decision_for(ownership_frame_seq);
	struct arbiter_output_receipt receipt = {
		.recognizer = recognizer,
		.requested_owner = requested_owner,
		.capability = TRAVELERS_PAD_CAP_BUTTON,
		.producer = producer,
	};

	if (decision == NULL || !decision_allows(&decision->token, decision->recognizer, &receipt) ||
	    !state.token.valid || state.token.kind != TRAVELERS_PAD_TOKEN_COMMITTED ||
	    state.token.session_id != decision->token.session_id ||
	    state.token.owner_generation != decision->token.owner_generation ||
	    state.token.phase != TRAVELERS_PAD_OWNER_PHASE_RESERVED) {
		LOG_ERR("arbiter activation rejected producer=%s ownership-frame=%u owner=%s",
			producer, ownership_frame_seq, owner_name(requested_owner));
		return false;
	}
	state.token.phase = TRAVELERS_PAD_OWNER_PHASE_ACTIVE;
	LOG_INF("arbiter owner active session=%u generation=%u owner=%s producer=%s",
		state.token.session_id, state.token.owner_generation, owner_name(state.token.owner), producer);
	return true;
}

bool travelers_pad_arbiter_acquire_post_session_lease(
	uint32_t session_id, enum travelers_pad_owner required_owner, uint32_t capability,
	struct travelers_pad_owner_token *lease)
{
	if (lease == NULL || !state.post_session.valid ||
	    state.post_session.kind != TRAVELERS_PAD_TOKEN_POST_SESSION ||
	    state.post_session.session_id != session_id ||
	    !token_has_capabilities(&state.post_session, required_owner, capability)) {
		return false;
	}
	*lease = state.post_session;
	return true;
}

bool travelers_pad_arbiter_authorize_post_session_output(
	const struct travelers_pad_owner_token *lease, enum travelers_pad_owner required_owner,
	uint32_t capability, const char *producer)
{
	ARG_UNUSED(producer);

	return lease != NULL && lease->valid && lease->kind == TRAVELERS_PAD_TOKEN_POST_SESSION &&
	       token_has_capabilities(lease, required_owner, capability) &&
	       state.post_session.valid && state.post_session.kind == TRAVELERS_PAD_TOKEN_POST_SESSION &&
	       state.post_session.session_id == lease->session_id &&
	       state.post_session.owner_generation == lease->owner_generation &&
	       token_has_capabilities(&state.post_session, required_owner, capability);
}

void travelers_pad_arbiter_observed(enum travelers_pad_recognizer_id recognizer, uint32_t frame_seq)
{
	struct arbiter_frame *frame = frame_for(frame_seq);

	if (!frame_accepts_recognizer(frame, recognizer)) {
		return;
	}
	frame->observed_mask |= 1U << recognizer;
	/* Observers consume the resolved lifecycle result; they never participate in
	 * selecting it. Their callback order or disabled state therefore cannot hold
	 * up pointer/scroll routing. / observerは解決済みlifecycle結果を消費するだけで
	 * 選択には参加しない。従ってobserverのcallback順や無効状態でpointer/scrollの
	 * routingを待たせない。 */
	if (!frame->resolved &&
	    (frame->observed_mask & state.primary_registered_mask) == state.primary_registered_mask) {
		frame->resolved = true;
		resolve_frame(frame);
	}
	/* An observer may be called after primary resolution, or it may have
	 * consumed its raw frame before the final primary acknowledgement. In both
	 * cases dispatch exactly once only when both facts are true.
	 * observerはprimary裁定後に呼ばれる場合と、最終primary acknowledgement前に生frameを
	 * 消費済みの場合がある。両方の条件が満たされた時だけ一度dispatchする。 */
	if (frame->resolved && !is_primary_recognizer(recognizer)) {
		dispatch_frame_handler(frame, recognizer);
	}
	/* Keep the immutable frame available to late observer callbacks. */
	if ((frame->observed_mask & state.registered_mask) == state.registered_mask) {
		frame->active = false;
	}
}

bool travelers_pad_arbiter_note_terminal_output(uint32_t ownership_frame_seq,
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
			LOG_ERR("terminal output record receipt overflow session=%u frame=%u",
				frame->snapshot.session_id, ownership_frame_seq);
			return true;
		}
		frame->output_receipts[frame->output_receipt_count++] = receipt;
		return true;
	}

	decision = decision_for(ownership_frame_seq);
	if (decision == NULL ||
	    !decision_allows(&decision->token, decision->recognizer, &receipt)) {
		LOG_ERR("terminal output record would reject producer=%s ownership-frame=%u owner=%s caps=0x%x",
			producer, ownership_frame_seq, owner_name(requested_owner), capability);
	} else if (!decision->output_verified) {
		decision->output_verified = true;
		LOG_INF("terminal output record preflight ownership-frame=%u winner=%s producer=%s accepted",
			ownership_frame_seq, owner_name(decision->token.owner), producer);
	}
	/* This is diagnostic only. Releases remain unconditional so a stale token
	 * cannot leave a host button held. / これは診断専用である。stale tokenで
	 * ホスト側のボタンが押下状態に残らないよう、releaseは常に送信する。 */
	return true;
}
