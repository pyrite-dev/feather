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
#ifndef BLAKE2S_INT_H
#define BLAKE2S_INT_H

#include <fpr_machdep.h>

#include <string.h>

#define BLAKE2S_INLINE static __inline

BLAKE2S_INLINE fpr_uint32_t load32(const void* src) {
#if defined(NATIVE_LITTLE_ENDIAN)
	fpr_uint32_t w;
	memcpy(&w, src, sizeof w);
	return w;
#else
	const fpr_uint8_t* p = (const fpr_uint8_t*)src;
	return ((fpr_uint32_t)(p[0]) << 0) |
	       ((fpr_uint32_t)(p[1]) << 8) |
	       ((fpr_uint32_t)(p[2]) << 16) |
	       ((fpr_uint32_t)(p[3]) << 24);
#endif
}

BLAKE2S_INLINE void store16(void* dst, fpr_uint16_t w) {
#if defined(NATIVE_LITTLE_ENDIAN)
	memcpy(dst, &w, sizeof w);
#else
	fpr_uint8_t* p = (fpr_uint8_t*)dst;
	*p++	       = (fpr_uint8_t)w;
	w >>= 8;
	*p++ = (fpr_uint8_t)w;
#endif
}

BLAKE2S_INLINE void store32(void* dst, fpr_uint32_t w) {
#if defined(NATIVE_LITTLE_ENDIAN)
	memcpy(dst, &w, sizeof w);
#else
	fpr_uint8_t* p = (fpr_uint8_t*)dst;
	p[0]	       = (fpr_uint8_t)(w >> 0);
	p[1]	       = (fpr_uint8_t)(w >> 8);
	p[2]	       = (fpr_uint8_t)(w >> 16);
	p[3]	       = (fpr_uint8_t)(w >> 24);
#endif
}

BLAKE2S_INLINE fpr_uint32_t rotr32(const fpr_uint32_t w, const unsigned c) {
	return (w >> c) | (w << (32 - c));
}

#include <stddef.h>

/* prevents compiler optimizing out memset() */
BLAKE2S_INLINE void secure_zero_memory(void* v, fpr_size_t n) {
	static void* (*const volatile memset_v)(void*, int, size_t) = &memset;
	memset_v(v, 0, n);
}

#endif
