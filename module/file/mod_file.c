#define _FHTTPD
#include <fhttpd.h>

static fr_module_t module = {
	FR_MODULE_VERSION_00,
	NULL,
	NULL
};
fr_module_t* file_module = &module;
