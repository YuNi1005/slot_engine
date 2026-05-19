/**
 * @file fruit_judge.c
 * @brief simple_fruit 役判定実装
 *
 * 判定ライン: 中段1ライン（3リール中段のシンボルが揃っているか）
 *
 * 特殊ルール:
 *   - チェリーは左リール中段にのみ止まれば入賞（2枚払い出し）
 *     ただし3つ揃いの場合は3つ揃い扱い（2枚）
 */

#include "fruit_judge.h"
#include "fruit_config.h"

void fruit_judge(SlotState *state, struct SlotGameDef *self)
{
    (void)self;

    state->win_count = 0;

    /* 中段のシンボルを取り出す
     * stop.pos[i] はリールの停止コマ番号（中段基準） */
    SymbolID s0 = FRUIT_REELS[0].symbols[state->stop.pos[0]];  /* 左中段 */
    SymbolID s1 = FRUIT_REELS[1].symbols[state->stop.pos[1]];  /* 中中段 */
    SymbolID s2 = FRUIT_REELS[2].symbols[state->stop.pos[2]];  /* 右中段 */

    /* --- 3つ揃い判定 --------------------------------------- */
    if (s0 == s1 && s1 == s2) {
        int i;
        for (i = 0; i < FRUIT_PAY_TABLE_SIZE; i++) {
            if (FRUIT_PAY_TABLE[i].sym == s0) {
                WinEntry *w     = &state->wins[state->win_count++];
                w->payline_id   = 0;              /* 中段ライン */
                w->symbol_id    = s0;
                w->payout       = FRUIT_PAY_TABLE[i].payout;
                return;
            }
        }
    }

    /* --- チェリー単独入賞（左リール中段のみ） -------------- */
    if (s0 == SYM_CHERRY) {
        WinEntry *w   = &state->wins[state->win_count++];
        w->payline_id = 0;
        w->symbol_id  = SYM_CHERRY;
        w->payout     = FRUIT_CHERRY_SINGLE_PAYOUT;
    }
}