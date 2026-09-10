/*
 * Travelers Pad contact-arbiter vocabulary.
 *
 * Recognizers submit immutable proposals and the resolver selects the only
 * contact owner. Primary semantic output requires the issued token, while
 * auxiliary continuation output is delegated explicitly by that live token.
 *
 * Travelers Pad 接触arbiter用の共通語彙。
 * recognizerが不変proposalを提出し、resolverが唯一のcontact ownerを選ぶ。
 * primary semantic出力には発行tokenを必須とし、補助継続出力もlive tokenから
 * 明示的に委譲する。
 */
#ifndef TRAVELERS_PAD_CONTACT_ARBITER_H_
#define TRAVELERS_PAD_CONTACT_ARBITER_H_

#include <stdbool.h>
#include <stdint.h>

#include <travelers_pad/contact_session.h>

enum travelers_pad_owner {
	TRAVELERS_PAD_OWNER_NONE,
	TRAVELERS_PAD_OWNER_POINTER_PROVISIONAL,
	TRAVELERS_PAD_OWNER_DRAG,
	TRAVELERS_PAD_OWNER_SCROLL,
	TRAVELERS_PAD_OWNER_MULTI_GESTURE,
	TRAVELERS_PAD_OWNER_EDGE_DIAL,
	TRAVELERS_PAD_OWNER_TIP_TAP,
};

enum travelers_pad_recognizer_id {
	TRAVELERS_PAD_RECOGNIZER_ROUTER,
	TRAVELERS_PAD_RECOGNIZER_TAP_DRAG,
	TRAVELERS_PAD_RECOGNIZER_MULTIFINGER_GESTURE,
	TRAVELERS_PAD_RECOGNIZER_EDGE_DIAL,
	TRAVELERS_PAD_RECOGNIZER_TIP_TAP,
	/* Auxiliary observer: proposes no owner and receives frames only so
	 * inertia can associate a physical release with the arbiter token. The
	 * preceding five IDs remain the primary recognizers. */
	TRAVELERS_PAD_RECOGNIZER_SCROLL_INERTIA,
	/* Auxiliary observer: detects edge-hold continuation candidates from raw
	 * contacts. It has no claim of its own; delayed output is delegated by the
	 * current drag or scroll owner token. */
	TRAVELERS_PAD_RECOGNIZER_EDGE_HOLD,
	TRAVELERS_PAD_RECOGNIZER_COUNT,
};

enum travelers_pad_auxiliary_id {
	TRAVELERS_PAD_AUXILIARY_EDGE_HOLD,
	TRAVELERS_PAD_AUXILIARY_COUNT,
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

/* Evidence is recognizer-local trace data only. It must never affect callback
 * ordering or the resolver policy. */
struct travelers_pad_arbiter_proposal {
	uint32_t frame_seq;
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

/* A recognizer can defer the semantic action which first establishes its
 * ownership until the resolver has issued a token.  This is needed because
 * the five input listeners run asynchronously: at proposal time the winning
 * token does not exist yet.  The callback runs exactly once for a newly
 * committed token, before a terminal frame invalidates it.
 * recognizerは、所有権を最初に確立するsemantic actionを、resolverがtokenを
 * 発行するまで保留できる。5個のinput listenerは非同期なのでproposal時点には
 * 勝者tokenがまだ存在しないためである。callbackは新規commit tokenごとに一度、
 * terminal frameがtokenを無効化する前に実行される。 */
typedef void (*travelers_pad_arbiter_resolve_handler_t)(
	void *context, const struct travelers_pad_owner_token *token, uint32_t ownership_frame_seq);

/* Recognizers register from their DT-backed init function. Therefore a node
 * with status = "disabled" has no registry entry and cannot alter ordering.
 * recognizerはDT-backed initから登録する。従ってstatus = "disabled"のnodeは
 * registryへ入らず、他機能の順序を変えない。 */
bool travelers_pad_arbiter_register(enum travelers_pad_recognizer_id recognizer);
bool travelers_pad_arbiter_bind_resolve_handler(
	enum travelers_pad_recognizer_id recognizer,
	travelers_pad_arbiter_resolve_handler_t handler, void *context);

/* Called by the session manager around one synchronously forwarded frame. */
void travelers_pad_arbiter_begin_frame(const struct travelers_pad_session_snapshot *snapshot);
/* Each recognizer receives session-manager frames in FIFO order. Obtain its
 * earliest unacknowledged frame immediately before processing an input sync.
 * 各recognizerはsession manager由来frameをFIFO順で受ける。input syncを処理する
 * 直前に、最も古い未通知frameを得る。 */
uint32_t travelers_pad_arbiter_next_frame(enum travelers_pad_recognizer_id recognizer);
bool travelers_pad_arbiter_submit(const struct travelers_pad_arbiter_proposal *proposal);
void travelers_pad_arbiter_observed(enum travelers_pad_recognizer_id recognizer, uint32_t frame_seq);
/* Return the arbiter owner applicable while processing one forwarded frame.
 * A CLAIM is visible as a provisional owner before all queued recognizers
 * have acknowledged the frame. Once resolved it becomes the committed owner;
 * the same owner remains visible for that frame's terminal release. This lets
 * a routing consumer suppress an incompatible raw path without importing a
 * recognizer-private mutable flag.
 * 転送済みframeを処理中に適用されるarbiter ownerを返す。CLAIMは、queue内の
 * 全recognizerがframeを確認する前は暫定ownerとして見える。解決後は確定ownerとなり、
 * 同じownerはそのsessionのterminal releaseでも見える。これによりrouting側は
 * recognizer固有の可変flagを直接参照せず、非互換な生経路を抑止できる。 */
enum travelers_pad_owner travelers_pad_arbiter_owner_for_frame(uint32_t frame_seq);

/* Record a terminal release against the frame that established its ownership.
 * Releases are deliberately never blocked: a stale token must not leave a
 * host button held. The record is diagnostic only and can expose an invalid
 * release path without changing that fail-safe rule.
 * terminal releaseを、その所有権を確立したframeへ記録する。stale tokenによって
 * ホストのボタンが押下状態に残らないよう、releaseは意図的に止めない。この記録は
 * 診断専用であり、安全側の規則を変えずに不正なrelease経路を検出する。 */
bool travelers_pad_arbiter_note_terminal_output(uint32_t ownership_frame_seq,
					 enum travelers_pad_recognizer_id recognizer,
					 enum travelers_pad_owner requested_owner,
					 uint32_t capability,
					 const char *producer);

/* Check a semantic action against an already-resolved token. This is
 * authoritative and returns false on mismatch.
 * It is used by resolver-dispatched first actions and primary recognizers'
 * continuation actions. Releases remain fail-safe so a stale token cannot
 * leave a host button held.
 * 解決済みtokenに対してsemantic actionを照合する。これはauthoritativeで、不一致なら
 * falseを返す。resolver-dispatchされた初回
 * actionおよびprimary recognizerの継続actionで使用する。releaseはstale tokenで
 * host側のボタンが押下状態に残らないよう、常に安全側で送る。 */
bool travelers_pad_arbiter_authorize_output(uint32_t ownership_frame_seq,
					    enum travelers_pad_recognizer_id recognizer,
					    enum travelers_pad_owner requested_owner,
					    uint32_t capability, const char *producer);

/* An auxiliary function may extend an already-committed primary action, but
 * may not establish ownership itself. This validates its delayed output
 * against the live token. It is intentionally quiet on a mismatch because
 * timers may wake after a normal release or ownership hand-off.
 * 補助機能は確定済みprimary操作を継続できるが、自身で所有権は確立できない。
 * 遅延出力をlive tokenに対して検証する。通常の解放・所有権移行後にもtimerが
 * 起き得るため、不一致は意図的に静かに扱う。 */
bool travelers_pad_arbiter_authorize_auxiliary_output(
	enum travelers_pad_auxiliary_id auxiliary, enum travelers_pad_owner required_owner,
	uint32_t capability, const char *producer);

#endif
