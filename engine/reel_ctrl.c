/**
 * @file reel_ctrl.c
 * @brief リール回転・停止・コマ送り制御実装
 *
 * コマ番号はリール配列の先頭を 0 とした環状インデックス。
 * current_pos は中段基準。上段は (current_pos - 1 + len) % len、
 * 下段は (current_pos + 1) % len で取得する。
 */

#include "reel_ctrl.h"

/* ============================================================
 * 内部ヘルパー
 * ============================================================ */

/**
 * @brief コマ番号をリール長でラップアラウンドする
 */
static uint8_t wrap(uint8_t pos, uint8_t len)
{
    return (uint8_t)(pos % len);
}

/**
 * @brief 現在位置から目標位置まで前向き（回転方向）に何コマか
 *
 * リールは一方向（インデックス増加方向）にしか回らない前提。
 * current == target の場合はリール 1 周分（len コマ）を返す
 * （「すでに止まっている」ではなく「1 周してから止まる」扱い）。
 */
static uint8_t forward_distance(uint8_t current, uint8_t target, uint8_t len)
{
    if (target > current) {
        return target - current;
    } else if (target < current) {
        return (uint8_t)(len - current + target);
    } else {
        /* current == target: 1 周して止まる */
        return len;
    }
}

/* ============================================================
 * API 実装
 * ============================================================ */

void reel_ctrl_init(ReelCtrlMgr *mgr,
                    const ReelStrip *strips,
                    uint8_t count)
{
    uint8_t i;
    mgr->count = count;
    for (i = 0; i < count; i++) {
        ReelCtrl *r    = &mgr->reels[i];
        r->state        = REEL_IDLE;
        r->strip        = &strips[i];
        r->current_pos  = 0;
        r->target_pos   = 0;
        r->frame_counter = 0;
        r->stop_offset  = 0;
    }
}

void reel_ctrl_start_all(ReelCtrlMgr *mgr)
{
    uint8_t i;
    for (i = 0; i < mgr->count; i++) {
        ReelCtrl *r = &mgr->reels[i];
        if (r->state == REEL_IDLE || r->state == REEL_STOPPED) {
            r->state         = REEL_SPINNING;
            r->frame_counter = 0;
        }
    }
}

void reel_ctrl_request_stop(ReelCtrlMgr *mgr,
                            uint8_t reel_index,
                            uint8_t target_pos)
{
    if (reel_index >= mgr->count) return;

    ReelCtrl *r = &mgr->reels[reel_index];
    if (r->state != REEL_SPINNING) return;

    uint8_t len    = r->strip->length;
    r->target_pos  = wrap(target_pos, len);
    r->stop_offset = forward_distance(r->current_pos, r->target_pos, len);
    r->state       = REEL_STOPPING;
}

void reel_ctrl_update(ReelCtrlMgr *mgr)
{
    uint8_t i;
    for (i = 0; i < mgr->count; i++) {
        ReelCtrl *r = &mgr->reels[i];

        if (r->state == REEL_IDLE || r->state == REEL_STOPPED) {
            continue;
        }

        /* フレームカウンタを進め、1コマ進むタイミングか確認 */
        r->frame_counter++;
        if (r->frame_counter < REEL_SPEED_FRAMES_PER_SYMBOL) {
            continue;
        }
        r->frame_counter = 0;

        /* 1コマ送り */
        uint8_t len     = r->strip->length;
        r->current_pos  = wrap((uint8_t)(r->current_pos + 1), len);

        if (r->state == REEL_STOPPING) {
            r->stop_offset--;
            if (r->stop_offset == 0) {
                r->state = REEL_STOPPED;
            }
        }
    }
}

bool reel_ctrl_all_stopped(const ReelCtrlMgr *mgr)
{
    uint8_t i;
    for (i = 0; i < mgr->count; i++) {
        if (mgr->reels[i].state != REEL_STOPPED) {
            return false;
        }
    }
    return true;
}

SymbolID reel_ctrl_get_symbol(const ReelCtrlMgr *mgr,
                              uint8_t reel_index,
                              uint8_t row)
{
    if (reel_index >= mgr->count) return 0;

    const ReelCtrl  *r   = &mgr->reels[reel_index];
    const ReelStrip *s   = r->strip;
    uint8_t          len = s->length;

    /* row 0=上段(-1), 1=中段(0), 2=下段(+1) */
    int offset = (int)row - 1;  /* -1, 0, +1 */
    int pos    = (int)r->current_pos + offset;

    /* 負のインデックスをラップ */
    pos = ((pos % len) + len) % len;

    return s->symbols[pos];
}

void reel_ctrl_get_stop_result(const ReelCtrlMgr *mgr, StopResult *out)
{
    uint8_t i;
    for (i = 0; i < mgr->count; i++) {
        out->pos[i] = mgr->reels[i].current_pos;
    }
}