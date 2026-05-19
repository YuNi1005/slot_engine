/**
 * @file main_cli.c
 * @brief CLI ターゲット エントリーポイント
 *
 * stdio だけで動く最小スロット実行環境。
 * SDL2 も外部ライブラリも不要。
 *
 * ゲームループ:
 *   1. slot_engine_update() でゲーム状態を進める
 *   2. renderer_cli_draw() でコンソールに描画
 *   3. BTN_MENU（q キー）で終了
 *
 * フレームレート:
 *   約 60fps を目標に 16ms スリープを挟む。
 *   精度は platform_cli の clock() 依存。
 */

#include <stdio.h>
#include <stdlib.h>

/* エンジン */
#include "engine/slot_engine.h"

/* プラットフォーム */
#include "platform/platform_cli.h"

/* タイトル */
#include "titles/simple_fruit/fruit_game.h"

/* レンダラ */
#include "renderer_cli.h"

/* ============================================================
 * ポータブル スリープ（msec 単位）
 * ============================================================ */

#ifdef _WIN32
#  include <windows.h>
#  define SLEEP_MS(ms)  Sleep(ms)
#else
#  include <unistd.h>
#  define SLEEP_MS(ms)  usleep((useconds_t)(ms) * 1000u)
#endif

/* ============================================================
 * main
 * ============================================================ */

int main(void)
{
    /* ---- プラットフォーム初期化 --------------------------- */
    PlatformCLI cli_data;
    PlatformAPI platform;
    platform_cli_init(&cli_data, &platform);

    /* ---- ヘルプ画面 --------------------------------------- */
    renderer_cli_print_help();

    /* ---- エンジン初期化 ----------------------------------- */
    SlotEngine engine;
    slot_engine_init(&engine, &g_fruit_game, &platform);

    /* ---- メインループ ------------------------------------- */
    while (slot_engine_is_running(&engine)) {

        /* 1フレーム更新（入力 → 状態遷移 → タイトルコールバック） */
        slot_engine_update(&engine);

        /* 描画 */
        renderer_cli_draw(&engine, &cli_data);

        /* BTN_MENU（q）で終了 */
        if (input_mgr_pressed(&engine.input, BTN_MENU)) {
            break;
        }

        /* クレジット切れで終了 */
        if (engine.state.credits == 0 &&
            engine.state.phase == SLOT_STATE_IDLE)
        {
            printf("\n  Game Over! No credits left.\n");
            break;
        }

        /* ~60fps: 16ms スリープ */
        SLEEP_MS(16);
    }

    /* ---- 終了処理 ----------------------------------------- */
    slot_engine_shutdown(&engine);

    printf("\n  Thanks for playing! Final credits: %u\n\n",
           engine.state.credits);

    return 0;
}