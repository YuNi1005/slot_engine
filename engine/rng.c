/**
 * @file rng.c
 * @brief 乱数生成器実装（xorshift64）
 *
 * xorshift64 は Marsaglia (2003) が提案した高速な疑似乱数アルゴリズム。
 * 周期は 2^64 - 1 で、スロット用途の抽選には十分な品質を持つ。
 * ゼロ状態は禁止（ゼロのまま回すと永遠に 0 が出る）。
 */

#include "rng.h"

/* デフォルトシード（任意の非ゼロ値） */
#define RNG_DEFAULT_SEED  0xDEADBEEFCAFEBABEULL

/* ============================================================
 * 内部ヘルパー
 * ============================================================ */

/**
 * @brief xorshift64 の 1 ステップ
 *
 * 標準的なシフト量 {13, 7, 17} を使用。
 * state は呼び出し後に更新され、新しい値が返る。
 */
static uint64_t xorshift64(uint64_t *state)
{
    uint64_t x = *state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    *state = x;
    return x;
}

/* ============================================================
 * API 実装
 * ============================================================ */

void rng_init(RNG *rng)
{
    rng->state = RNG_DEFAULT_SEED;
}

void rng_seed(RNG *rng, uint64_t seed)
{
    /* xorshift64 はゼロ状態を禁止する */
    rng->state = (seed != 0) ? seed : RNG_DEFAULT_SEED;
}

uint64_t rng_next(RNG *rng)
{
    return xorshift64(&rng->state);
}

uint32_t rng_range(RNG *rng, uint32_t range)
{
    if (range == 0) {
        return 0;
    }

    /*
     * 剰余バイアスを最小化するため、上位 32 ビットを使う。
     * 64 ビット全体を range で割り、商の範囲でサンプリングする方が
     * 厳密だが、スロット用途では上位ビット法で十分。
     */
    uint64_t val = rng_next(rng);
    return (uint32_t)((val >> 32) % range);
}