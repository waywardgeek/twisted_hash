#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "blake2.h"
#include "blake2b.h"
#include "blake2b-common.h"

#define BLAKE2B_G1_V1(a, b, c, d) do {     \
  a = ADD(a, b); d = XOR(d, a); d = ROT32(d); \
  c = ADD(c, d); b = XOR(b, c); b = ROT24(b); \
} while(0)

#define BLAKE2B_G2_V1(a, b, c, d) do {     \
  a = ADD(a, b); d = XOR(d, a); d = ROT16(d); \
  c = ADD(c, d); b = XOR(b, c); b = ROT63(b); \
} while(0)

#define BLAKE2B_DIAG_V1(a, b, c, d) do {                 \
  d = _mm256_permute4x64_epi64(d, _MM_SHUFFLE(2,1,0,3)); \
  c = _mm256_permute4x64_epi64(c, _MM_SHUFFLE(1,0,3,2)); \
  b = _mm256_permute4x64_epi64(b, _MM_SHUFFLE(0,3,2,1)); \
} while(0)

#define BLAKE2B_UNDIAG_V1(a, b, c, d) do {               \
  d = _mm256_permute4x64_epi64(d, _MM_SHUFFLE(0,3,2,1)); \
  c = _mm256_permute4x64_epi64(c, _MM_SHUFFLE(1,0,3,2)); \
  b = _mm256_permute4x64_epi64(b, _MM_SHUFFLE(2,1,0,3)); \
} while(0)

  #include "blake2b-load-avx2-simple.h"

#define BLAKE2B_ROUND_V1(a, b, c, d, r) do { \
  BLAKE2B_G1_V1(a, b, c, d);                \
  BLAKE2B_G2_V1(a, b, c, d);                \
  BLAKE2B_DIAG_V1(a, b, c, d);                  \
  BLAKE2B_G1_V1(a, b, c, d);                \
  BLAKE2B_G2_V1(a, b, c, d);                \
  BLAKE2B_UNDIAG_V1(a, b, c, d);                \
} while(0)

#define BLAKE2B_ROUNDS_V1(a, b, c, d) do {   \
  BLAKE2B_ROUND_V1(a, b, c, d,  0);        \
  BLAKE2B_ROUND_V1(a, b, c, d,  1);        \
  BLAKE2B_ROUND_V1(a, b, c, d,  2);        \
} while(0)

void blake2b_permute3(__m256i v[4]) {
  __m256i a = v[0];
  __m256i b = v[1];
  __m256i c = v[2];
  __m256i d = v[3];

  BLAKE2B_ROUNDS_V1(a, b, c, d);

  v[0] = a;
  v[1] = b;
  v[2] = c;
  v[3] = d;
}

void blake2b_permute12(__m256i v[4]) {
  __m256i a = v[0];
  __m256i b = v[1];
  __m256i c = v[2];
  __m256i d = v[3];

  BLAKE2B_ROUNDS_V1(a, b, c, d);
  BLAKE2B_ROUNDS_V1(a, b, c, d);
  BLAKE2B_ROUNDS_V1(a, b, c, d);
  BLAKE2B_ROUNDS_V1(a, b, c, d);

  v[0] = a;
  v[1] = b;
  v[2] = c;
  v[3] = d;
}
