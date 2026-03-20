#define _FHTTPD
#include <fhttpd.h>

#include <string.h>
#include <stdlib.h>

#define TRY_LOOKUPARR(x, y) ((x) == NULL ? NULL : context->stringarraykv_lookup((x)->arraykv, (y)))

static int hook(fr_context_t* context, fr_request_t* req, fr_response_t* res) {
	char**		arr = NULL;
	int		len;
	struct fpr_stat st;
	int		i;

	arr = context->config_lookup_array(context, "DirectoryIndex", &len);

	for(i = 0; i < len; i++) {
		char* p = fpr_strvacat(req->path_translated2, req->path_translated[strlen(req->path_translated2) - 1] == '/' ? "" : "/", arr[i], NULL);

		if(fpr_stat(p, &st) == 0 && !FPR_S_ISDIR(st.st_mode)) {
			strcpy(req->path_virtual, req->path);

			if(req->path_virtual[strlen(req->path_virtual) - 1] != '/') strcat(req->path_virtual, "/");
			strcat(req->path_virtual, arr[i]);

			strcpy(req->path_virtual2, req->path_virtual);

			free(p);
			return FR_MODULE_LOOP;
		}

		free(p);
	}

	return FR_MODULE_DECLINE;
}

static int directive(fr_context_t* context, int argc, char** argv) {
	if(strcmp(argv[0], "DirectoryIndex") == 0) {
		if(argc >= 2) {
			int i;

			for(i = 1; i < argc; i++) {
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

static void register_stuff(fr_context_t* context) {
	context->register_hook(hook, FR_MODULE_HOOK_FIRST);
}

static fr_module_t module = {
    FR_MODULE_VERSION_00,
    directive,
    register_stuff};
FR_MODULE_DATA fr_module_t* dir_module = &module;
