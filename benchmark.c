#include "twistedhash.h"
#include "uint1024.h"

#include <stdio.h>
#include <stdlib.h>

void printhex(char* label, const void* data, size_t datalen) {
  const uint8_t* p = data;
  printf("%s:", label);
  uint32_t i;
  for (i = 0; i < datalen; ++i) {
    if (i % 32 == 0) {
      printf("\n %u: ", i/32);
    } else if (i % 8 == 0) {
      printf(" ");
    }
    printf("%02x", *p);
    ++p;
  }
  printf("\n");
}

static uint64_t runBenchmark(size_t length) {
  uint32_t i;
  uint8_t *data = calloc(length, sizeof(uint8_t));
  uint32_t *p = (void*)data;
  uint1024_t digest;

  for (i = 0; i < length/sizeof(uint32_t); i++) {
    p[i] = i;
  }
  fhContext context;
  uint64_t start_cycle = cpucycles();
  fhInit(&context);
  fhUpdate(&context, data, length);
  fhFinal(&context, &digest);
  uint64_t end_cycle = cpucycles();
  free(data);
  return end_cycle - start_cycle;
}

int main() {
  uint64_t loops = 10000;
  uint32_t length;
  for (length = 1; length < 16384; length++) {
    if (length > 4096) {
      loops = 100;
    } else if (length > 600) {
      loops = 1000;
    }
    uint64_t total_cycles = 0;
    uint32_t i;
    for (i = 0; i < loops; i++) {
      total_cycles += runBenchmark(length);
    }
    printf("%u: %.2f cpb\n", length, (float)total_cycles / (loops * length));
  }
  return 0;
}
