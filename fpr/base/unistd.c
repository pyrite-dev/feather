#include <fpr.h>
#include <fpr_int.h>

int fpr_gethostname(char* name, int namelen) {
#if defined(FPR_IS_PS2)
	if(namelen < 4) return -1;

	strcpy(name, "PS2");
	return 0;
#else
	return gethostname(name, namelen);
#endif
}

void fpr_msleep(int ms) {
#if defined(FPR_IS_WIN32)
	Sleep(ms);
#else
	struct timespec ts;

	ts.tv_sec  = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000;

	nanosleep(&ts, NULL);
#endif
}
