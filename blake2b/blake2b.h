#ifndef BLAKE2_AVX2_BLAKE2B_H
#define BLAKE2_AVX2_BLAKE2B_H

#include <stddef.h>
#include <immintrin.h>

// Cryptograhically scramble the bits between upper and lower.
void blake2b_permute3(__m256i v[4]);
void blake2b_permute12(__m256i v[4]);
void blake2bp_permute2(__m256i v[16]);
void blake2bp_permute3(__m256i v[16]);
void blake2bp_permute12(__m256i z[16]);

#endif
