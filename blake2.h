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
#ifndef __BLAKE2_H__
#define __BLAKE2_H__

#include <stddef.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

  enum blake2b_constant
  {
    BLAKE2B_BLOCKBYTES = 128,
    BLAKE2B_OUTBYTES   = 64,
    BLAKE2B_KEYBYTES   = 64,
    BLAKE2B_SALTBYTES  = 16,
    BLAKE2B_PERSONALBYTES = 16
  };

  typedef struct __blake2b_state
  {
    uint64_t h[8];
    uint64_t t[2];
    uint64_t f[2];
    uint8_t  buf[16 * BLAKE2B_BLOCKBYTES];
    size_t   buflen;
    uint8_t  last_node;
  } blake2b_state;

  typedef struct __blake2b_param
  {
    uint8_t  digest_length; // 1
    uint8_t  key_length;    // 2
    uint8_t  fanout;        // 3
    uint8_t  depth;         // 4
    uint32_t leaf_length;   // 8
    uint64_t node_offset;   // 16
    uint8_t  node_depth;    // 17
    uint8_t  inner_length;  // 18
    uint8_t  reserved[14];  // 32
    uint8_t  salt[BLAKE2B_SALTBYTES]; // 48
    uint8_t  personal[BLAKE2B_PERSONALBYTES];  // 64
  } blake2b_param;

  // Streaming API
  int blake2b_init( blake2b_state *S, const uint8_t outlen );
  int blake2b_init_key( blake2b_state *S, const uint8_t outlen, const void *key, const uint8_t keylen );
  int blake2b_init_param( blake2b_state *S, const blake2b_param *P );
  int blake2b_update( blake2b_state *S, const uint8_t *in, uint64_t inlen );
  int blake2b_final( blake2b_state *S, uint8_t *out, uint8_t outlen );

  // Simple API
  int blake2b( uint8_t *out, const void *in, const void *key, const uint8_t outlen, const uint64_t inlen, uint8_t keylen );

  static inline int blake2( uint8_t *out, const void *in, const void *key, const uint8_t outlen, const uint64_t inlen, uint8_t keylen )
  {
    return blake2b( out, in, key, outlen, inlen, keylen );
  }

#if defined(__cplusplus)
}
#endif

#endif

