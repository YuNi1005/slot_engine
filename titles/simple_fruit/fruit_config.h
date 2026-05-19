/**
 * @file fruit_config.h
 * @brief simple_fruit タイトル設定
 *
 * リール配列と払い出しテーブルを定数として定義する。
 * ゲームバランスを調整したいときはここだけ編集する。
 *
 * シンボルID一覧:
 *   0 = CHERRY  チェリー
 *   1 = LEMON   レモン
 *   2 = ORANGE  オレンジ
 *   3 = PLUM    プラム
 *   4 = BELL    ベル
 *   5 = BAR     バー
 *   6 = SEVEN   セブン
 */

#ifndef FRUIT_CONFIG_H
#define FRUIT_CONFIG_H

#include <stdint.h>
#include "game_def.h"

/* ============================================================
 * シンボル ID
 * ============================================================ */

#define SYM_CHERRY  0
#define SYM_LEMON   1
#define SYM_ORANGE  2
#define SYM_PLUM    3
#define SYM_BELL    4
#define SYM_BAR     5
#define SYM_SEVEN   6
#define SYM_COUNT   7

/* ============================================================
 * シンボル表示名（renderer_cli 用）
 * ============================================================ */

static const char * const FRUIT_SYM_NAMES[SYM_COUNT] = {
    "CHR",   /* CHERRY */
    "LMN",   /* LEMON  */
    "ORN",   /* ORANGE */
    "PLM",   /* PLUM   */
    "BEL",   /* BELL   */
    "BAR",   /* BAR    */
    " 7 ",   /* SEVEN  */
};

/* ============================================================
 * リール配列（各21コマ）
 * ============================================================ */

/* リール 0（左） */
static const SymbolID REEL0_SYMBOLS[] = {
    SYM_CHERRY, SYM_LEMON,  SYM_ORANGE, SYM_PLUM,   SYM_BELL,
    SYM_CHERRY, SYM_LEMON,  SYM_ORANGE, SYM_PLUM,   SYM_BAR,
    SYM_CHERRY, SYM_LEMON,  SYM_ORANGE, SYM_PLUM,   SYM_BELL,
    SYM_CHERRY, SYM_LEMON,  SYM_ORANGE, SYM_BAR,    SYM_SEVEN,
    SYM_CHERRY,
};

/* リール 1（中） */
static const SymbolID REEL1_SYMBOLS[] = {
    SYM_LEMON,  SYM_ORANGE, SYM_PLUM,   SYM_BELL,   SYM_CHERRY,
    SYM_LEMON,  SYM_ORANGE, SYM_PLUM,   SYM_BAR,    SYM_CHERRY,
    SYM_LEMON,  SYM_ORANGE, SYM_PLUM,   SYM_BELL,   SYM_CHERRY,
    SYM_LEMON,  SYM_ORANGE, SYM_BAR,    SYM_SEVEN,  SYM_CHERRY,
    SYM_BELL,
};

/* リール 2（右） */
static const SymbolID REEL2_SYMBOLS[] = {
    SYM_ORANGE, SYM_PLUM,   SYM_BELL,   SYM_LEMON,  SYM_CHERRY,
    SYM_ORANGE, SYM_PLUM,   SYM_BAR,    SYM_LEMON,  SYM_CHERRY,
    SYM_ORANGE, SYM_PLUM,   SYM_BELL,   SYM_LEMON,  SYM_CHERRY,
    SYM_ORANGE, SYM_BAR,    SYM_SEVEN,  SYM_LEMON,  SYM_CHERRY,
    SYM_BELL,
};

/* リール定義まとめ */
static const ReelStrip FRUIT_REELS[] = {
    { REEL0_SYMBOLS, (uint8_t)(sizeof(REEL0_SYMBOLS)/sizeof(REEL0_SYMBOLS[0])) },
    { REEL1_SYMBOLS, (uint8_t)(sizeof(REEL1_SYMBOLS)/sizeof(REEL1_SYMBOLS[0])) },
    { REEL2_SYMBOLS, (uint8_t)(sizeof(REEL2_SYMBOLS)/sizeof(REEL2_SYMBOLS[0])) },
};

#define FRUIT_REEL_COUNT  3

/* ============================================================
 * 払い出しテーブル（中段1ライン判定）
 *
 * 3つ揃い → 払い出し枚数
 * ============================================================ */

typedef struct {
    SymbolID sym;
    uint16_t payout;
} FruitPayEntry;

static const FruitPayEntry FRUIT_PAY_TABLE[] = {
    { SYM_SEVEN,   100 },  /* 7 揃い       100枚 */
    { SYM_BAR,      30 },  /* BAR 揃い      30枚 */
    { SYM_BELL,     15 },  /* BELL 揃い     15枚 */
    { SYM_PLUM,      8 },  /* PLUM 揃い      8枚 */
    { SYM_ORANGE,    5 },  /* ORANGE 揃い    5枚 */
    { SYM_LEMON,     3 },  /* LEMON 揃い     3枚 */
    { SYM_CHERRY,    2 },  /* CHERRY 揃い    2枚 */
};

#define FRUIT_PAY_TABLE_SIZE  \
    (int)(sizeof(FRUIT_PAY_TABLE)/sizeof(FRUIT_PAY_TABLE[0]))

/* チェリー左リール単独入賞（左リール中段のみチェリー） */
#define FRUIT_CHERRY_SINGLE_PAYOUT  2

/* ============================================================
 * ゲーム設定
 * ============================================================ */

#define FRUIT_INITIAL_CREDITS  50   /**< ゲーム開始時のクレジット */
#define FRUIT_BET_PER_GAME     3    /**< 1ゲームのベット枚数（固定） */

#endif /* FRUIT_CONFIG_H */