#ifndef __FPR_MACHDEP_H__
#define __FPR_MACHDEP_H__

#if defined(__NetBSD__) || defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__linux__)
#define FPR_IS_UNIX
#elif defined(_WIN32)
#define FPR_IS_WIN32
#elif defined(_PSP)
#define FPR_IS_PSP
#elif defined(_EE)
#define FPR_IS_PS2
#endif

#if defined(FPR_IS_WIN32)
#define fpr_newline "\r\n"
#else
#define fpr_newline "\n"
#endif

#undef FPR_HAS_IPV6
#undef FPR_HAS_POLL
#undef FPR_HAS_FORK
#undef FPR_HAS_UNIX_SOCKET
#undef FPR_USE_SOCKLEN_T

#if !defined(_WIN32)
#define FPR_USE_SOCKLEN_T
#endif

#if defined(_WIN32)
#define FPR_HAS_IPV6
#endif

#if defined(__NetBSD__) || defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__linux__)
#define FPR_HAS_IPV6
#define FPR_HAS_POLL
#define FPR_HAS_FORK
#define FPR_HAS_UNIX_SOCKET
#endif

typedef unsigned char fpr_bool;
#define fpr_false 0
#define fpr_true 1

typedef unsigned char  fpr_uint8_t;
typedef unsigned short fpr_uint16_t;
typedef unsigned int   fpr_uint32_t;
typedef unsigned long  fpr_uint64_t;

typedef signed char fpr_int8_t;
typedef short	    fpr_int16_t;
typedef int	    fpr_int32_t;
typedef long	    fpr_int64_t;

#endif
