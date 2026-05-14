/**
 * @file rng.h
 * @brief 乱数生成 API（xorshift64 ベース）
 *
 * エンジン内部で使う疑似乱数生成器。
 * シード注入に対応しているので再現テストが可能。
 * スレッド安全ではない（シングルスレッド前提）。
 */

#ifndef RNG_H
#define RNG_H

#include <stdint.h>

/* ============================================================
 * 状態構造体
 * ============================================================ */

/**
 * @brief 乱数生成器の内部状態
 *
 * この構造体を直接操作してはならない。
 * rng_init() / rng_seed() 経由でのみ初期化する。
 */
typedef struct {
    uint64_t state; /**< xorshift64 の内部レジスタ */
} RNG;

/* ============================================================
 * API
 * ============================================================ */

/**
 * @brief 乱数生成器を初期化する（デフォルトシード）
 *
 * デフォルトシードは固定値（0xDEADBEEFCAFEBABEULL）。
 * 再現性が不要な場合は platform の get_time_ms() を
 * シードとして rng_seed() を呼ぶ。
 *
 * @param rng  初期化する RNG 構造体へのポインタ
 */
void rng_init(RNG *rng);

/**
 * @brief 任意のシードで乱数生成器を初期化する
 *
 * seed = 0 の場合はデフォルトシードに置き換える（0 は xorshift64 の禁止値）。
 *
 * @param rng   初期化する RNG 構造体へのポインタ
 * @param seed  シード値
 */
void rng_seed(RNG *rng, uint64_t seed);

/**
 * @brief 64ビット乱数を生成する
 *
 * @param rng  乱数生成器へのポインタ
 * @return ランダムな uint64_t 値
 */
uint64_t rng_next(RNG *rng);

/**
 * @brief [0, range) の範囲で一様な乱数を生成する
 *
 * range = 0 の場合は 0 を返す（未定義動作防止）。
 * 剰余バイアスを避けるため上位ビット切り捨て法を使用。
 *
 * @param rng    乱数生成器へのポインタ
 * @param range  生成範囲の上限（この値自体は含まない）
 * @return [0, range) の乱数値
 */
uint32_t rng_range(RNG *rng, uint32_t range);

#endif /* RNG_H */