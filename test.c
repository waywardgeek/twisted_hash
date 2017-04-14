#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "blake2b/blake2b.h"
#include "uint1024.h"

static void shuffle(void *state) {
  uint64_t *in = state;
  uint64_t buf[4*16];
  int i, lane;

  for(lane = 0; lane < 4; lane++) {
    uint32_t sourceAddr = 16*lane;
    uint32_t destAddr = lane;
    for(i = 0; i < 16; i++) {
      buf[destAddr] = in[sourceAddr];
      sourceAddr++;
      destAddr += 4;
    }
  }
  memcpy(state, buf, sizeof(buf));
}

static void unshuffle(void *state) {
  uint64_t *in = state;
  uint64_t buf[4*16];
  int i, lane;

  for(lane = 0; lane < 4; lane++) {
    uint32_t sourceAddr = lane;
    uint32_t destAddr = 16*lane;
    for(i = 0; i < 16; i++) {
      buf[destAddr] = in[sourceAddr];
      destAddr++;
      sourceAddr += 4;
    }
  }
  memcpy(state, buf, sizeof(buf));
}

int main() {
  uint32_t i, j;
  uint256_t state1[16];
  uint256_t state2[16];
  uint64_t count;
  uint64_t* p = (void*)state1;
  uint64_t loops = 1 << 20;

  for (count = 0; count < sizeof(state1)/sizeof(uint64_t); count++) {
    p[count] = count;
  }
  p = (void*)state2;
  for (count = 0; count < sizeof(state2)/sizeof(uint64_t); count++) {
    p[count] = count;
  }
  uint64_t start_cycle = cpucycles();
  for (i = 0; i < loops; i++) {
    for (j = 0; j < 16; j += 4) {
      blake2b_permute12(state1 + j);
    }
  }
  uint64_t end_cycle = cpucycles();
  printf("BLAKE2b %.2f cycles per byte\n",
      (float)(end_cycle - start_cycle) / (sizeof(state1) * loops));
  start_cycle = cpucycles();
  shuffle(state2);
  for (i = 0; i < loops; i++) {
    blake2bp_permute12(state2);
  }
  unshuffle(state2);
  end_cycle = cpucycles();
  printf("BLAKE2bp %.2f cycles per byte\n",
      (float)(end_cycle - start_cycle) / (sizeof(state1) * loops));
  if (memcmp(state1, state2, 4*128)) {
    printf("FAILED\n");
  } else {
    printf("PASSED\n");
  }
  return 0;
}
