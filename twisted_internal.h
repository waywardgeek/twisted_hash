#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "uint1024.h"

#define FH_BLOCK_SIZE 128
#define FH_NUM_LANES 4
#define FH_PARALLEL_BLOCK_SIZE (FH_BLOCK_SIZE * FH_NUM_LANES)
#define FH_GRAPH_SIZE 8
#define FH_SUPERBLOCK_SIZE (FH_PARALLEL_BLOCK_SIZE * FH_GRAPH_SIZE)

// Internal state, aligned to 32 byte boundary.
struct fhContextInt_st {
  uint1024_t state[FH_NUM_LANES];
  uint1024_t buffer[FH_NUM_LANES * FH_GRAPH_SIZE];
  uint32_t num_bytes;
  bool compressed;
};

typedef struct fhContextInt_st fhContextInt;
