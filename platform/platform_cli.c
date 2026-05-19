/**
 * @file platform_cli.c
 * @brief CLI プラットフォーム実装（stdio + POSIX/Windows 最小依存）
 *
 * 依存ライブラリ: C標準ライブラリのみ
 *   <stdio.h> <stdlib.h> <string.h> <time.h>
 *   + Windows では <conio.h>、POSIX では <termios.h> <unistd.h>
 *
 * キー割り当て:
 *   z    = レバー（スタート）
 *   1    = 第1停止
 *   2    = 第2停止
 *   3    = 第3停止
 *   b    = 1BET
 *   m    = MAXベット
 *   q    = 終了（BTN_MENU 長押し扱い）
 */

#include "platform_cli.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Windows: ANSI エスケープシーケンスを有効化 */
#ifdef _WIN32
#  include <windows.h>
static void win_enable_ansi(void)
{
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD  mode = 0;
    if (GetConsoleMode(h, &mode)) {
        SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
}
#endif

/* ============================================================
 * プラットフォーム別ノンブロッキングキー入力
 * ============================================================ */

#ifdef _WIN32
#  include <conio.h>
#  define KB_HIT()   _kbhit()
#  define KB_GET()   _getch()
#else
/* POSIX: termios でカノニカルモード OFF */
#  include <termios.h>
#  include <unistd.h>
#  include <fcntl.h>

static struct termios s_orig_termios;
static int            s_raw_mode = 0;

static void cli_enable_raw(void)
{
    if (s_raw_mode) return;
    tcgetattr(STDIN_FILENO, &s_orig_termios);
    struct termios raw = s_orig_termios;
    raw.c_lflag &= (tcflag_t)~(ICANON | ECHO);
    raw.c_cc[VMIN]  = 0;   /* ノンブロッキング */
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    s_raw_mode = 1;
}

static void cli_disable_raw(void)
{
    if (!s_raw_mode) return;
    tcsetattr(STDIN_FILENO, TCSANOW, &s_orig_termios);
    s_raw_mode = 0;
}

static int KB_HIT(void)
{
    cli_enable_raw();
    unsigned char c = 0;
    return (int)(read(STDIN_FILENO, &c, 1) > 0 ? (s_orig_termios.c_cc[0] = c, 1) : 0);
}

/* POSIX 版では KB_HIT が文字を読むため KB_GET はキャッシュを返す */
static char s_posix_last_char = 0;

static int posix_kbhit(void)
{
    cli_enable_raw();
    char c = 0;
    ssize_t n = read(STDIN_FILENO, &c, 1);
    if (n > 0) { s_posix_last_char = c; return 1; }
    return 0;
}

#  undef KB_HIT
#  define KB_HIT()  posix_kbhit()
#  define KB_GET()  s_posix_last_char
#endif /* _WIN32 */

/* ============================================================
 * キー → ButtonID マッピング
 * ============================================================ */

static const struct { char key; ButtonID btn; } KEY_MAP[] = {
    { 'z', BTN_LEVER  },
    { '1', BTN_STOP_0 },
    { '2', BTN_STOP_1 },
    { '3', BTN_STOP_2 },
    { 'b', BTN_BET_1  },
    { 'm', BTN_BET_MAX},
    { 'q', BTN_MENU   },
};
#define KEY_MAP_SIZE  (int)(sizeof(KEY_MAP)/sizeof(KEY_MAP[0]))

/* ============================================================
 * PlatformAPI コールバック実装
 * ============================================================ */

/* ---- 描画 ------------------------------------------------- */

static void cli_draw_begin(struct PlatformAPI *api)
{
    (void)api;
    /* CLI では描画開始時に何もしない（renderer_cli が直接 printf する） */
}

static void cli_draw_end(struct PlatformAPI *api)
{
    (void)api;
    fflush(stdout);
}

static void cli_draw_rect(struct PlatformAPI *api, Rect rect, Color color)
{
    /* CLI では塗りつぶし矩形は未対応（renderer_cli が文字描画で代用） */
    (void)api; (void)rect; (void)color;
}

static void cli_draw_text(struct PlatformAPI *api,
                          int x, int y,
                          const char *text,
                          Color color,
                          int size,
                          TextAlign align)
{
    /* ANSI カーソル移動で x/y を再現。y はコンソール行、x は列。 */
    (void)api; (void)color; (void)size; (void)align;
    printf("\033[%d;%dH%s", y + 1, x + 1, text);
}

static void cli_draw_image(struct PlatformAPI *api,
                            uint32_t image_id,
                            const Rect *src,
                            Rect dst)
{
    (void)api; (void)image_id; (void)src; (void)dst;
    /* CLI では画像未対応 */
}

/* ---- 音声 ------------------------------------------------- */

static void cli_audio_play(struct PlatformAPI *api,
                            SoundID sound_id, bool loop)
{
    (void)api; (void)sound_id; (void)loop;
    /* CLI では音声未対応 */
}

static void cli_audio_stop(struct PlatformAPI *api, SoundID sound_id)
{
    (void)api; (void)sound_id;
}

static void cli_audio_set_volume(struct PlatformAPI *api, float volume)
{
    (void)api; (void)volume;
}

/* ---- 入力 ------------------------------------------------- */

static void cli_get_input(struct PlatformAPI *api, InputState *out)
{
    PlatformCLI *cli = (PlatformCLI *)api->user_data;

    /* 全ボタンをリセット */
    memset(out, 0, sizeof(InputState));

    /* 押されているキーを確認 */
    char key = 0;
    if (cli->pending_key != 0) {
        key = cli->pending_key;
        cli->pending_key = 0;
    } else if (KB_HIT()) {
        key = (char)KB_GET();
    }

    if (key != 0) {
        int i;
        cli->last_key = key;  /* 入力確認用に保存 */
        for (i = 0; i < KEY_MAP_SIZE; i++) {
            if (KEY_MAP[i].key == key) {
                out->held[KEY_MAP[i].btn]    = true;
                out->pressed[KEY_MAP[i].btn] = true;
                break;
            }
        }
    }
}

/* ---- 時刻 ------------------------------------------------- */

static uint32_t cli_get_time_ms(struct PlatformAPI *api)
{
    PlatformCLI *cli = (PlatformCLI *)api->user_data;
    long now = (long)clock();
    long diff = now - cli->start_clock;
    /* clock() はクロックティック単位。CLOCKS_PER_SEC でミリ秒に換算 */
    return (uint32_t)((diff * 1000L) / CLOCKS_PER_SEC);
}

/* ---- セーブ ----------------------------------------------- */

static bool cli_save_write(struct PlatformAPI *api,
                            const void *data, uint32_t size)
{
    PlatformCLI *cli = (PlatformCLI *)api->user_data;
    if (size > sizeof(cli->save_buf)) return false;
    memcpy(cli->save_buf, data, size);
    cli->save_size = size;
    return true;
}

static uint32_t cli_save_read(struct PlatformAPI *api,
                               void *out, uint32_t size)
{
    PlatformCLI *cli = (PlatformCLI *)api->user_data;
    if (cli->save_size == 0) return 0;
    uint32_t copy = (size < cli->save_size) ? size : cli->save_size;
    memcpy(out, cli->save_buf, copy);
    return copy;
}

/* ============================================================
 * 公開 API
 * ============================================================ */

void platform_cli_init(PlatformCLI *cli, PlatformAPI *api)
{
    memset(cli, 0, sizeof(PlatformCLI));
    cli->start_clock = (long)clock();

#ifdef _WIN32
    /* Windows コンソールで ANSI エスケープを有効化 */
    win_enable_ansi();
#endif

    api->user_data         = cli;
    api->draw_begin        = cli_draw_begin;
    api->draw_end          = cli_draw_end;
    api->draw_rect         = cli_draw_rect;
    api->draw_text         = cli_draw_text;
    api->draw_image        = cli_draw_image;
    api->audio_play        = cli_audio_play;
    api->audio_stop        = cli_audio_stop;
    api->audio_set_volume  = cli_audio_set_volume;
    api->get_input         = cli_get_input;
    api->get_time_ms       = cli_get_time_ms;
    api->save_write        = cli_save_write;
    api->save_read         = cli_save_read;
}