/**
 * @file slot_engine.c
 * @brief スロットエンジン本体
 *
 * ゲームの 1 フレームを以下の順序で処理する:
 *   1. タイミング更新（delta_ms 計算）
 *   2. 入力取得・チャタリング除去
 *   3. ゲームフェーズ遷移（IDLE → BET → SPINNING → STOPPING → JUDGING → PAYOUT）
 *   4. タイトルコールバック呼び出し（on_frame_update は毎フレーム）
 *   5. 描画（draw_begin → レンダラ → draw_end）は外部レンダラが担当
 *
 * このファイルはプラットフォーム固有コードを一切含まない。
 */

#include "slot_engine.h"
#include "save_mgr.h"
#include <string.h>  /* memset */

/* ============================================================
 * 内部ヘルパー：フェーズ遷移
 * ============================================================ */

/** IDLE フェーズの処理（ベット待ち） */
static void phase_idle(SlotEngine *eng)
{
    /* BTN_BET_1 または BTN_BET_MAX でベットフェーズへ */
    if (input_mgr_pressed(&eng->input, BTN_BET_1) ||
        input_mgr_pressed(&eng->input, BTN_BET_MAX))
    {
        eng->state.phase = SLOT_STATE_BET;
    }
}

/** BET フェーズの処理 */
static void phase_bet(SlotEngine *eng)
{
    uint8_t bet = eng->game->on_bet(&eng->state, eng->game);
    if (bet == 0) {
        /* ベット拒否（クレジット不足など）→ IDLE に戻る */
        eng->state.phase = SLOT_STATE_IDLE;
        return;
    }
    eng->state.bet = bet;

    /* レバーでスピン開始 */
    if (input_mgr_pressed(&eng->input, BTN_LEVER)) {
        eng->game->on_spin_start(&eng->state, eng->game);
        reel_ctrl_start_all(&eng->reels);

        /* スピン開始演出をキューに積む */
        EffectRequest req = {EFFECT_SPIN_START, EFFECT_PRIO_NORMAL, 0, 0};
        effect_mgr_push(&eng->effects, req);

        eng->state.phase = SLOT_STATE_SPINNING;
    }
}

/** SPINNING フェーズの処理 */
static void phase_spinning(SlotEngine *eng)
{
    /* 各停止ボタン処理 */
    static const ButtonID stop_btns[SLOT_MAX_REELS] = {
        BTN_STOP_0, BTN_STOP_1, BTN_STOP_2
    };

    uint8_t i;
    for (i = 0; i < eng->reels.count; i++) {
        if (input_mgr_pressed(&eng->input, stop_btns[i])) {
            uint8_t target = eng->game->on_stop_request(
                &eng->state, eng->game, i);
            reel_ctrl_request_stop(&eng->reels, i, target);

            /* 停止演出をキューに積む */
            EffectRequest req = {EFFECT_REEL_STOP, EFFECT_PRIO_NORMAL,
                                 (uint32_t)i, 0};
            effect_mgr_push(&eng->effects, req);
        }
    }

    /* 全リール停止したら判定フェーズへ */
    if (reel_ctrl_all_stopped(&eng->reels)) {
        reel_ctrl_get_stop_result(&eng->reels, &eng->state.stop);
        eng->state.phase = SLOT_STATE_JUDGING;
    }
}

/** JUDGING フェーズの処理 */
static void phase_judging(SlotEngine *eng)
{
    eng->state.win_count    = 0;
    eng->state.total_payout = 0;
    eng->game->on_judge(&eng->state, eng->game);
    eng->state.phase = SLOT_STATE_PAYOUT;
}

/** PAYOUT フェーズの処理 */
static void phase_payout(SlotEngine *eng)
{
    uint32_t payout = eng->game->on_payout(&eng->state, eng->game);
    eng->state.total_payout = payout;
    eng->state.credits     += payout;
    eng->state.total_games++;

    if (payout > 0) {
        EffectRequest req = {EFFECT_COIN_OUT, EFFECT_PRIO_NORMAL, payout, 0};
        effect_mgr_push(&eng->effects, req);
    }

    /* セーブ */
    save_mgr_save(&eng->state, eng->platform);

    eng->state.phase = SLOT_STATE_IDLE;
}

/* ============================================================
 * API 実装
 * ============================================================ */

void slot_engine_init(SlotEngine *engine,
                      SlotGameDef *game,
                      struct PlatformAPI *platform)
{
    memset(engine, 0, sizeof(SlotEngine));
    engine->game     = game;
    engine->platform = platform;
    engine->running  = true;

    /* 各サブシステム初期化 */
    uint32_t now = platform->get_time_ms(platform);
    rng_seed(&engine->rng, (uint64_t)now ^ 0xABCD1234u);
    timing_init(&engine->timing, now);
    input_mgr_init(&engine->input);
    effect_mgr_init(&engine->effects);

    /* セーブデータ読み込み（失敗してもゼロ初期化のまま続行） */
    save_mgr_load(&engine->state, platform);

    /* タイトル初期化 */
    game->on_init(&engine->state, game);

    /* リール初期化（on_init でリール設定が完了した後） */
    reel_ctrl_init(&engine->reels, game->reels, game->reel_count);

    engine->state.phase = SLOT_STATE_IDLE;
}

void slot_engine_update(SlotEngine *engine)
{
    struct PlatformAPI *p = engine->platform;
    uint32_t now = p->get_time_ms(p);

    /* 1. タイミング更新 */
    timing_tick(&engine->timing, now);
    uint32_t delta = timing_delta(&engine->timing);

    /* 2. 入力更新 */
    input_mgr_update(&engine->input, p);

    /* 3. リール更新 */
    reel_ctrl_update(&engine->reels);

    /* 4. フェーズ遷移 */
    switch (engine->state.phase) {
        case SLOT_STATE_IDLE:     phase_idle(engine);     break;
        case SLOT_STATE_BET:      phase_bet(engine);      break;
        case SLOT_STATE_SPINNING: phase_spinning(engine); break;
        case SLOT_STATE_JUDGING:  phase_judging(engine);  break;
        case SLOT_STATE_PAYOUT:   phase_payout(engine);   break;
        default: break;
    }

    /* 5. タイトルの毎フレーム更新 */
    engine->game->on_frame_update(&engine->state, engine->game, delta);

    /* 6. 描画は外部レンダラが担当するため、ここでは draw_begin/end のみ */
    p->draw_begin(p);
    /* ※ レンダラは slot_engine の状態を読んで描画する（別ファイル） */
    p->draw_end(p);
}

void slot_engine_shutdown(SlotEngine *engine)
{
    save_mgr_save(&engine->state, engine->platform);
    engine->game->on_shutdown(engine->game);
    engine->running = false;
}

bool slot_engine_is_running(const SlotEngine *engine)
{
    return engine->running;
}