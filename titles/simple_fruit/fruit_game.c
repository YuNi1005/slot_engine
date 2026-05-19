/**
 * @file fruit_game.c
 * @brief simple_fruit タイトル実装（最小構成）
 *
 * このファイルで SlotGameDef の全コールバックを実装し、
 * g_fruit_game として公開する。
 *
 * タイトル固有状態は FruitState に収める。
 * エンジンには void* として渡すだけで、中身は触られない。
 */

#include "fruit_game.h"
#include "fruit_config.h"
#include "fruit_judge.h"
#include <stddef.h>  /* NULL */

/* ============================================================
 * タイトル固有状態
 * ============================================================ */

/**
 * @brief simple_fruit の拡張状態
 *
 * 今回は最小構成なので連続入賞フラグだけ持つ。
 */
typedef struct {
    uint32_t consecutive_wins; /**< 連続入賞ゲーム数（デモ用カウンタ） */
} FruitState;

/* グローバルに 1 つだけ確保（組み込みでも安全な静的確保） */
static FruitState s_fruit_state;

/* ============================================================
 * コールバック実装
 * ============================================================ */

static void on_init(SlotState *state, struct SlotGameDef *self)
{
    FruitState *fs = (FruitState *)self->title_state;
    fs->consecutive_wins = 0;

    /* 初期クレジット設定 */
    state->credits = FRUIT_INITIAL_CREDITS;
}

static void on_shutdown(struct SlotGameDef *self)
{
    (void)self;
    /* 動的確保なし → 何もしない */
}

static uint8_t on_bet(const SlotState *state, struct SlotGameDef *self)
{
    (void)self;
    /* クレジット不足ならベット拒否 */
    if (state->credits < FRUIT_BET_PER_GAME) {
        return 0;
    }
    return FRUIT_BET_PER_GAME;
}

static void on_spin_start(SlotState *state, struct SlotGameDef *self)
{
    /* ベット分を消費 */
    state->credits -= state->bet;
    (void)self;
    /* 乱数による内部抽選はエンジンの RNG を直接使わず、
     * 停止制御（on_stop_request）で簡易的に実装する。
     * 本来はここで内部抽選を行い結果をタイトル状態に保存する。 */
}

static uint8_t on_stop_request(const SlotState *state,
                                struct SlotGameDef *self,
                                uint8_t reel_index)
{
    (void)self;
    /*
     * 最小実装: プレイヤーが停止ボタンを押した瞬間の
     * リール位置をそのまま止める（滑りなし）。
     * 実際の stop.pos[i] はエンジン側の reel_ctrl が持っているが、
     * コールバック時点では state->stop はまだ更新前なので
     * ここでは「現在位置を返す = そのまま止まる」ことを示す 0 を返す。
     *
     * reel_ctrl_request_stop() は「target_pos まで進んで止まる」
     * なので 0 を返すと次に来る 0 番コマで止まる。
     * 簡易版のため全リール0番コマ固定。
     * より正確にするには ReelCtrlMgr の current_pos を参照する必要がある
     * （Phase 4 で改善予定）。
     */
    (void)state;
    (void)reel_index;
    /* Phase 3 簡易版: 常に現在の停止位置付近（コマ0）を返す */
    return 0;
}

static void on_judge(SlotState *state, struct SlotGameDef *self)
{
    fruit_judge(state, self);
}

static uint32_t on_payout(SlotState *state, struct SlotGameDef *self)
{
    FruitState *fs = (FruitState *)self->title_state;

    uint32_t total = 0;
    uint8_t i;
    for (i = 0; i < state->win_count; i++) {
        total += state->wins[i].payout;
    }

    if (total > 0) {
        fs->consecutive_wins++;
    } else {
        fs->consecutive_wins = 0;
    }

    return total;
}

static void on_frame_update(SlotState *state,
                             struct SlotGameDef *self,
                             uint32_t delta_ms)
{
    (void)state;
    (void)self;
    (void)delta_ms;
    /* フルーツスロットは毎フレーム更新処理なし */
}

/* ============================================================
 * SlotGameDef インスタンス
 * ============================================================ */

SlotGameDef g_fruit_game = {
    /* メタ情報 */
    .title_name  = "Simple Fruit",
    .version     = "1.0.0",

    /* リール定義 */
    .reels       = FRUIT_REELS,
    .reel_count  = FRUIT_REEL_COUNT,

    /* タイトル固有状態 */
    .title_state = &s_fruit_state,

    /* コールバック */
    .on_init         = on_init,
    .on_shutdown     = on_shutdown,
    .on_bet          = on_bet,
    .on_spin_start   = on_spin_start,
    .on_stop_request = on_stop_request,
    .on_judge        = on_judge,
    .on_payout       = on_payout,
    .on_frame_update = on_frame_update,
};