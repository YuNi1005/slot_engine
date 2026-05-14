/**
 * @file reel_ctrl.h
 * @brief リール制御 API
 *
 * 各リールの回転・停止・コマ送り状態を管理する。
 * 描画（何を表示するか）はレンダラ側が担当し、
 * このモジュールは「どのコマが何番目にあるか」という
 * 論理状態のみを管理する。
 */

#ifndef REEL_CTRL_H
#define REEL_CTRL_H

#include <stdint.h>
#include <stdbool.h>
#include "../game_def.h"

/* ============================================================
 * 定数
 * ============================================================ */

/** 1リールに表示する窓枠のコマ数（上段・中段・下段） */
#define REEL_VISIBLE_ROWS  3

/**
 * @brief リールの回転速度（コマ/秒）
 *
 * 実機は約 80 rpm ≒ 約 0.75 秒で 1 周（21 コマ）なので
 * 21 / 0.75 ≒ 28 コマ/秒。
 */
#define REEL_SPEED_FRAMES_PER_SYMBOL  4  /**< 1コマ進むのに要するフレーム数（60fps想定）*/

/* ============================================================
 * リール状態
 * ============================================================ */

/**
 * @brief 1本のリールの動作状態
 */
typedef enum {
    REEL_IDLE = 0,   /**< 停止中・待機 */
    REEL_SPINNING,   /**< 回転中 */
    REEL_STOPPING,   /**< 停止動作中（目的コマに向かって減速） */
    REEL_STOPPED     /**< 完全停止 */
} ReelState;

/**
 * @brief 1本のリールの制御データ
 */
typedef struct {
    ReelState       state;           /**< 現在の動作状態 */
    const ReelStrip *strip;          /**< リール配列定義（参照のみ） */

    uint8_t         current_pos;     /**< 現在の停止コマ番号（中段基準） */
    uint8_t         target_pos;      /**< 停止目標コマ番号 */

    uint32_t        frame_counter;   /**< コマ送りカウンタ */
    uint32_t        stop_offset;     /**< 停止までの残りコマ数 */
} ReelCtrl;

/* ============================================================
 * リールコントローラ群
 * ============================================================ */

/**
 * @brief 全リールをまとめて管理する構造体
 */
typedef struct {
    ReelCtrl reels[SLOT_MAX_REELS]; /**< 各リールの制御データ */
    uint8_t  count;                 /**< 有効なリール数 */
} ReelCtrlMgr;

/* ============================================================
 * API
 * ============================================================ */

/**
 * @brief リールコントローラを初期化する
 *
 * @param mgr    初期化する ReelCtrlMgr へのポインタ
 * @param strips リール配列定義の先頭ポインタ（game_def から渡す）
 * @param count  リール本数
 */
void reel_ctrl_init(ReelCtrlMgr *mgr,
                    const ReelStrip *strips,
                    uint8_t count);

/**
 * @brief 全リールの回転を開始する
 *
 * IDLE または STOPPED 状態のリールを SPINNING に移行する。
 *
 * @param mgr  ReelCtrlMgr へのポインタ
 */
void reel_ctrl_start_all(ReelCtrlMgr *mgr);

/**
 * @brief 指定リールに停止目標を設定し、停止動作を開始する
 *
 * SPINNING 状態のリールのみ受け付ける。
 * 目標コマまでの最短コマ数を計算して stop_offset に設定する。
 *
 * @param mgr         ReelCtrlMgr へのポインタ
 * @param reel_index  停止するリールの番号（0始まり）
 * @param target_pos  停止目標コマ番号
 */
void reel_ctrl_request_stop(ReelCtrlMgr *mgr,
                            uint8_t reel_index,
                            uint8_t target_pos);

/**
 * @brief 毎フレーム呼ぶ。全リールのコマ送りを進める
 *
 * @param mgr  ReelCtrlMgr へのポインタ
 */
void reel_ctrl_update(ReelCtrlMgr *mgr);

/**
 * @brief 全リールが停止しているか判定する
 *
 * @param mgr  ReelCtrlMgr へのポインタ（変更しない）
 * @return true = 全リール STOPPED、false = 回転中のリールあり
 */
bool reel_ctrl_all_stopped(const ReelCtrlMgr *mgr);

/**
 * @brief 表示窓上のシンボルIDを取得する
 *
 * row = 0 が上段、1 が中段（基準）、2 が下段。
 *
 * @param mgr         ReelCtrlMgr へのポインタ（変更しない）
 * @param reel_index  リール番号（0始まり）
 * @param row         行番号（0=上段, 1=中段, 2=下段）
 * @return シンボルID
 */
SymbolID reel_ctrl_get_symbol(const ReelCtrlMgr *mgr,
                              uint8_t reel_index,
                              uint8_t row);

/**
 * @brief 全リールの停止位置を StopResult に書き出す
 *
 * @param mgr  ReelCtrlMgr へのポインタ（変更しない）
 * @param out  結果を書き込む StopResult へのポインタ
 */
void reel_ctrl_get_stop_result(const ReelCtrlMgr *mgr, StopResult *out);

#endif /* REEL_CTRL_H */