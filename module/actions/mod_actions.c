#define _FHTTPD
#include <fhttpd.h>

static int hook_rewrite(fr_context_t* context, fr_request_t* req, fr_response_t* res) {
	char* p = ppr_strvacat("Action_", req->handler2, NULL);
	char* s = context->config_lookup(context, p);

	(void)res;

	if(s != NULL) {
		strcpy(req->handler, "cgi|");
		strcat(req->handler, s);

		strcpy(req->handler2, req->handler);
	}

	return FR_MODULE_DECLINE;
}

static int directive(fr_context_t* context, int argc, char** argv) {
	if(strcmp(argv[0], "Action") == 0) {
		if(argc == 3) {
			char* p = ppr_strvacat("Action_", argv[1], NULL);

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
	context->register_hook(hook_rewrite, FR_MODULE_HOOK_REWRITE);
}

FR_MODULE_DATA fr_module_t actions_module = {
    FR_MODULE_VERSION_00,
    directive,	    /* directive */
    register_stuff, /* register_stuff */
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL};
