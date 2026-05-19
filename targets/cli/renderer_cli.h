/**
 * @file renderer_cli.h
 * @brief CLI レンダラ API
 *
 * SlotEngine の状態を読んで ANSI エスケープで
 * コンソールにスロット画面を描画する。
 */

#ifndef RENDERER_CLI_H
#define RENDERER_CLI_H

#include "engine/slot_engine.h"
#include "platform/platform_cli.h"

/**
 * @brief スロット画面を描画する
 *
 * slot_engine_update() の後、毎フレーム呼ぶ。
 *
 * @param engine  描画元の SlotEngine（読み取り専用）
 */
void renderer_cli_draw(const SlotEngine *engine, const PlatformCLI *cli);

/**
 * @brief 操作説明を1回だけ表示する
 */
void renderer_cli_print_help(void);


#endif /* RENDERER_CLI_H */