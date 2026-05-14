/**
 * @file input_mgr.c
 * @brief 入力管理実装
 *
 * チャタリング除去はカウンタ方式。
 * raw_held が INPUT_DEBOUNCE_FRAMES フレーム連続して true に
 * なった時点で confirmed_held を true に確定する。
 * 逆も同様（連続 false で held 解除）。
 *
 * pressed（エッジ）は confirmed_held が前フレームは false で
 * 今フレームは true になった瞬間に発生する。
 */

#include "input_mgr.h"
#include <string.h>  /* memset */

void input_mgr_init(InputMgr *mgr)
{
    memset(mgr, 0, sizeof(InputMgr));
}

void input_mgr_update(InputMgr *mgr, struct PlatformAPI *api)
{
    /* プラットフォームから生の入力を取得 */
    InputState raw;
    api->get_input(api, &raw);

    /* 前フレームを保存 */
    mgr->previous = mgr->current;

    /* チャタリング除去 + エッジ検出 */
    int i;
    for (i = 0; i < BTN_COUNT; i++) {
        bool raw_now = raw.held[i];

        if (raw_now == mgr->raw_held[i]) {
            /* 同じ状態が続いている: カウンタを進める */
            if (mgr->debounce_count[i] < INPUT_DEBOUNCE_FRAMES) {
                mgr->debounce_count[i]++;
            }
        } else {
            /* 状態が変わった: カウンタをリセット */
            mgr->debounce_count[i] = 0;
            mgr->raw_held[i]       = raw_now;
        }

        /* カウンタが閾値に達したら確定 */
        if (mgr->debounce_count[i] >= INPUT_DEBOUNCE_FRAMES) {
            mgr->current.held[i] = mgr->raw_held[i];
        }

        /* 押し下げエッジ = 前フレーム false && 今フレーム true */
        mgr->current.pressed[i] =
            (!mgr->previous.held[i]) && mgr->current.held[i];
    }
}

bool input_mgr_pressed(const InputMgr *mgr, ButtonID btn)
{
    return mgr->current.pressed[(int)btn];
}

bool input_mgr_held(const InputMgr *mgr, ButtonID btn)
{
    return mgr->current.held[(int)btn];
}