/**
 * @file effect_mgr.h
 * @brief 演出キュー管理 API
 *
 * タイトル側が「この演出を再生してほしい」というリクエストを
 * キューに積み、エンジン（または描画層）が順番に処理する。
 * 演出の具体的な再生はレンダラが行い、このモジュールは
 * キューの管理と優先度制御のみを担当する。
 */

#ifndef EFFECT_MGR_H
#define EFFECT_MGR_H

#include <stdint.h>
#include <stdbool.h>

/* ============================================================
 * 定数
 * ============================================================ */

/** 演出キューの最大エントリ数 */
#define EFFECT_QUEUE_MAX  16

/* ============================================================
 * 演出リクエスト
 * ============================================================ */

/**
 * @brief 演出の種類
 *
 * タイトル固有の演出は EFFECT_TITLE_BASE 以上の値を使う。
 */
typedef enum {
    EFFECT_NONE = 0,       /**< 演出なし（ダミー） */
    EFFECT_SPIN_START,     /**< リール回転開始 */
    EFFECT_REEL_STOP,      /**< リール停止 */
    EFFECT_WIN_FLASH,      /**< 入賞フラッシュ */
    EFFECT_BONUS_IN,       /**< ボーナス突入 */
    EFFECT_COIN_OUT,       /**< コイン払い出し */
    EFFECT_TITLE_BASE = 100 /**< タイトル固有演出のベース値 */
} EffectType;

/**
 * @brief 演出優先度
 *
 * 高い優先度の演出は割り込んで再生される（実装は描画層に委ねる）。
 */
typedef enum {
    EFFECT_PRIO_LOW = 0,
    EFFECT_PRIO_NORMAL,
    EFFECT_PRIO_HIGH
} EffectPriority;

/**
 * @brief 1件の演出リクエスト
 */
typedef struct {
    EffectType     type;      /**< 演出種別 */
    EffectPriority priority;  /**< 優先度 */
    uint32_t       param;     /**< 演出固有パラメータ（任意用途） */
    uint32_t       duration_ms; /**< 推奨再生時間（ms）、0 = 演出側任せ */
} EffectRequest;

/* ============================================================
 * 演出マネージャ
 * ============================================================ */

/**
 * @brief 演出キュー管理構造体
 */
typedef struct {
    EffectRequest queue[EFFECT_QUEUE_MAX]; /**< リングバッファ */
    uint8_t       head;   /**< 次に読み出すインデックス */
    uint8_t       tail;   /**< 次に書き込むインデックス */
    uint8_t       count;  /**< キュー内のエントリ数 */
} EffectMgr;

/* ============================================================
 * API
 * ============================================================ */

/**
 * @brief 演出マネージャを初期化する
 *
 * @param mgr  初期化する EffectMgr へのポインタ
 */
void effect_mgr_init(EffectMgr *mgr);

/**
 * @brief 演出リクエストをキューに追加する
 *
 * キューが満杯の場合はリクエストを破棄して false を返す。
 *
 * @param mgr  EffectMgr へのポインタ
 * @param req  追加するリクエスト
 * @return true = 成功、false = キュー満杯で破棄
 */
bool effect_mgr_push(EffectMgr *mgr, EffectRequest req);

/**
 * @brief キューの先頭から演出リクエストを取り出す
 *
 * キューが空の場合は out->type = EFFECT_NONE を設定して false を返す。
 *
 * @param mgr  EffectMgr へのポインタ
 * @param out  取り出したリクエストの書き込み先
 * @return true = 取り出し成功、false = キュー空
 */
bool effect_mgr_pop(EffectMgr *mgr, EffectRequest *out);

/**
 * @brief キューにエントリが残っているか確認する
 *
 * @param mgr  EffectMgr へのポインタ（変更しない）
 * @return true = エントリあり
 */
bool effect_mgr_has_pending(const EffectMgr *mgr);

/**
 * @brief キューを空にする
 *
 * @param mgr  EffectMgr へのポインタ
 */
void effect_mgr_clear(EffectMgr *mgr);

#endif /* EFFECT_MGR_H */