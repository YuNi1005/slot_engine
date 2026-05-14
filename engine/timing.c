/**
 * @file timing.c
 * @brief タイミング管理実装
 *
 * フレーム時刻の差分計算とビタ押し精度判定を行う。
 * uint32_t のミリ秒タイマーは約 49.7 日でラップアラウンドするため、
 * 差分計算はすべてラップアラウンド対応の引き算で行う。
 */

#include "timing.h"

/* ============================================================
 * 内部ヘルパー
 * ============================================================ */

/**
 * @brief uint32_t タイマーのラップアラウンド対応差分
 *
 * (a - b) を符号なし演算で計算する。
 * タイマーが一周した場合も正しく動作する
 * （ただし差が UINT32_MAX/2 = 約24.8 日を超えると不正確）。
 */
static uint32_t elapsed_ms(uint32_t later, uint32_t earlier)
{
    return later - earlier; /* uint32 の自然なオーバーフロー動作を利用 */
}

/* ============================================================
 * API 実装
 * ============================================================ */

void timing_init(TimingMgr *tmg, uint32_t now_ms)
{
    tmg->last_frame_ms = now_ms;
    tmg->delta_ms      = 0;
    tmg->frame_count   = 0;
}

void timing_tick(TimingMgr *tmg, uint32_t now_ms)
{
    tmg->delta_ms      = elapsed_ms(now_ms, tmg->last_frame_ms);
    tmg->last_frame_ms = now_ms;
    tmg->frame_count++;
}

uint32_t timing_delta(const TimingMgr *tmg)
{
    return tmg->delta_ms;
}

uint32_t timing_frame_count(const TimingMgr *tmg)
{
    return tmg->frame_count;
}

bool timing_is_vita(uint32_t target_ms, uint32_t pressed_ms)
{
    return timing_diff_ms(target_ms, pressed_ms) <= TIMING_VITA_WINDOW_MS;
}

uint32_t timing_diff_ms(uint32_t a, uint32_t b)
{
    /*
     * どちらが大きいか分からないため、
     * 差を符号なしで計算して小さいほうを返す。
     * ラップアラウンドが起きていない限り正しい絶対差を返す。
     */
    uint32_t d1 = a - b; /* a >= b の場合の差 */
    uint32_t d2 = b - a; /* b >  a の場合の差 */
    return (d1 < d2) ? d1 : d2;
}