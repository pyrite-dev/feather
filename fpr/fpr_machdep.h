#ifndef __FPR_MACHDEP_H__
#define __FPR_MACHDEP_H__

#if defined(__NetBSD__) || defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__linux__) || defined(__sun__)
#define FPR_IS_UNIX
#elif defined(_WIN32)
#define FPR_IS_WIN32
#elif defined(_PSP)
#define FPR_IS_PSP
#elif defined(_EE)
#define FPR_IS_PS2
#elif defined(__NETWARE__)
#define FPR_IS_NETWARE
#endif

#if defined(FPR_IS_WIN32) || defined(FPR_IS_NETWARE)
#define fpr_newline "\r\n"
#else
#define fpr_newline "\n"
#endif

#undef FPR_HAS_IPV6
#undef FPR_HAS_POLL
#undef FPR_HAS_FORK
#undef FPR_HAS_UNIX_SOCKET
#undef FPR_USE_SOCKLEN_T

#if defined(FPR_IS_UNIX) || defined(FPR_IS_PSP) || defined(FPR_IS_PS2)
#define FPR_USE_SOCKLEN_T
#endif

#if defined(_WIN32)
#define FPR_HAS_IPV6
#endif

#if defined(__NetBSD__) || defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__linux__) || defined(__sun__)
#define FPR_HAS_IPV6
#define FPR_HAS_POLL
#define FPR_HAS_FORK
#define FPR_HAS_UNIX_SOCKET
#endif

typedef unsigned char fpr_bool;
#define fpr_false 0
#define fpr_true 1

#if defined(_MSC_VER) || defined(__WATCOMC__)
typedef unsigned __int8	 fpr_uint8_t;
typedef unsigned __int16 fpr_uint16_t;
typedef unsigned __int32 fpr_uint32_t;

typedef __int8	fpr_int8_t;
typedef __int16 fpr_int16_t;
typedef __int32 fpr_int32_t;

typedef __int64 fpr_size_t;
#elif !defined(__STRICT_ANSI__) && (defined(__GNUC__) || defined(__clang__))
typedef unsigned char  fpr_uint8_t;
typedef unsigned short fpr_uint16_t;
typedef unsigned int   fpr_uint32_t;

typedef signed char fpr_int8_t;
typedef short	    fpr_int16_t;
typedef int	    fpr_int32_t;

typedef long long fpr_size_t;
#else
typedef unsigned char  fpr_uint8_t;
typedef unsigned short fpr_uint16_t;
typedef unsigned int   fpr_uint32_t;

typedef signed char fpr_int8_t;
typedef short	    fpr_int16_t;
typedef int	    fpr_int32_t;

typedef long fpr_size_t;
#endif

typedef fpr_size_t fpr_time_t;

#endif
