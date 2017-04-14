CFLAGS = -O3 -Wall -march=native -Wno-unused-functions

all: filehash benchmark test

filehash: filehash.c fasthash.c blake2b/blake2bp.c blake2b/blake2b.c
	gcc $(CFLAGS) -o filehash filehash.c fasthash.c blake2b/blake2bp.c blake2b/blake2b.c

benchmark: benchmark.c fasthash.c blake2b/blake2bp.c blake2b/blake2b.c
	gcc $(CFLAGS) -o benchmark benchmark.c fasthash.c blake2b/blake2bp.c blake2b/blake2b.c

test: test.c blake2b/blake2bp.c blake2b/blake2b.c
	gcc $(CFLAGS) -o test test.c blake2b/blake2bp.c blake2b/blake2b.c

clean:
	rm -f *.o filehash benchmark test
