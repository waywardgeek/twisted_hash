#include <stdint.h>
#include <immintrin.h>

typedef __m256i uint256_t;

struct uint512_st {
  uint256_t l;
  uint256_t u;
};

typedef struct uint512_st uint512_t;

struct uint1024_st {
  uint512_t l;
  uint512_t u;
};

typedef struct uint1024_st uint1024_t;

static inline uint512_t uint512_zero() {
  uint512_t v;
  v.l = _mm256_setzero_si256();
  v.u = _mm256_setzero_si256();
  return v;
}

static inline uint1024_t uint1024_zero() {
  uint1024_t v;
  v.l = uint512_zero();
  v.u = uint512_zero();
  return v;
}

static inline uint512_t uint512_XOR(uint512_t a, uint512_t b) {
  uint512_t v;
  v.l = _mm256_xor_si256(a.l, b.l);
  v.u = _mm256_xor_si256(a.u, b.u);
  return v;
}

static inline uint1024_t uint1024_XOR(uint1024_t a, uint1024_t b) {
  uint1024_t v;
  v.l = uint512_XOR(a.l, b.l);
  v.u = uint512_XOR(a.u, b.u);
  return v;
}

static inline uint64_t cpucycles(void) {
  uint64_t result;
  __asm__ __volatile__(
    "rdtsc\n"
    "shlq $32,%%rdx\n"
    "orq %%rdx,%%rax"
    : "=a" ( result ) ::  "%rdx"
  );
  return result;
}
