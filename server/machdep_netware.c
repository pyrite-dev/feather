#include <fhttpd.h>

#if defined(FPR_IS_NETWARE)
#include <nwthread.h>
#include <nwconio.h>

static void unload(void) {
	running = fpr_false;
}

void netware_init(void) {
	AtUnload(unload);
}

void netware_start(void (*main_stuff)(void* arg)) {
	DestroyScreen(GetCurrentScreen());
	SetCurrentScreen(CreateScreen("Feather Console", 0));
	BeginThread(main_stuff, NULL, 0, NULL);
	ThreadSwitch();
}
#endif
