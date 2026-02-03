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
#endif

#endif
