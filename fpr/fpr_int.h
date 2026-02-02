#ifndef __FPR_INT_H__
#define __FPR_INT_H__

#include "../config.h"

#if defined(HAS_POLL)
#include <poll.h>
#elif !defined(_WIN32)
#include <sys/select.h>
#endif

#include <stdlib.h>
#include <string.h>

#endif
