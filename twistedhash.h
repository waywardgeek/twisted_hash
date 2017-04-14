#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define FH_DIGEST_SIZE 32

struct fhContext_st {
  uint32_t offset;  // 0 to 31 offset into buf to align on 32-byte boundary.
  uint8_t buffer[
    /*alignment*/ 31 +
    /*state*/ 4*128 +
    /*buffer*/ 4*128*8 +
    /*num_bytes*/ 4 +
    /*compressed*/ 1
  ];
};

typedef struct fhContext_st fhContext;

void fhInit(fhContext *context);
void fhUpdate(fhContext *context, const void *data, size_t datalen);
void fhFinal(fhContext *context, void *digest);
void fhFastHash(void *data, size_t datalen, void *digest);
