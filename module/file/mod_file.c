#define _FHTTPD
#include <fhttpd.h>

#define TRY_LOOKUP(x,y) ((x) == NULL ? NULL : context->stringkv_lookup((x)->kv, (y)))

static int hook(fr_context_t* context, fr_request_t* req, fr_response_t* res){
	const char* s = NULL;
	char* p;
	char* p2;
	FILE* f;

	if(s == NULL) s = TRY_LOOKUP(context->config_root, "DocumentRoot");
	if(s == NULL) s = TRY_LOOKUP(context->config_vhost, "DocumentRoot");

	if(s == NULL) return FR_MODULE_DECLINE;

	p = context->path_transform(s);
	p2 = fpr_strvacat(p, req->path, NULL);

	free(p2);
	free(p);

	return FR_MODULE_DECLINE;
}

static int directive(fr_context_t* context, int argc, char** argv){
	if(strcmp(argv[0], "DirectoryIndex") == 0) {
		if(argc >= 2) {
			int i;

			for(i = 1; i < argc; i++){
				context->stringarraykv_push(context->config_current->arraykv, "DirectoryIndex", argv[i]);
			}
		} else {
			fprintf(stderr, "%s: %s: DirectoryIndex takes 1 argument or more\n", context->argv0, context->config_path);

			return FR_MODULE_ERROR;
		}

		return FR_MODULE_OK;
	}

	return FR_MODULE_DECLINE;
}

static void register_hooks(fr_context_t* context){
	context->register_hook(hook, FR_MODULE_HOOK_LAST);
}

static fr_module_t module = {
	FR_MODULE_VERSION_00,
	directive,
	register_hooks
};
fr_module_t* file_module = &module;
