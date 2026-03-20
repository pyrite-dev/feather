#define _FHTTPD
#include <fhttpd.h>

static int hook(fr_context_t* context, fr_request_t* req, fr_response_t* res) {
	printf("%s\n", req->handler);

	return FR_MODULE_DECLINE;
}

static void register_stuff(fr_context_t* context) {
	context->register_hook(hook, FR_MODULE_HOOK_MIDDLE);
}

static fr_module_t module = {
    FR_MODULE_VERSION_00,
    NULL,
    register_stuff};
FR_MODULE_DATA fr_module_t* cgi_module = &module;
