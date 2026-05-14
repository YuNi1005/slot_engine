/**
 * @file slot_engine.h
 * @brief スロットエンジン公開 API
 *
 * エントリーポイント（main_xxx.c）とレンダラから見えるインターフェース。
 * タイトルとプラットフォームを渡して init → ループ内で update → shutdown。
 *
 * 使い方:
 *   SlotEngine engine;
 *   slot_engine_init(&engine, &my_game_def, &platform_api);
 *   while (running) {
 *       slot_engine_update(&engine);
 *   }
 *   slot_engine_shutdown(&engine);
 */

#ifndef SLOT_ENGINE_H
#define SLOT_ENGINE_H

#include "../game_def.h"
#include "../platform.h"
#include "rng.h"
#include "timing.h"
#include "reel_ctrl.h"
#include "input_mgr.h"
#include "effect_mgr.h"

/* ============================================================
 * エンジン本体構造体
 * ============================================================ */

/**
 * @brief スロットエンジンの内部状態
 *
 * メインループを持つターゲット（main_xxx.c）がスタックまたは
 * グローバルに 1 インスタンス確保する。
 */
typedef struct {
    SlotGameDef   *game;     /**< タイトル定義（外部所有） */
    struct PlatformAPI *platform; /**< HAL（外部所有） */

    SlotState      state;    /**< エンジン管理のゲーム共有状態 */

    RNG            rng;      /**< 乱数生成器 */
    TimingMgr      timing;   /**< フレームタイミング管理 */
    ReelCtrlMgr    reels;    /**< リール制御 */
    InputMgr       input;    /**< 入力管理 */
    EffectMgr      effects;  /**< 演出キュー */

    bool           running;  /**< false にするとメインループを抜ける */
} SlotEngine;

/* ============================================================
 * API
 * ============================================================ */

/**
 * @brief エンジンを初期化する
 *
 * タイトルの on_init を呼び、各サブシステムをセットアップする。
 * セーブデータがあれば読み込んで state を復元する。
 *
 * @param engine    初期化する SlotEngine へのポインタ
 * @param game      タイトル定義へのポインタ
 * @param platform  HAL へのポインタ
 */
void slot_engine_init(SlotEngine *engine,
                      SlotGameDef *game,
                      struct PlatformAPI *platform);

/**
 * @brief 1フレーム分の更新処理を行う
 *
 * メインループから毎フレーム呼ぶ。
 * 内部で入力取得→ゲーム状態遷移→タイトルコールバック→描画を行う。
 *
 * @param engine  SlotEngine へのポインタ
 */
void slot_engine_update(SlotEngine *engine);

/**
 * @brief エンジンを終了する
 *
 * タイトルの on_shutdown を呼び、セーブデータを書き込む。
 *
 * @param engine  SlotEngine へのポインタ
 */
void slot_engine_shutdown(SlotEngine *engine);

/**
 * @brief エンジンがまだ動作中か返す
 *
 * @param engine  SlotEngine へのポインタ（変更しない）
 * @return true = 動作中、false = 終了要求あり
 */
bool slot_engine_is_running(const SlotEngine *engine);

#endif /* SLOT_ENGINE_H */