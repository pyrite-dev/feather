#define _FHTTPD
#include <fhttpd.h>

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static int hook_rewrite(fr_context_t* context, fr_request_t* req, fr_response_t* res) {
	char* hl;
	char* h;

	if(req->path[0] != '/') return FR_MODULE_DECLINE;

	if((hl = context->config_lookup(context, "RemoteIPHeader")) != NULL && (h = context->request_get_header(req, hl)) != NULL && strlen(h) < 256) {
		strcpy(req->realip, h);
	}

	return FR_MODULE_DECLINE;
}

static int directive(fr_context_t* context, int argc, char** argv) {
	if(strcmp(argv[0], "RemoteIPHeader") == 0) {
		if(argc == 2) {
			char* h = fpr_strdup(argv[1]);
			int   i;

			for(i = 0; h[i] != 0; i++) h[i] = tolower(h[i]);

			context->stringkv_set(&context->config_current->kv, "RemoteIPHeader", h);

			free(h);
		} else {
			fprintf(stderr, "%s: %s: RemoteIPHeader takes 1 argument\n", context->argv0, context->config_path);

			return FR_MODULE_ERROR;
		}

		return FR_MODULE_OK;
	}

	return FR_MODULE_DECLINE;
}

static void register_stuff(fr_context_t* context) {
	context->register_hook(hook_rewrite, FR_MODULE_HOOK_REWRITE);
}

static fr_module_t module = {
    FR_MODULE_VERSION_00,
    directive,
    register_stuff};
FR_MODULE_DATA fr_module_t* remoteip_module = &module;
