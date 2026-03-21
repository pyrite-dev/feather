#define _FHTTPD
#include <fhttpd.h>

static int hook(fr_context_t* context, fr_request_t* req, fr_response_t* res) {
	char* p = fpr_strvacat("Action_", req->handler, NULL);
	char* s = context->config_lookup(context, p);

	if(s != NULL && strcmp(req->path_virtual2, s) != 0) { /* this is to prevent infinite loop */
		free(p);
		strcpy(req->path_virtual2, s);
		return FR_MODULE_LOOP;
	}

	free(p);

	return FR_MODULE_DECLINE;
}

static int directive(fr_context_t* context, int argc, char** argv) {
	if(strcmp(argv[0], "Action") == 0) {
		if(argc == 3) {
			char* p = fpr_strvacat("Action_", argv[1], NULL);

			context->stringkv_set(&context->config_current->kv, p, argv[2]);

			free(p);
		} else {
			context->log("%s: %s: Action takes 2 arguments", context->argv0, context->config_path);

			return FR_MODULE_ERROR;
		}

		return FR_MODULE_OK;
	}

	return FR_MODULE_DECLINE;
}

static void register_stuff(fr_context_t* context) {
	context->register_hook(hook, FR_MODULE_HOOK_FIRST);
}

static fr_module_t module = {
    FR_MODULE_VERSION_00,
    directive,
    register_stuff};
FR_MODULE_DATA fr_module_t* actions_module = &module;
