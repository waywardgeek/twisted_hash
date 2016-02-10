CC=gcc
CFLAGS=-Wall -std=gnu99 -O3 -march=native -DSUPERCOP
FILES=amd64cpuinfo.c bench.c blake2b.c
HEADERS=blake2.h blake2b-load-sse2.h blake2b-load-sse41.h blake2b-round.h blake2-impl.h blake2-config.h

twblake2b: $(FILES) $(HEADERS)
	$(CC) $(CFLAGS) $(FILES) -o twblake2b
