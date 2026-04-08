#include <fhttpd.h>

#if defined(FPR_IS_NETWARE)
#include <nwthread.h>

static void unload(void) {
	running = fpr_false;
}

void netware_init(void) {
	AtUnload(unload);
}
#endif
