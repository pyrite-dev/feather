#include <fpr.h>
#include <fpr_int.h>

void fpr_init(void) {
	fpr_socket_init();

#if defined(FPR_IS_NETWARE)
	SetCurrentNameSpace(NW_NS_LONG);
#endif

	fpr_gmtime_mutex    = fpr_mutex_create();
	fpr_localtime_mutex = fpr_mutex_create();
}

void fpr_uninit(void) {
	fpr_mutex_destroy(fpr_localtime_mutex);
	fpr_mutex_destroy(fpr_gmtime_mutex);

	fpr_socket_uninit();
}
