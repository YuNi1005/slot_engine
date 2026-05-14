/**
 * @file timing.h
 * @brief タイミング管理 API
 *
 * フレーム時刻の管理とビタ押し精度判定を担当する。
 * 時刻の実取得は PlatformAPI.get_time_ms() に委ねるため、
 * このモジュール自体はプラットフォーム非依存。
 */

#ifndef TIMING_H
#define TIMING_H

#include <stdint.h>
#include <stdbool.h>

/* ============================================================
 * 定数
 * ============================================================ */

/**
 * @brief ビタ押し許容ウィンドウ（ミリ秒）
 *
 * 目標コマが表示窓の中央に来てからこの時間以内に停止ボタンが
 * 押された場合、「ビタ押し成功」と判定する。
 * 実機の許容値（約167ms = 1フレーム）に近い値を設定している。
 */
#define TIMING_VITA_WINDOW_MS  167u

/* ============================================================
 * タイミングマネージャ
 * ============================================================ */

/**
 * @brief タイミング管理構造体
 *
 * エンジンが 1 つ保持する。直接フィールドを書かず、
 * API 関数を通じてのみ操作すること。
 */
typedef struct {
    uint32_t last_frame_ms;   /**< 直前フレームの時刻（ms） */
    uint32_t delta_ms;        /**< 前フレームからの経過時間（ms） */
    uint32_t frame_count;     /**< 起動からの累計フレーム数 */
} TimingMgr;

/* ============================================================
 * API
 * ============================================================ */

/**
 * @brief タイミングマネージャを初期化する
 *
 * @param tmg      初期化する TimingMgr へのポインタ
 * @param now_ms   現在時刻（platform の get_time_ms() の値）
 */
void timing_init(TimingMgr *tmg, uint32_t now_ms);

/**
 * @brief フレーム開始時に呼ぶ。delta_ms を更新する
 *
 * メインループの先頭で毎フレーム呼ぶこと。
 *
 * @param tmg      TimingMgr へのポインタ
 * @param now_ms   現在時刻（platform の get_time_ms() の値）
 */
void timing_tick(TimingMgr *tmg, uint32_t now_ms);

/**
 * @brief 前フレームからの経過時間（ms）を返す
 *
 * @param tmg  TimingMgr へのポインタ
 * @return delta_ms（timing_tick() で更新された値）
 */
uint32_t timing_delta(const TimingMgr *tmg);

/**
 * @brief 起動からの累計フレーム数を返す
 *
 * @param tmg  TimingMgr へのポインタ
 * @return フレームカウント
 */
uint32_t timing_frame_count(const TimingMgr *tmg);

/**
 * @brief ビタ押し成功か判定する
 *
 * @param target_ms   目標コマが窓中央に来た時刻（ms）
 * @param pressed_ms  停止ボタンが押された時刻（ms）
 * @return true = ビタ押し成功、false = 失敗
 */
bool timing_is_vita(uint32_t target_ms, uint32_t pressed_ms);

/**
 * @brief 2 時刻の差（絶対値）を返す（uint32 ラップアラウンド対応）
 *
 * @param a  時刻 A（ms）
 * @param b  時刻 B（ms）
 * @return |a - b| のミリ秒数（最大 UINT32_MAX/2 まで正確）
 */
uint32_t timing_diff_ms(uint32_t a, uint32_t b);

#endif /* TIMING_H */