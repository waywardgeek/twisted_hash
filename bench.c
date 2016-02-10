/*
 * BLAKE2 reference source code package - reference C implementations
 *
 * Copyright 2012, Samuel Neves <sneves@dei.uc.pt>.
 * You may use this under the terms of the CC0, the OpenSSL Licence, or the
 * Apache Public License 2.0, at your option.  The terms of these licenses can
 * be found at:
 *
 * - OpenSSL license   : https://www.openssl.org/source/license.html
 * - Apache 2.0        : http://www.apache.org/licenses/LICENSE-2.0
 * - CC0 1.0 Universal : http://www.apache.org/licenses/LICENSE-2.0
 *
 * More information about the BLAKE2 hash function can be found at
 * https://blake2.net.
 * 
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int crypto_hash( unsigned char *out, const unsigned char *in, unsigned long long inlen );

static int bench_cmp( const void *x, const void *y )
{
  const int64_t *ix = ( const int64_t * )x;
  const int64_t *iy = ( const int64_t * )y;
  return *ix - *iy;
}

unsigned long long cpucycles( void );

void bench()
{
#define BENCH_TRIALS     32
#define BENCH_MAXLEN   1536
  static unsigned char in[4096];
  static unsigned long long median[4096 + 1];
  int i, j;
  printf( "#bytes  median  per byte\n" );

  /* 1 ... BENCH_MAXLEN */
  for( j = 0; j <= 4096; ++j )
  {
    uint64_t cycles[BENCH_TRIALS + 1];

    for( i = 0; i <= BENCH_TRIALS; ++i )
    {
      cycles[i] = cpucycles();
      crypto_hash( in, in, j );
    }

    for( i = 0; i < BENCH_TRIALS; ++i )
      cycles[i] = cycles[i + 1] - cycles[i];

    qsort( cycles, BENCH_TRIALS, sizeof( uint64_t ), bench_cmp );
    median[j] = cycles[BENCH_TRIALS / 2];
  }

  for( j = 0; j <= BENCH_MAXLEN; j += 8 )
    printf( "%5d, %7.2f\n", j, ( double )median[j] / j );

  printf( "#2048   %6llu   %7.2f\n", median[2048], ( double )median[2048] / 2048.0 );
  printf( "#4096   %6llu   %7.2f\n", median[4096], ( double )median[4096] / 4096.0 );
  printf( "#long     long   %7.2f\n", ( double )( median[4096] - median[2048] ) / 2048.0 );
}

int main()
{
  bench();
  return 0;
}