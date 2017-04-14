#include "fasthash.h"

#include <stdio.h>
#include <stdlib.h>

#define BUF_SIZE 16384

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

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s filename\n", argv[0]);
    return 1;
  }
  FILE *file = fopen(argv[1], "rb");
  if (file == NULL) {
    fprintf(stderr, "Unable to read file %s\n", argv[1]);
    return 1;
  }
  fhContext context;
  fhInit(&context);
  uint8_t *buf = calloc(BUF_SIZE, sizeof(uint8_t));
  size_t num_bytes =  fread(buf, sizeof(uint8_t), BUF_SIZE, file);
  while (num_bytes > 0) {
    fhUpdate(&context, buf, num_bytes);
    num_bytes =  fread(buf, sizeof(uint8_t), BUF_SIZE, file);
  }
  uint8_t digest[FH_DIGEST_SIZE];
  fhFinal(&context, digest);
  printhex("Digest", digest, FH_DIGEST_SIZE);
  free(buf);
  fclose(file);
  return 0;
}
