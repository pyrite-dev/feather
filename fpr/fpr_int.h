#ifndef __FPR_INT_H__
#define __FPR_INT_H__

#include "../config.h"

/* generic section */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#if !defined(_WIN32)
#include <unistd.h>
#include <dlfcn.h>
#endif

/* socket section */
#if defined(_WIN32)
#include <winsock2.h>
#include <ws2ipdef.h>
#else
#if defined(HAS_POLL)
#include <poll.h>
#else
#include <sys/select.h>
#endif

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#endif

/* windows.h wants to be the last one included */
#if defined(_WIN32)
#include <windows.h>
#endif

#endif
