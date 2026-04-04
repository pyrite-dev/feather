#ifndef __FPR_INT_H__
#define __FPR_INT_H__

#include "../config.h"

/* generic section */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>

#if defined(FPR_IS_UNIX)
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dlfcn.h>
#endif

#if defined(FPR_IS_UNIX) || defined(FPR_IS_PSP) || defined(FPR_IS_PS2)
#include <unistd.h>
#include <dirent.h>
#endif

/* thread section */
#if defined(FPR_IS_WIN32)
#if !defined(FPR_USE_CREATETHREAD)
#include <process.h>
#endif
#elif defined(FPR_IS_UNIX) || defined(FPR_IS_PSP)
#include <pthread.h>
#endif

/* socket section */
#if defined(FPR_IS_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#include <ws2ipdef.h>

#ifndef IPPROTO_IPV6
#define IPPROTO_IPV6 41
#endif

#ifndef IPV6_V6ONLY
#define IPV6_V6ONLY 27
#endif
#else
#if defined(FPR_HAS_POLL)
#include <poll.h>
#else
#include <sys/select.h>
#endif

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#if defined(FPR_HAS_UNIX_SOCKET)
#include <sys/un.h>
#endif
#endif

/* windows.h wants to be the last one included */
#if defined(FPR_IS_WIN32)
#include <windows.h>
#endif

#endif
