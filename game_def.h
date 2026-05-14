/**
 * @file game_def.h
 * @brief SlotGameDef構造体 — タイトル↔エンジンのインターフェース定義
 *
 * タイトル側はこの構造体にコールバック関数を詰めてエンジンに渡す。
 * エンジン側はこの構造体を通じてタイトルの処理を呼び出す。
 * タイトルはエンジン内部を直接触らない。
 */

#ifndef GAME_DEF_H
#define GAME_DEF_H

#include <stdint.h>  /* uint8_t, uint32_t など */

/* ============================================================
 * 定数
 * ============================================================ */

#define SLOT_MAX_REELS      3    /**< リール本数（最大） */
#define SLOT_MAX_SYMBOLS    32   /**< 1リール上のシンボル種類数（最大） */
#define SLOT_REEL_LEN_MAX   32   /**< 1リールのコマ数（最大） */
#define SLOT_MAX_PAYLINES   9    /**< 入賞ライン数（最大） */

/* ============================================================
 * 基本型エイリアス
 * ============================================================ */

/** コマ番号（リール上の位置、0始まり） */
typedef uint8_t SymbolID;

/** ゲームカウンタ（ゲーム数・G数カウント用） */
typedef uint32_t GameCount;

/* ============================================================
 * スロット状態フラグ
 * ============================================================ */

/**
 * @brief スロット機本体の状態
 *
 * エンジンがゲームの進行を管理するためのフェーズ。
 * タイトルはこれを参照して演出・表示を切り替える。
 */
typedef enum {
    SLOT_STATE_IDLE = 0,    /**< 待機中（コイン投入待ち） */
    SLOT_STATE_BET,         /**< ベット受付中 */
    SLOT_STATE_SPINNING,    /**< リール回転中 */
    SLOT_STATE_STOPPING,    /**< リール停止処理中 */
    SLOT_STATE_JUDGING,     /**< 役判定中 */
    SLOT_STATE_PAYOUT,      /**< 払い出し処理中 */
    SLOT_STATE_BONUS,       /**< ボーナスゲーム中 */
    SLOT_STATE_ERROR        /**< エラー状態 */
} SlotPhase;

/* ============================================================
 * リール配列定義
 * ============================================================ */

/**
 * @brief 1本のリール配列定義
 */
typedef struct {
    const SymbolID *symbols;   /**< シンボルIDの配列（コマ順） */
    uint8_t         length;    /**< コマ数 */
} ReelStrip;

/* ============================================================
 * 停止結果
 * ============================================================ */

/**
 * @brief 全リールの停止コマ番号
 */
typedef struct {
    uint8_t pos[SLOT_MAX_REELS];   /**< 各リールの停止位置（コマ番号） */
} StopResult;

/* ============================================================
 * 入賞情報
 * ============================================================ */

/**
 * @brief 1件の入賞結果
 */
typedef struct {
    uint8_t  payline_id;    /**< 入賞したラインのID */
    uint8_t  symbol_id;     /**< 入賞シンボルID */
    uint16_t payout;        /**< 払い出し枚数 */
} WinEntry;

/* ============================================================
 * スロット共有状態（エンジン管理）
 * ============================================================ */

/**
 * @brief エンジンが管理するゲーム共通状態
 *
 * タイトル側のコールバックに読み取り専用で渡される。
 * タイトルはこれを書き換えず、自分のタイトル固有状態は
 * title_state ポインタで管理する。
 */
typedef struct {
    SlotPhase   phase;          /**< 現在のゲームフェーズ */
    uint32_t    credits;        /**< 残りクレジット枚数 */
    uint8_t     bet;            /**< 現在のベット枚数 */
    GameCount   total_games;    /**< 累計ゲーム数 */
    StopResult  stop;           /**< 直前の停止結果 */
    WinEntry    wins[SLOT_MAX_PAYLINES]; /**< 直前の入賞リスト */
    uint8_t     win_count;      /**< 今回の入賞件数 */
    uint32_t    total_payout;   /**< 今回の総払い出し枚数 */
} SlotState;

/* ============================================================
 * SlotGameDef — タイトル↔エンジンの契約構造体
 * ============================================================ */

/**
 * @brief タイトル定義構造体
 *
 * タイトル側でこの構造体のインスタンスを1つ作り、
 * 各フィールドに関数ポインタ・定数を設定してエンジンに渡す。
 *
 * コールバックの呼び出し順序（1ゲームの流れ）:
 *   on_init → [ループ開始]
 *     on_bet → on_spin_start →
 *     on_stop_request（各リール）→
 *     on_judge → on_payout →
 *     on_frame_update（毎フレーム）
 *   → [ループ終了] → on_shutdown
 */
typedef struct SlotGameDef {

    /* ---- メタ情報 ----------------------------------------- */
    const char *title_name;     /**< タイトル名（ログ・デバッグ用） */
    const char *version;        /**< バージョン文字列 (例: "1.0.0") */

    /* ---- リール定義 --------------------------------------- */
    const ReelStrip *reels;     /**< リール配列定義の先頭ポインタ */
    uint8_t          reel_count;/**< リール本数 */

    /* ---- タイトル固有状態 --------------------------------- */
    /**
     * タイトル固有の状態構造体へのポインタ。
     * エンジンはこれを void* として保持するだけで中身は触らない。
     * タイトル側のコールバックが自分でキャストして使う。
     */
    void *title_state;

    /* ---- ライフサイクルコールバック ----------------------- */

    /**
     * @brief 初期化
     * エンジン起動時に1回だけ呼ばれる。
     * タイトル固有状態の初期化・リール設定などを行う。
     * @param state  エンジン共有状態（初期値）
     * @param self   この SlotGameDef 自身へのポインタ
     */
    void (*on_init)(SlotState *state, struct SlotGameDef *self);

    /**
     * @brief 終了処理
     * エンジン終了時に1回だけ呼ばれる。
     * 動的確保したリソースを解放する。
     */
    void (*on_shutdown)(struct SlotGameDef *self);

    /* ---- ゲームフロー コールバック ------------------------ */

    /**
     * @brief ベット処理
     * プレイヤーがベットを確定したときに呼ばれる。
     * @return 実際にベットする枚数（0 = ベット拒否）
     */
    uint8_t (*on_bet)(const SlotState *state, struct SlotGameDef *self);

    /**
     * @brief スピン開始
     * リール回転を開始する直前に呼ばれる。
     * 乱数を引いて内部抽選を行うタイミング。
     */
    void (*on_spin_start)(SlotState *state, struct SlotGameDef *self);

    /**
     * @brief 停止要求（リールごと）
     * プレイヤーが停止ボタンを押したときにリールごとに呼ばれる。
     * @param reel_index  停止するリールの番号（0始まり）
     * @return 停止させるコマ番号
     */
    uint8_t (*on_stop_request)(const SlotState *state,
                               struct SlotGameDef *self,
                               uint8_t reel_index);

    /**
     * @brief 役判定
     * 全リール停止後に呼ばれる。
     * state->stop を見て入賞を判定し、state->wins / win_count を埋める。
     */
    void (*on_judge)(SlotState *state, struct SlotGameDef *self);

    /**
     * @brief 払い出し処理
     * 役判定後に呼ばれる。演出開始の合図にも使われる。
     * @return 払い出す枚数（0 = 払い出しなし）
     */
    uint32_t (*on_payout)(SlotState *state, struct SlotGameDef *self);

    /* ---- 毎フレーム更新 ----------------------------------- */

    /**
     * @brief フレーム更新
     * エンジンのメインループから毎フレーム呼ばれる。
     * 演出状態の更新・タイマー管理などに使う。
     * @param delta_ms  前フレームからの経過時間（ミリ秒）
     */
    void (*on_frame_update)(SlotState *state,
                            struct SlotGameDef *self,
                            uint32_t delta_ms);

} SlotGameDef;

#endif /* GAME_DEF_H */