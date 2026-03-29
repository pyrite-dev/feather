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
