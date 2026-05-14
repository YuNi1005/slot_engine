/**
 * @file effect_mgr.c
 * @brief 演出キュー実装
 *
 * FIFO リングバッファで演出リクエストを管理する。
 * head/tail は常に [0, EFFECT_QUEUE_MAX) の範囲を循環する。
 */

#include "effect_mgr.h"
#include <string.h>  /* memset */

void effect_mgr_init(EffectMgr *mgr)
{
    memset(mgr, 0, sizeof(EffectMgr));
}

bool effect_mgr_push(EffectMgr *mgr, EffectRequest req)
{
    if (mgr->count >= EFFECT_QUEUE_MAX) {
        return false;  /* キュー満杯 */
    }
    mgr->queue[mgr->tail] = req;
    mgr->tail = (uint8_t)((mgr->tail + 1) % EFFECT_QUEUE_MAX);
    mgr->count++;
    return true;
}

bool effect_mgr_pop(EffectMgr *mgr, EffectRequest *out)
{
    if (mgr->count == 0) {
        out->type = EFFECT_NONE;
        return false;
    }
    *out = mgr->queue[mgr->head];
    mgr->head = (uint8_t)((mgr->head + 1) % EFFECT_QUEUE_MAX);
    mgr->count--;
    return true;
}

bool effect_mgr_has_pending(const EffectMgr *mgr)
{
    return mgr->count > 0;
}

void effect_mgr_clear(EffectMgr *mgr)
{
    mgr->head  = 0;
    mgr->tail  = 0;
    mgr->count = 0;
}