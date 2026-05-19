/**
 * @file fruit_game.h
 * @brief simple_fruit タイトル定義の宣言
 *
 * エントリーポイント（main_cli.c など）はこれを include して
 * g_fruit_game を slot_engine_init() に渡す。
 */

#ifndef FRUIT_GAME_H
#define FRUIT_GAME_H

#include "game_def.h"

/**
 * @brief simple_fruit の SlotGameDef インスタンス
 *
 * fruit_game.c で定義・初期化される。
 * 外部からは読み取り専用で使う（コールバックを直接呼ばないこと）。
 */
extern SlotGameDef g_fruit_game;

#endif /* FRUIT_GAME_H */