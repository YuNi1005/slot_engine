/**
 * @file save_mgr.c
 * @brief セーブデータ管理実装
 *
 * シリアライズは SlotState をそのままバイト列として扱う
 * （エンディアン固定のプラットフォームを前提とした簡易実装）。
 * 異なるアーキテクチャ間でのデータ移行は想定外。
 *
 * セーブデータのバイト構成:
 *   [ SaveHeader (12 bytes) ][ SlotState (可変) ]
 */

#include "save_mgr.h"
#include <string.h>  /* memcpy */

/* ============================================================
 * CRC-32 実装（テーブルレス、低メモリ版）
 * ============================================================ */

uint32_t save_mgr_crc32(const void *data, uint32_t size)
{
    const uint8_t *p = (const uint8_t *)data;
    uint32_t crc = 0xFFFFFFFFu;
    uint32_t i, bit;

    for (i = 0; i < size; i++) {
        crc ^= p[i];
        for (bit = 0; bit < 8; bit++) {
            if (crc & 1u) {
                crc = (crc >> 1) ^ 0xEDB88320u;  /* IEEE 802.3 多項式 */
            } else {
                crc >>= 1;
            }
        }
    }
    return crc ^ 0xFFFFFFFFu;
}

/* ============================================================
 * API 実装
 * ============================================================ */

bool save_mgr_save(const SlotState *state, struct PlatformAPI *api)
{
    /* ヘッダ構築 */
    SaveHeader hdr;
    hdr.magic     = SAVE_MAGIC;
    hdr.version   = SAVE_VERSION;
    hdr.data_size = (uint16_t)sizeof(SlotState);
    hdr.crc32     = save_mgr_crc32(state, sizeof(SlotState));

    /*
     * ヘッダ + SlotState を 1 つのバッファに詰めて書き込む。
     * 組み込みでスタックが小さい場合はここを要注意。
     * sizeof(SaveHeader) + sizeof(SlotState) ≒ 12 + 約30バイト程度。
     */
    uint8_t buf[sizeof(SaveHeader) + sizeof(SlotState)];
    memcpy(buf,                    &hdr,   sizeof(SaveHeader));
    memcpy(buf + sizeof(SaveHeader), state, sizeof(SlotState));

    return api->save_write(api, buf, (uint32_t)sizeof(buf));
}

bool save_mgr_load(SlotState *out, struct PlatformAPI *api)
{
    uint8_t buf[sizeof(SaveHeader) + sizeof(SlotState)];
    uint32_t read_size;

    read_size = api->save_read(api, buf, (uint32_t)sizeof(buf));
    if (read_size < sizeof(SaveHeader)) {
        return false;  /* データ不足 */
    }

    /* ヘッダ検証 */
    SaveHeader hdr;
    memcpy(&hdr, buf, sizeof(SaveHeader));

    if (hdr.magic != SAVE_MAGIC) {
        return false;  /* マジックナンバー不一致 */
    }
    if (hdr.version != SAVE_VERSION) {
        return false;  /* バージョン不一致 */
    }
    if (hdr.data_size != sizeof(SlotState)) {
        return false;  /* 構造体サイズ不一致（フォーマット変更） */
    }
    if (read_size < sizeof(SaveHeader) + sizeof(SlotState)) {
        return false;  /* データ本体が足りない */
    }

    /* CRC 検証 */
    const uint8_t *data_ptr = buf + sizeof(SaveHeader);
    uint32_t actual_crc = save_mgr_crc32(data_ptr, sizeof(SlotState));
    if (actual_crc != hdr.crc32) {
        return false;  /* データ破損 */
    }

    /* 復元 */
    memcpy(out, data_ptr, sizeof(SlotState));
    return true;
}