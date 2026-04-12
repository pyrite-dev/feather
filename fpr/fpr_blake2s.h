/*
   BLAKE2 reference source code package - reference C implementations

   Copyright 2012, Samuel Neves <sneves@dei.uc.pt>.  You may use this under the
   terms of the CC0, the OpenSSL Licence, or the Apache Public License 2.0, at
   your option.  The terms of these licenses can be found at:

   - CC0 1.0 Universal : http://creativecommons.org/publicdomain/zero/1.0
   - OpenSSL license   : https://www.openssl.org/source/license.html
   - Apache 2.0        : http://www.apache.org/licenses/LICENSE-2.0

   More information about the BLAKE2 hash function can be found at
   https://blake2.net.
*/
#ifndef FPR_BLAKE2S_H
#define FPR_BLAKE2S_H

#include <fpr_machdep.h>

enum fpr_blake2s_constant {
	BLAKE2S_BLOCKBYTES    = 64,
	BLAKE2S_OUTBYTES      = 32,
	BLAKE2S_KEYBYTES      = 32,
	BLAKE2S_SALTBYTES     = 8,
	BLAKE2S_PERSONALBYTES = 8
};

typedef struct fpr_blake2s_state__ {
	fpr_uint32_t h[8];
	fpr_uint32_t t[2];
	fpr_uint32_t f[2];
	fpr_uint8_t  buf[BLAKE2S_BLOCKBYTES];
	fpr_size_t   buflen;
	fpr_size_t   outlen;
	fpr_uint8_t  last_node;
} fpr_blake2s_state;

#ifdef FPR_HAS_PACK
#pragma pack(1)
#endif
struct fpr_blake2s_param__ {
	fpr_uint8_t  digest_length; /* 1 */
	fpr_uint8_t  key_length;    /* 2 */
	fpr_uint8_t  fanout;	    /* 3 */
	fpr_uint8_t  depth;	    /* 4 */
	fpr_uint32_t leaf_length;   /* 8 */
	fpr_uint32_t node_offset;   /* 12 */
	fpr_uint16_t xof_length;    /* 14 */
	fpr_uint8_t  node_depth;    /* 15 */
	fpr_uint8_t  inner_length;  /* 16 */
	/* fpr_uint8_t  reserved[0]; */
	fpr_uint8_t salt[BLAKE2S_SALTBYTES];	     /* 24 */
	fpr_uint8_t personal[BLAKE2S_PERSONALBYTES]; /* 32 */
};
#ifdef FPR_HAS_PACK
#pragma pack()
#endif

typedef struct fpr_blake2s_param__ fpr_blake2s_param;

#if 0
  /* Padded structs result in a compile-time error */
  enum {
    BLAKE2_DUMMY_1 = 1/(int)(sizeof(fpr_blake2s_param) == BLAKE2S_OUTBYTES)
  };
#endif

/* Streaming API */
int fpr_blake2s_init(fpr_blake2s_state* S, fpr_size_t outlen);
int fpr_blake2s_init_key(fpr_blake2s_state* S, fpr_size_t outlen, const void* key, fpr_size_t keylen);
int fpr_blake2s_init_param(fpr_blake2s_state* S, const fpr_blake2s_param* P);
int fpr_blake2s_update(fpr_blake2s_state* S, const void* in, fpr_size_t inlen);
int fpr_blake2s_final(fpr_blake2s_state* S, void* out, fpr_size_t outlen);

/* Simple API */
int fpr_blake2s(void* out, fpr_size_t outlen, const void* in, fpr_size_t inlen, const void* key, fpr_size_t keylen);

#endif
