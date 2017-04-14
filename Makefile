CFLAGS = -O3 -Wall -march=native -Wno-unused-functions

all: twistedsum benchmark test

twistedsum: twistedsum.c twistedhash.c blake2b/blake2bp.c blake2b/blake2b.c
	gcc $(CFLAGS) -o twistedsum twistedsum.c twistedhash.c blake2b/blake2bp.c blake2b/blake2b.c

benchmark: benchmark.c twistedhash.c blake2b/blake2bp.c blake2b/blake2b.c
	gcc $(CFLAGS) -o benchmark benchmark.c twistedhash.c blake2b/blake2bp.c blake2b/blake2b.c

test: test.c blake2b/blake2bp.c blake2b/blake2b.c
	gcc $(CFLAGS) -o test test.c blake2b/blake2bp.c blake2b/blake2b.c

clean:
	rm -f *.o twistedsum benchmark test
