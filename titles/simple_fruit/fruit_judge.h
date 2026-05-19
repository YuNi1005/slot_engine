/**
 * @file fruit_judge.h
 * @brief simple_fruit 役判定 API
 */

#ifndef FRUIT_JUDGE_H
#define FRUIT_JUDGE_H

#include "game_def.h"

/**
 * @brief 役判定を行い state->wins / win_count を埋める
 *
 * SlotGameDef.on_judge コールバックとして登録する。
 *
 * @param state  エンジン共有状態（stop / wins / win_count を参照・書き込み）
 * @param self   SlotGameDef へのポインタ（未使用）
 */
void fruit_judge(SlotState *state, struct SlotGameDef *self);

#endif /* FRUIT_JUDGE_H */