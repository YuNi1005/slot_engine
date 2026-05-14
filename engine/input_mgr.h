/**
 * @file input_mgr.h
 * @brief 入力管理 API
 *
 * PlatformAPI.get_input() が返す生の InputState から
 * チャタリング除去・押し下げエッジ検出を行い、
 * エンジン上位層に「このフレームで新たに押されたボタン」を提供する。
 */

#ifndef INPUT_MGR_H
#define INPUT_MGR_H

#include <stdbool.h>
#include "../platform.h"

/* ============================================================
 * 定数
 * ============================================================ */

/**
 * @brief チャタリング除去フィルタのサンプル数
 *
 * 連続してこの回数だけ同じ状態が確認されて初めて確定とする。
 * 60fps × 3 ≒ 50ms のチャタリングを吸収する。
 */
#define INPUT_DEBOUNCE_FRAMES  3

/* ============================================================
 * 入力マネージャ
 * ============================================================ */

/**
 * @brief 入力管理構造体
 *
 * エンジンが 1 つ保持する。
 */
typedef struct {
    InputState current;   /**< 今フレームの確定入力 */
    InputState previous;  /**< 前フレームの確定入力 */
    uint8_t debounce_count[BTN_COUNT]; /**< 各ボタンのチャタリングカウンタ */
    bool raw_held[BTN_COUNT];          /**< 生の held 状態（前フレーム） */
} InputMgr;

/* ============================================================
 * API
 * ============================================================ */

/**
 * @brief 入力マネージャを初期化する
 *
 * @param mgr  初期化する InputMgr へのポインタ
 */
void input_mgr_init(InputMgr *mgr);

/**
 * @brief 毎フレーム呼ぶ。プラットフォームから入力を取得・処理する
 *
 * 内部で PlatformAPI.get_input() を呼んでチャタリング除去を行う。
 * エンジンのフレーム先頭で必ず呼ぶこと。
 *
 * @param mgr  InputMgr へのポインタ
 * @param api  PlatformAPI へのポインタ
 */
void input_mgr_update(InputMgr *mgr, struct PlatformAPI *api);

/**
 * @brief このフレームで新たにボタンが押されたか（押し下げエッジ）
 *
 * @param mgr  InputMgr へのポインタ（変更しない）
 * @param btn  確認するボタン ID
 * @return true = このフレームで押し下げエッジあり
 */
bool input_mgr_pressed(const InputMgr *mgr, ButtonID btn);

/**
 * @brief ボタンが押し続けられているか（held）
 *
 * @param mgr  InputMgr へのポインタ（変更しない）
 * @param btn  確認するボタン ID
 * @return true = 押しっぱなし
 */
bool input_mgr_held(const InputMgr *mgr, ButtonID btn);

#endif /* INPUT_MGR_H */