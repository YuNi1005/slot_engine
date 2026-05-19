/**
 * @file platform_cli.h
 * @brief CLI プラットフォーム初期化 API
 *
 * stdio だけで動く最小 HAL。
 * 外部ライブラリ依存ゼロなので、どの環境でも必ずビルドできる。
 *
 * 使い方:
 *   PlatformCLI cli;
 *   PlatformAPI api;
 *   platform_cli_init(&cli, &api);
 *   // あとは api を slot_engine_init() に渡すだけ
 */

#ifndef PLATFORM_CLI_H
#define PLATFORM_CLI_H

#include "platform.h"

/* ============================================================
 * CLI プラットフォーム固有データ
 * ============================================================ */

/**
 * @brief CLI プラットフォームの内部状態
 *
 * PlatformAPI.user_data に渡す。
 */
typedef struct {
    /* セーブバッファ（ファイルの代わりにメモリに持つ簡易版） */
    uint8_t  save_buf[256];
    uint32_t save_size;

    /* タイマー基準時刻（clock() の初期値） */
    long     start_clock;

    /* キー入力バッファ（ノンブロッキング用） */
    char     pending_key;   /**< 0 = 未入力 */
    char     last_key;      /**< 最後に押されたキー（表示確認用） */
} PlatformCLI;

/* ============================================================
 * API
 * ============================================================ */

/**
 * @brief CLI プラットフォームを初期化する
 *
 * PlatformCLI と PlatformAPI を両方セットアップする。
 * 以降は api を通じてのみエンジンと通信する。
 *
 * @param cli  PlatformCLI 構造体へのポインタ（呼び出し元が確保）
 * @param api  書き込み先の PlatformAPI へのポインタ
 */
void platform_cli_init(PlatformCLI *cli, PlatformAPI *api);

#endif /* PLATFORM_CLI_H */