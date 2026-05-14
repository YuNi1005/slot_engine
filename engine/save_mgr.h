/**
 * @file save_mgr.h
 * @brief セーブ管理 API
 *
 * SlotState をシリアライズして永続化する。
 * 実際の読み書きは PlatformAPI.save_write / save_read に委ねる。
 * データの先頭に CRC-32 を付加してデータ破損を検出する。
 */

#ifndef SAVE_MGR_H
#define SAVE_MGR_H

#include <stdint.h>
#include <stdbool.h>
#include "../game_def.h"
#include "../platform.h"

/* ============================================================
 * セーブデータフォーマット（内部構造）
 * ============================================================ */

/**
 * @brief セーブデータのヘッダ
 *
 * バイナリ先頭に配置する。マジックナンバーとバージョンで
 * 互換性チェックを行い、CRC でデータ整合性を検証する。
 */
typedef struct {
    uint32_t magic;       /**< マジックナンバー: 0x534C4F54 ('SLOT') */
    uint16_t version;     /**< セーブフォーマットバージョン */
    uint16_t data_size;   /**< SlotState のバイトサイズ */
    uint32_t crc32;       /**< SlotState データの CRC-32 */
} SaveHeader;

#define SAVE_MAGIC    0x534C4F54u  /**< 'SLOT' */
#define SAVE_VERSION  1u           /**< 現在のフォーマットバージョン */

/* ============================================================
 * API
 * ============================================================ */

/**
 * @brief SlotState をセーブする
 *
 * ヘッダ（マジック・バージョン・CRC）を付加してから
 * PlatformAPI.save_write() で書き込む。
 *
 * @param state  保存する SlotState へのポインタ
 * @param api    PlatformAPI へのポインタ
 * @return true = 成功
 */
bool save_mgr_save(const SlotState *state, struct PlatformAPI *api);

/**
 * @brief セーブデータを読み込んで SlotState に復元する
 *
 * マジック・バージョン・CRC を検証する。いずれかが不正なら
 * false を返し、out の内容は変更しない。
 *
 * @param out  復元先の SlotState へのポインタ
 * @param api  PlatformAPI へのポインタ
 * @return true = 正常に復元できた
 */
bool save_mgr_load(SlotState *out, struct PlatformAPI *api);

/**
 * @brief CRC-32 を計算する（テスト・検証用に公開）
 *
 * 多項式: 0xEDB88320（IEEE 802.3）
 *
 * @param data  計算対象のバイト列
 * @param size  バイト数
 * @return CRC-32 値
 */
uint32_t save_mgr_crc32(const void *data, uint32_t size);

#endif /* SAVE_MGR_H */