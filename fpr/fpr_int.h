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
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>

#if !defined(FPR_DL_IS_DUMMY)
#include <dlfcn.h>
#endif
#endif

/* thread section */
#if defined(MULTITHREAD)
#if defined(FPR_IS_WIN32)
#if !defined(FPR_USE_CREATETHREAD)
#include <process.h>
#endif
#else
#include <pthread.h>
#endif
#endif

/* socket section */
#if defined(FPR_IS_WIN32)
#include <winsock2.h>
#include <ws2ipdef.h>
#else
#if defined(FPR_HAS_POLL)
#include <poll.h>
#else
#include <sys/select.h>
#endif

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
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
