/**
 * @file platform.h
 * @brief PlatformAPI構造体 — エンジン↔HAL（ハードウェア抽象層）のインターフェース定義
 *
 * エンジンはこの構造体を通じてのみ画面・音・入力・時刻にアクセスする。
 * プラットフォーム側（SDL2 / WASM / 組み込み / CLI）はこの構造体を
 * 実装してエンジンに渡す。
 *
 * エンジンはプラットフォーム固有ヘッダを一切 include しない。
 */

#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>   /* uint8_t, uint32_t など */
#include <stdbool.h>  /* bool */

/* ============================================================
 * 描画関連の型定義
 * ============================================================ */

/**
 * @brief 色（RGBA 各 0–255）
 */
typedef struct {
    uint8_t r, g, b, a;
} Color;

/**
 * @brief 矩形領域
 */
typedef struct {
    int x, y;          /**< 左上座標 */
    int w, h;          /**< 幅・高さ */
} Rect;

/**
 * @brief テキスト描画アラインメント
 */
typedef enum {
    TEXT_ALIGN_LEFT = 0,
    TEXT_ALIGN_CENTER,
    TEXT_ALIGN_RIGHT
} TextAlign;

/* ============================================================
 * 入力関連の型定義
 * ============================================================ */

/**
 * @brief ボタン識別子
 *
 * スロット機の物理ボタンに対応する論理ID。
 * プラットフォームはこれにキー/GPIO を割り当てる。
 */
typedef enum {
    BTN_LEVER = 0,    /**< レバー（スタート） */
    BTN_STOP_0,       /**< 第1停止ボタン */
    BTN_STOP_1,       /**< 第2停止ボタン */
    BTN_STOP_2,       /**< 第3停止ボタン */
    BTN_BET_1,        /**< 1BET */
    BTN_BET_MAX,      /**< MAXベット */
    BTN_MENU,         /**< メニュー／設定 */
    BTN_COUNT         /**< ボタン総数（配列サイズ用） */
} ButtonID;

/**
 * @brief ボタン状態スナップショット
 *
 * エンジンが毎フレーム platform.get_input() で取得する。
 * pressed  = 今フレームだけ押された（エッジ）
 * held     = 押しっぱなし
 */
typedef struct {
    bool pressed[BTN_COUNT];   /**< 押し下げエッジ（このフレームで新たに押された） */
    bool held[BTN_COUNT];      /**< 押下継続中 */
} InputState;

/* ============================================================
 * 音声関連の型定義
 * ============================================================ */

/**
 * @brief サウンドID（プラットフォームが管理するサウンドの識別子）
 * 値は platform_xxx_load_sound() が返す不透明ハンドル。
 */
typedef uint32_t SoundID;

#define SOUND_ID_INVALID  0xFFFFFFFFu  /**< 無効なサウンドID */

/* ============================================================
 * PlatformAPI — エンジン↔HALの契約構造体
 * ============================================================ */

/**
 * @brief HAL関数テーブル
 *
 * プラットフォーム側でこの構造体のインスタンスを1つ作り、
 * 各フィールドに実装関数のポインタを設定してエンジンに渡す。
 */
typedef struct PlatformAPI {

    /* ---- プラットフォーム固有データ ----------------------- */
    /**
     * プラットフォーム実装が自由に使える内部データへのポインタ。
     * エンジンはこれを各コールバックにそのまま渡すだけで中身は触らない。
     * SDL2 なら SDL_Renderer* など、組み込みなら LCD制御構造体など。
     */
    void *user_data;

    /* ---- 描画 API ----------------------------------------- */

    /**
     * @brief フレーム描画開始
     * 毎フレームの描画処理の前に呼ぶ。バックバッファクリアなどを行う。
     */
    void (*draw_begin)(struct PlatformAPI *api);

    /**
     * @brief フレーム描画終了・画面反映
     * 毎フレームの描画処理の後に呼ぶ。スワップ・フラッシュなどを行う。
     */
    void (*draw_end)(struct PlatformAPI *api);

    /**
     * @brief 矩形塗りつぶし
     * @param rect   塗りつぶす領域
     * @param color  塗りつぶし色
     */
    void (*draw_rect)(struct PlatformAPI *api, Rect rect, Color color);

    /**
     * @brief テキスト描画
     * @param x, y    描画基準座標
     * @param text    描画する文字列（UTF-8）
     * @param color   文字色
     * @param size    文字サイズ（ポイントまたは相対スケール。実装依存）
     * @param align   アラインメント
     */
    void (*draw_text)(struct PlatformAPI *api,
                      int x, int y,
                      const char *text,
                      Color color,
                      int size,
                      TextAlign align);

    /**
     * @brief 画像描画
     * @param image_id  platform_xxx_load_image() が返したID
     * @param src       ソース矩形（NULLで画像全体）
     * @param dst       描画先矩形
     */
    void (*draw_image)(struct PlatformAPI *api,
                       uint32_t image_id,
                       const Rect *src,
                       Rect dst);

    /* ---- 音声 API ----------------------------------------- */

    /**
     * @brief サウンド再生
     * @param sound_id  platform_xxx_load_sound() が返したID
     * @param loop      ループ再生するか
     */
    void (*audio_play)(struct PlatformAPI *api, SoundID sound_id, bool loop);

    /**
     * @brief サウンド停止
     * @param sound_id  停止するサウンドのID
     */
    void (*audio_stop)(struct PlatformAPI *api, SoundID sound_id);

    /**
     * @brief 音量設定
     * @param volume  0.0（無音）〜 1.0（最大）
     */
    void (*audio_set_volume)(struct PlatformAPI *api, float volume);

    /* ---- 入力 API ----------------------------------------- */

    /**
     * @brief 入力状態の取得
     * エンジンが毎フレーム呼ぶ。チャタリング除去済みの状態を返す。
     * @param out  結果を書き込む InputState へのポインタ
     */
    void (*get_input)(struct PlatformAPI *api, InputState *out);

    /* ---- 時刻 API ----------------------------------------- */

    /**
     * @brief 現在時刻（ミリ秒）
     * エンジン起動からの経過時間。タイミング判定に使う。
     * @return 経過ミリ秒（単調増加）
     */
    uint32_t (*get_time_ms)(struct PlatformAPI *api);

    /* ---- ファイル / 永続化 API ---------------------------- */

    /**
     * @brief セーブデータ書き込み
     * @param data  書き込むバイト列
     * @param size  バイト数
     * @return 成功なら true
     */
    bool (*save_write)(struct PlatformAPI *api,
                       const void *data, uint32_t size);

    /**
     * @brief セーブデータ読み込み
     * @param out   読み込み先バッファ
     * @param size  バッファサイズ（バイト）
     * @return 実際に読み込んだバイト数（0 = データなし）
     */
    uint32_t (*save_read)(struct PlatformAPI *api,
                          void *out, uint32_t size);

} PlatformAPI;

#endif /* PLATFORM_H */