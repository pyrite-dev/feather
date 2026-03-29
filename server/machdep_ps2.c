#include <fhttpd.h>

#if defined(FPR_IS_PS2)
#include <debug.h>

void ps2_init(void) {
	init_scr();
}
#endif
