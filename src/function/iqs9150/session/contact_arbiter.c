/*
 * Contact arbitration core / 接触仲裁core.
 *
 * Shadow stage: accept proposals emitted while the session manager forwards a
 * single synchronous input frame, resolve them independently of callback
 * order, and compare the result with the legacy owner. No semantic output is
 * gated yet, so this file cannot change the established device UX.
 *
 * shadow段階: session managerが一つの同期input frameをforwardする間に出た
 * proposalを収集し、callback順に依存せず解決してlegacy ownerと比較する。まだ
 * semantic outputをgateしないため、本ファイルは確立済みのUXを変更しない。
 */

#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include <travelers_pad/contact_arbiter.h>

LOG_MODULE_REGISTER(travelers_pad_arbiter, LOG_LEVEL_INF);

#define TRAVELERS_PAD_ARBITER_MAX_PROPOSALS 8

struct arbiter_state {
	struct travelers_pad_session_snapshot frame;
	struct travelers_pad_arbiter_proposal proposals[TRAVELERS_PAD_ARBITER_MAX_PROPOSALS];
	struct travelers_pad_owner_token token;
	uint32_t registered_mask;
	uint8_t proposal_count;
	bool dispatching;
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
 * conflict. It is shadow-only until all five primary recognizers migrate.
 * この表はZephyr input callbackのリンク順ではなく方針を明示する。同一frameの
 * 競合では大きい値が勝つ。primary recognizer全移行まではshadow専用である。 */
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

void travelers_pad_arbiter_begin_frame(const struct travelers_pad_session_snapshot *snapshot)
{
	if (snapshot == NULL) {
		return;
	}
	state.frame = *snapshot;
	state.proposal_count = 0;
	state.dispatching = snapshot->session_id != 0 && !snapshot->cancelled;

	/* A new contact session invalidates every old shadow token before any
	 * downstream recognizer sees its first frame. */
	if (state.token.valid && state.token.session_id != snapshot->session_id) {
		state.token.valid = false;
	}
}

bool travelers_pad_arbiter_submit(const struct travelers_pad_arbiter_proposal *proposal)
{
	uint32_t bit;

	if (proposal == NULL || !state.dispatching ||
	    proposal->recognizer >= TRAVELERS_PAD_RECOGNIZER_COUNT ||
	    proposal->requested_owner == TRAVELERS_PAD_OWNER_NONE) {
		return false;
	}
	bit = 1U << proposal->recognizer;
	if ((state.registered_mask & bit) == 0U) {
		LOG_WRN("shadow proposal from unregistered recognizer=%u", proposal->recognizer);
		return false;
	}
	if (state.proposal_count >= ARRAY_SIZE(state.proposals)) {
		LOG_ERR("shadow proposal queue overflow session=%u", state.frame.session_id);
		return false;
	}
	state.proposals[state.proposal_count++] = *proposal;
	return true;
}

static const struct travelers_pad_arbiter_proposal *select_winner(void)
{
	const struct travelers_pad_arbiter_proposal *winner = NULL;

	for (uint8_t i = 0; i < state.proposal_count; i++) {
		const struct travelers_pad_arbiter_proposal *proposal = &state.proposals[i];

		if (proposal->intent != TRAVELERS_PAD_PROPOSAL_CLAIM) {
			continue;
		}
		if (winner == NULL || owner_priority(proposal->requested_owner) >
				      owner_priority(winner->requested_owner) ||
		    (owner_priority(proposal->requested_owner) ==
			     owner_priority(winner->requested_owner) &&
		     proposal->recognizer < winner->recognizer)) {
			/* The final recognizer-ID comparison is deterministic only. Primary
			 * claims are intentionally assigned distinct policy priorities. */
			winner = proposal;
		}
	}
	return winner;
}

void travelers_pad_arbiter_resolve_frame(enum travelers_pad_owner legacy_owner)
{
	const struct travelers_pad_arbiter_proposal *winner;
	bool new_token = false;

	if (!state.dispatching) {
		return;
	}
	winner = select_winner();
	if (winner != NULL && !state.token.valid) {
		state.token = (struct travelers_pad_owner_token){
			.session_id = state.frame.session_id,
			.owner_generation = 1,
			.owner = winner->requested_owner,
			.capabilities = winner->capabilities,
			.kind = TRAVELERS_PAD_TOKEN_COMMITTED,
			.valid = true,
		};
		new_token = true;
	} else if (state.token.valid && state.token.session_id != state.frame.session_id) {
		/* Defensive fallback: begin_frame normally invalidates this path. */
		state.token.valid = false;
	}

	if (new_token) {
		LOG_INF("shadow session=%u frame=%u winner=%s recognizer=%u caps=0x%x",
			state.frame.session_id, state.frame.frame_seq,
			owner_name(state.token.owner), winner->recognizer, state.token.capabilities);
		if (legacy_owner != state.token.owner) {
			LOG_INF("shadow session=%u differs: legacy=%s shadow=%s",
				state.frame.session_id, owner_name(legacy_owner),
				owner_name(state.token.owner));
		}
	}

	/* A terminal frame may still carry a proposal or legacy action. Preserve the
	 * result until this comparison is complete, then invalidate it so no later
	 * work can mistake it for a live-session token. */
	if (state.frame.contact_count == 0) {
		state.token.valid = false;
	}
	state.dispatching = false;
}

bool travelers_pad_arbiter_shadow_gate(const char *producer,
					const struct travelers_pad_owner_token *token,
					uint32_t capability)
{
	bool valid = token != NULL && token->valid && token->session_id == state.frame.session_id &&
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
