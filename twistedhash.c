#include "fasthash.h"
#include "fasthash_internal.h"
#include "blake2b/blake2b.h"

#include <stdio.h>
#include <string.h>

// This is a random IV.  TODO: Replace it with digits of PI.
static uint8_t IV[FH_PARALLEL_BLOCK_SIZE] = {
  96, 144, 73, 140, 220, 120, 131, 111, 154, 251, 231, 35, 172, 139, 46, 243,
  130, 57, 204, 13, 169, 116, 218, 36, 235, 85, 166, 87, 26, 146, 222, 30,
  80, 71, 136, 136, 19, 112, 7, 69, 114, 68, 237, 2, 53, 245, 62, 244,
  187, 75, 237, 79, 56, 208, 133, 144, 17, 47, 202, 85, 96, 101, 132, 5,
  61, 101, 80, 82, 97, 16, 48, 118, 55, 117, 186, 189, 113, 37, 10, 191,
  105, 132, 169, 129, 165, 243, 107, 63, 49, 46, 186, 163, 228, 215, 60, 139,
  183, 62, 249, 182, 119, 22, 200, 222, 200, 248, 135, 179, 255, 126, 157, 113,
  228, 0, 229, 52, 162, 166, 113, 118, 74, 143, 108, 155, 247, 1, 61, 21,
  30, 212, 84, 133, 150, 89, 39, 115, 179, 251, 242, 179, 249, 151, 188, 226,
  7, 242, 33, 23, 32, 229, 69, 62, 185, 77, 3, 151, 133, 102, 186, 227,
  149, 247, 128, 105, 206, 65, 212, 125, 250, 181, 67, 183, 7, 162, 59, 46,
  70, 195, 252, 115, 67, 228, 98, 168, 83, 38, 123, 96, 66, 55, 216, 18,
  236, 143, 175, 12, 19, 140, 188, 121, 37, 25, 179, 172, 114, 44, 186, 58,
  124, 150, 84, 146, 89, 77, 206, 169, 245, 94, 3, 207, 140, 24, 51, 228,
  55, 164, 101, 201, 28, 26, 223, 94, 61, 64, 158, 48, 57, 192, 80, 111,
  40, 37, 118, 158, 151, 238, 65, 72, 95, 93, 214, 131, 42, 244, 103, 185,
  146, 63, 19, 38, 206, 198, 28, 228, 135, 53, 82, 9, 123, 24, 157, 0,
  56, 32, 135, 148, 151, 125, 131, 21, 61, 122, 220, 148, 84, 254, 4, 208,
  124, 168, 161, 129, 76, 40, 25, 86, 14, 1, 148, 255, 115, 117, 35, 84,
  206, 133, 7, 230, 136, 99, 249, 12, 66, 106, 92, 171, 223, 38, 96, 199,
  89, 250, 165, 50, 172, 223, 136, 88, 148, 136, 142, 33, 170, 157, 243, 243,
  130, 226, 89, 20, 155, 246, 136, 137, 30, 54, 24, 141, 69, 219, 26, 121,
  11, 91, 248, 132, 80, 221, 134, 28, 209, 191, 71, 241, 43, 175, 23, 191,
  188, 21, 4, 227, 146, 218, 205, 83, 152, 15, 185, 38, 73, 50, 49, 192,
  147, 59, 97, 96, 3, 127, 190, 96, 220, 190, 229, 168, 143, 47, 105, 72,
  204, 89, 74, 208, 197, 164, 16, 19, 201, 249, 100, 124, 33, 134, 83, 119,
  146, 55, 50, 30, 211, 165, 135, 135, 191, 201, 130, 173, 61, 60, 41, 36,
  174, 242, 129, 229, 50, 84, 162, 116, 248, 179, 96, 188, 50, 34, 207, 93,
  245, 255, 41, 62, 189, 212, 247, 12, 69, 210, 82, 40, 84, 22, 212, 217,
  146, 54, 198, 31, 225, 36, 28, 132, 124, 195, 74, 142, 165, 200, 99, 141,
  241, 197, 107, 130, 74, 75, 81, 156, 136, 2, 91, 216, 180, 240, 145, 42,
  12, 234, 249, 17, 238, 197, 42, 107, 138, 145, 170, 239, 223, 63, 162, 34,
};

static inline fhContextInt *alignContext(fhContext *context) {
  return (fhContextInt*)((uintptr_t)context + context->offset);
}


// Initialize the context.
void fhInit(fhContext *context) {
  uint32_t lowbits = (uintptr_t)context & 31;
  if (lowbits == 0) {
    context->offset = 0;
  } else {
    context->offset = 32 - lowbits;
  }
  fhContextInt *context_int = alignContext(context);
  context_int->num_bytes = 0;
  context_int->compressed = false;
  // Only initialize first lane.  This helps with small hash performance.
  memcpy(context_int->state, IV, FH_BLOCK_SIZE);
}

// XOR 4 parallel uint1024_t values onto the state.
static inline void xor4ParallelUint1024(uint1024_t *dest, uint1024_t *source) {
  *dest = uint1024_XOR(*dest, *source++);
  dest++;
  *dest = uint1024_XOR(*dest, *source++);
  dest++;
  *dest = uint1024_XOR(*dest, *source++);
  dest++;
  *dest = uint1024_XOR(*dest, *source);
}

// Permute a full 4KiB super-block.  This hashes 4 lanes in parallel and the
// twisted hash algorithm to reduce the total number of rounds by 2X.
static void permuteSuperblock(fhContextInt *context_int) {
  uint1024_t *p = context_int->buffer;
  uint1024_t *state = context_int->state;
  if (!context_int->compressed) {
    // We need to initialize the rest of state to IV.
    memcpy(state + 1, IV + FH_BLOCK_SIZE, FH_PARALLEL_BLOCK_SIZE - FH_BLOCK_SIZE);
    context_int->compressed = true;
    if (context_int->num_bytes < FH_SUPERBLOCK_SIZE) {
      memset((uint8_t*)(void*)context_int->buffer + context_int->num_bytes, 0,
             FH_SUPERBLOCK_SIZE - context_int->num_bytes);
    }
  }
  // Absorb pattern is ABCDEFGHEBGDAFCH.
  // Absorb ABCDEFGH
  xor4ParallelUint1024(state, p);
  p += 4;
  blake2bp_permute3(&(state->l.l));
  xor4ParallelUint1024(state, p);
  p += 4;
  blake2bp_permute3(&(state->l.l));
  xor4ParallelUint1024(state, p);
  p += 4;
  blake2bp_permute3(&(state->l.l));
  xor4ParallelUint1024(state, p);
  p += 4;
  blake2bp_permute3(&(state->l.l));
  xor4ParallelUint1024(state, p);
  p += 4;
  blake2bp_permute3(&(state->l.l));
  xor4ParallelUint1024(state, p);
  p += 4;
  blake2bp_permute3(&(state->l.l));
  xor4ParallelUint1024(state, p);
  p += 4;
  blake2bp_permute3(&(state->l.l));
  xor4ParallelUint1024(state, p);
  blake2bp_permute3(&(state->l.l));
  // Now absorb EBGDAFCH.
  p -= 3 << 2;  // Point to E
  xor4ParallelUint1024(state, p);
  blake2bp_permute3(&(state->l.l));
  p -= 3 << 2;  // Point to B
  xor4ParallelUint1024(state, p);
  blake2bp_permute3(&(state->l.l));
  p += 5 << 2;  // Point to G
  xor4ParallelUint1024(state, p);
  blake2bp_permute3(&(state->l.l));
  p -= 3 << 2;  // Point to D
  xor4ParallelUint1024(state, p);
  blake2bp_permute3(&(state->l.l));
  p -= 3 << 2;  // Point to A
  xor4ParallelUint1024(state, p);
  blake2bp_permute3(&(state->l.l));
  p += 5 << 2;  // Point to F
  xor4ParallelUint1024(state, p);
  blake2bp_permute3(&(state->l.l));
  p -= 3 << 2;  // Point to C
  xor4ParallelUint1024(state, p);
  blake2bp_permute3(&(state->l.l));
  p += 5 << 2;  // Point to H
  xor4ParallelUint1024(state, p);
  blake2bp_permute3(&(state->l.l));
}

// Hash the data.
void fhUpdate(fhContext *context, const void *data, size_t datalen) {
  fhContextInt *context_int = alignContext(context);
  const uint8_t *p = data;
  uint8_t *buf = (uint8_t*)(void*)context_int->buffer;
  if (datalen + context_int->num_bytes < FH_SUPERBLOCK_SIZE) {
    // Just buffer the data.
    memcpy(buf + context_int->num_bytes, data, datalen);
    context_int->num_bytes += datalen;
    return;
  }
  // Copy as much of the data as we can into the buf_aligned.
  uint32_t new_bytes = FH_SUPERBLOCK_SIZE - context_int->num_bytes;
  memcpy(buf + context_int->num_bytes, p, new_bytes);
  datalen -= new_bytes;
  p += new_bytes;
  context_int->num_bytes += new_bytes;
  // Process as many superblocks as we can.
  while (context_int->num_bytes == FH_SUPERBLOCK_SIZE) {
    permuteSuperblock(context_int);
    if (datalen > FH_SUPERBLOCK_SIZE) {
      new_bytes = FH_SUPERBLOCK_SIZE;
    } else {
      new_bytes = datalen;
    }
    memcpy(buf, p, new_bytes);
    datalen -= new_bytes;
    p += new_bytes;
    context_int->num_bytes = new_bytes;
  }
}

// Finalize the digest.  XOR length onto the state.
//
// If the superblock is 1/2 full or more, state, zero-pad, and use superblock
// hash.
//
// Otherwise, absorb blocks in counter-mode, 127 bytes at a time, into the 4
// lanes in parallel.  When there are 1 to 3 blocks left, absorb non-parallel
// in counter mode.
//
// If only one lane has data (the 1-block case), return the lower 256 bits of
// that lane.  Otherwise, combine the lower 256 bits of all lanes into 128
// bytes, permute 12 rounds, and return the lower 256 bits.
void fhFinal(fhContext *context, void *digest) {
  fhContextInt *context_int = alignContext(context);
  uint1024_t *state = context_int->state;
  uint1024_t *v = context_int->buffer;
  *(uint32_t*)(void*)state ^= context_int->num_bytes;
  while (context_int->num_bytes > (FH_SUPERBLOCK_SIZE*5/8)) {
    uint32_t num_bytes = context_int->num_bytes;
    if (num_bytes > FH_SUPERBLOCK_SIZE) {
      num_bytes = FH_SUPERBLOCK_SIZE;
    }
    permuteSuperblock(context_int);
    v += FH_NUM_LANES * FH_GRAPH_SIZE;
    context_int->num_bytes -= num_bytes;
  }
  if (context_int->num_bytes > 0) {
    // Add counters to remaining blocks.  These are not needed when using twisted hash.
    uint8_t *start = (uint8_t*)(void*)v;
    uint8_t *p = start;
    uint8_t *end = start + context_int->num_bytes;
    uint8_t count = 0;
    while (p < end) {
      *end++ = *p;
      *p = count++;
      p += FH_BLOCK_SIZE;
    }
    context_int->num_bytes = end - start;
    // Process blocks in parallel while we have 4 or more blocks.
    while (context_int->num_bytes > 2*FH_BLOCK_SIZE) {
      uint32_t num_bytes = context_int->num_bytes;
      if (num_bytes > FH_PARALLEL_BLOCK_SIZE) {
        num_bytes = FH_PARALLEL_BLOCK_SIZE;
      }
      if (!context_int->compressed) {
        // We need to initialize the rest of state to IV.
        memcpy(state + 1, IV + FH_BLOCK_SIZE, FH_PARALLEL_BLOCK_SIZE - FH_BLOCK_SIZE);
        context_int->compressed = true;
        if (num_bytes < FH_PARALLEL_BLOCK_SIZE) {
          // Need to zero the rest.
          memset(((uint8_t*)(void*)v) + num_bytes, 0, FH_PARALLEL_BLOCK_SIZE - num_bytes);
        }
      }
      xor4ParallelUint1024(state, v);
      blake2bp_permute12(&(state->l.l));
      v += FH_NUM_LANES;
      context_int->num_bytes -= num_bytes;
    }
  }
  // At this point, state[0] contains the low 256-bits of each lane.  Process
  // the rest with 1 lane of BLAKE2b using state[0].
  if (context_int->num_bytes == 0 && context_int->compressed) {
    // Force absorb of one block to mix lanes.
    context_int->num_bytes = FH_BLOCK_SIZE;
  }
  while (context_int->num_bytes > 0) {
    uint32_t num_bytes = context_int->num_bytes;
    if (num_bytes >= FH_BLOCK_SIZE) {
      num_bytes = FH_BLOCK_SIZE;
    } else if(!context_int->compressed) {
      memset(((uint8_t*)(void*)v) + num_bytes, 0, FH_BLOCK_SIZE - num_bytes);
    }
    *state = uint1024_XOR(*state, *v++);
    blake2b_permute12(&(state->l.l));
    context_int->num_bytes -= num_bytes;
  }
  memcpy(digest, state, FH_DIGEST_SIZE);
}

void fhFastHash(void *data, size_t datalen, void *digest) {
  fhContext context;
  fhInit(&context);
  fhUpdate(&context, data, datalen);
  fhFinal(&context, digest);
}
