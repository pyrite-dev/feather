#define _FHTTPD
#include <fhttpd.h>

static int hook(fr_context_t* context, fr_request_t* req, fr_response_t* res){
}

static void register_hooks(fr_context_t* context){
	context->register_hook(hook, FR_MODULE_HOOK_LAST);
}

static fr_module_t module = {
	FR_MODULE_VERSION_00,
	NULL,
	register_hooks
};
fr_module_t* file_module = &module;
