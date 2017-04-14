#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "blake2.h"
#include "blake2b-common.h"
#include "blake2b-load-avx2-simple.h"

#define BLAKE2B_G_V4(a, b, c, d) do {                       \
  a = ADD(a, b); d = XOR(d, a); d = ROT32(d);                        \
  c = ADD(c, d); b = XOR(b, c); b = ROT24(b);                        \
  a = ADD(a, b); d = XOR(d, a); d = ROT16(d);                        \
  c = ADD(c, d); b = XOR(b, c); b = ROT63(b);                        \
} while(0)

#define BLAKE2B_ROUND_V4(v) do {                \
  BLAKE2B_G_V4(v[ 0], v[ 4], v[ 8], v[12]);  \
  BLAKE2B_G_V4(v[ 1], v[ 5], v[ 9], v[13]);  \
  BLAKE2B_G_V4(v[ 2], v[ 6], v[10], v[14]);  \
  BLAKE2B_G_V4(v[ 3], v[ 7], v[11], v[15]);  \
  BLAKE2B_G_V4(v[ 0], v[ 5], v[10], v[15]);  \
  BLAKE2B_G_V4(v[ 1], v[ 6], v[11], v[12]);  \
  BLAKE2B_G_V4(v[ 2], v[ 7], v[ 8], v[13]);  \
  BLAKE2B_G_V4(v[ 3], v[ 4], v[ 9], v[14]);  \
} while(0)

void printhex(char* label, const void* data, size_t datalen);

void blake2bp_permute2(__m256i z[16]) {
  BLAKE2B_ROUND_V4(z);
  BLAKE2B_ROUND_V4(z);
}

void blake2bp_permute3(__m256i z[16]) {
  BLAKE2B_ROUND_V4(z);
  BLAKE2B_ROUND_V4(z);
  BLAKE2B_ROUND_V4(z);
}

void blake2bp_permute12(__m256i z[16]) {
  int r;
  for(r = 0; r < 12; ++r) {
    BLAKE2B_ROUND_V4(z);
  }
}
