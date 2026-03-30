#include <fhttpd.h>

#include <stdio.h>
#include <stdlib.h>

static int hook(fr_context_t* context, fr_request_t* req, fr_response_t* res) {
	struct fpr_stat st;

	if(req->path[0] != '/') return FR_MODULE_DECLINE;

	if(fpr_stat(req->path_translated3, &st) != 0 || FPR_S_ISDIR(st.st_mode)) return FR_MODULE_DECLINE;

	if(strstr(req->handler3, "fcgi|") == req->handler3) {
		fpr_url_t url;

		fpr_url_init(&url);
		if(fpr_url_parse(&url, req->handler3 + 5)) {
		}
		fpr_url_deinit(&url); /* just to be sure */
	}

	return FR_MODULE_DECLINE;
}

static void register_stuff(fr_context_t* context) {
	context->register_hook(hook, FR_MODULE_HOOK_MIDDLE);
}

static fr_module_t module = {
    FR_MODULE_VERSION_00,
    NULL,
    register_stuff};
FR_MODULE_DATA fr_module_t* fcgi_module = &module;
