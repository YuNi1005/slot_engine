/**
 * @file renderer_cli.c
 * @brief CLI レンダラ実装
 *
 * 【チカチカ対策】
 *   毎フレームの画面全消去（ANSI_CLEAR）をやめ、
 *   カーソルをホーム位置（左上）に戻して上書きする方式に変更。
 *   ANSI_ERASE_LINE で行末残像も消す。
 *
 * 出力イメージ:
 *
 *   +=================================+
 *   |      SIMPLE FRUIT SLOT          |
 *   +=================================+
 *   |  [ CHR ]  [ LMN ]  [ ORN ]     |
 *   |  [ BEL ]  [ PLM ]  [ BAR ]  <-- pay line
 *   |  [ LMN ]  [ ORN ]  [ PLM ]     |
 *   +=================================+
 *   |  Credits :    47   Bet : 3      |
 *   |  Phase   : SPINNING             |
 *   |  Last    : ---                  |
 *   |  Games   : 0                    |
 *   +=================================+
 *   Last key  : [b]
 *   [z]Lever  [b]Bet  [1][2][3]Stop  [q]Quit
 */

#include "renderer_cli.h"
#include "titles/simple_fruit/fruit_config.h"
#include "platform/platform_cli.h"
#include <stdio.h>
#include <string.h>

/* ============================================================
 * ANSI エスケープ定義
 * ============================================================ */

/* 画面全消去（初回のみ使う） */
#define ANSI_CLEAR      "\033[2J\033[H"
/* カーソルをホーム（左上）に戻すだけ — 画面は消さない ← チカチカ防止の肝 */
#define ANSI_HOME       "\033[H"
/* 行末まで消去（上書き時の残像対策） */
#define ANSI_ERASE_LINE "\033[K"

#define ANSI_RESET      "\033[0m"
#define ANSI_BOLD       "\033[1m"
#define ANSI_RED        "\033[31m"
#define ANSI_YELLOW     "\033[33m"
#define ANSI_CYAN       "\033[36m"
#define ANSI_GREEN      "\033[32m"
#define ANSI_WHITE      "\033[37m"

/* ============================================================
 * フェーズ名テーブル（固定幅8文字で揃える）
 * ============================================================ */

static const char * const PHASE_NAMES[] = {
    "IDLE    ",
    "BET     ",
    "SPINNING",
    "STOPPING",
    "JUDGING ",
    "PAYOUT  ",
    "BONUS   ",
    "ERROR   ",
};
#define PHASE_NAMES_SIZE  (int)(sizeof(PHASE_NAMES)/sizeof(PHASE_NAMES[0]))

/* ============================================================
 * 最後に押されたキーのログ（入力確認用）
 * ============================================================ */

/* last_key は PlatformCLI.last_key から直接取得するため個別変数不要 */

/* ============================================================
 * シンボル名取得
 * ============================================================ */

static const char *sym_name(uint8_t sym_id)
{
    if (sym_id < SYM_COUNT) return FRUIT_SYM_NAMES[sym_id];
    return "???";
}

/* ============================================================
 * 描画実装
 * ============================================================ */

static int s_first_draw = 1;  /* 初回のみ画面全消去するフラグ */

void renderer_cli_draw(const SlotEngine *engine, const PlatformCLI *cli)
{
    const SlotState   *st  = &engine->state;
    const ReelCtrlMgr *mgr = &engine->reels;

    /* 初回だけ全消去、以降はカーソルをホームに戻して上書き */
    if (s_first_draw) {
        printf(ANSI_CLEAR);
        s_first_draw = 0;
    } else {
        printf(ANSI_HOME);
    }

    /* ---- ヘッダ ------------------------------------------- */
    printf(ANSI_BOLD ANSI_CYAN
           "  +=================================+" ANSI_ERASE_LINE "\n"
           "  |      SIMPLE FRUIT SLOT          |" ANSI_ERASE_LINE "\n"
           "  +=================================+" ANSI_ERASE_LINE "\n"
           ANSI_RESET);

    /* ---- リール窓（3行 × 3リール） ----------------------- */
    {
        int row;
        for (row = 0; row < REEL_VISIBLE_ROWS; row++) {
            printf("  |  ");
            int reel;
            for (reel = 0; reel < (int)mgr->count; reel++) {
                SymbolID sym  = reel_ctrl_get_symbol(mgr, (uint8_t)reel, (uint8_t)row);
                const char *n = sym_name(sym);
                if (row == 1) {
                    printf(ANSI_YELLOW "[ %s ]" ANSI_RESET, n);
                } else {
                    printf(ANSI_WHITE  "[ %s ]" ANSI_RESET, n);
                }
                if (reel < (int)mgr->count - 1) printf("  ");
            }
            if (row == 1) {
                printf("  " ANSI_YELLOW "<-- pay line" ANSI_RESET);
            }
            printf(ANSI_ERASE_LINE "\n");
        }
    }

    /* ---- ステータス --------------------------------------- */
    printf("  +=================================+" ANSI_ERASE_LINE "\n");

    printf("  |  Credits : " ANSI_GREEN "%5u" ANSI_RESET
           "   Bet : %-2u" ANSI_ERASE_LINE "\n",
           st->credits, st->bet);

    const char *phase_str = (st->phase < (SlotPhase)PHASE_NAMES_SIZE)
                            ? PHASE_NAMES[st->phase] : "UNKNOWN ";
    printf("  |  Phase   : " ANSI_CYAN "%s" ANSI_RESET
           ANSI_ERASE_LINE "\n", phase_str);

    /* 入賞表示（固定幅で書いて残像防止） */
    if (st->win_count > 0) {
        printf("  |  " ANSI_RED ANSI_BOLD "WIN!  +%-5u coins"
               ANSI_RESET ANSI_ERASE_LINE "\n", st->total_payout);
    } else if (st->phase == SLOT_STATE_IDLE && st->total_games > 0) {
        printf("  |  Last    : no win      " ANSI_ERASE_LINE "\n");
    } else {
        printf("  |  Last    : ---         " ANSI_ERASE_LINE "\n");
    }

    printf("  |  Games   : %-6u" ANSI_ERASE_LINE "\n", st->total_games);
    printf("  +=================================+" ANSI_ERASE_LINE "\n");

    /* ---- 入力確認ログ ------------------------------------- */
    if (cli->last_key != 0) {
        printf("  Last key  : [%c]   " ANSI_ERASE_LINE "\n", cli->last_key);
    } else {
        printf("  Last key  : (none)" ANSI_ERASE_LINE "\n");
    }

    /* ---- 操作ガイド --------------------------------------- */
    printf("\n"
           "  " ANSI_BOLD "[z]" ANSI_RESET "Lever  "
           ANSI_BOLD "[b]" ANSI_RESET "Bet  "
           ANSI_BOLD "[1][2][3]" ANSI_RESET "Stop  "
           ANSI_BOLD "[q]" ANSI_RESET "Quit"
           ANSI_ERASE_LINE "\n");

    fflush(stdout);
}

void renderer_cli_print_help(void)
{
    printf(ANSI_CLEAR);
    printf("  +=================================+\n");
    printf("  |      SIMPLE FRUIT SLOT          |\n");
    printf("  |                                 |\n");
    printf("  |  How to play:                   |\n");
    printf("  |    [b] or [m] ... Bet           |\n");
    printf("  |    [z]         ... Spin (Lever) |\n");
    printf("  |    [1][2][3]   ... Stop reels   |\n");
    printf("  |    [q]         ... Quit         |\n");
    printf("  |                                 |\n");
    printf("  |  Match 3 on the center line:    |\n");
    printf("  |       7  = 100 coins            |\n");
    printf("  |      BAR =  30 coins            |\n");
    printf("  |      BEL =  15 coins            |\n");
    printf("  |      PLM =   8 coins            |\n");
    printf("  |      ORN =   5 coins            |\n");
    printf("  |      LMN =   3 coins            |\n");
    printf("  |      CHR =   2 coins            |\n");
    printf("  +=================================+\n");
    printf("\n  Press ENTER to start...\n");
    fflush(stdout);
    getchar();
}