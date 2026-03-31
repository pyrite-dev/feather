#define _FHTTPD
#include <fhttpd.h>

#include <string.h>
#include <stdlib.h>

static int directive(fr_context_t* context, int argc, char** argv) {
	if(strcmp(argv[0], "LoadModule") == 0) {
		if(argc == 3) {
			char*	     p	 = context->path_transform(argv[2]);
			void*	     h	 = fpr_dlopen(p); /* XXX: this leaks! but we don't know good ways to solve... yet */
			fr_module_t* mod = fpr_dlsym(h, argv[1]);
			char*	     s;

			if(mod == NULL) {
				s   = fpr_strvacat("_", argv[1], NULL); /* watcom likes to export this way */
				mod = fpr_dlsym(h, s);
				free(s);
			}

			if(mod == NULL) {
				context->log("%s: %s: Failed to load %s", context->argv0, context->config_path, p);
				free(p);
				fpr_dlclose(h);

				return FR_MODULE_ERROR;
			}

			module_load(*mod);

			free(p);
		} else {
			context->log("%s: %s: LoadModule takes 2 arguments", context->argv0, context->config_path);

			return FR_MODULE_ERROR;
		}

		return FR_MODULE_OK;
	}

	return FR_MODULE_DECLINE;
}

FR_MODULE_DATA fr_module_t dso_module = {
    FR_MODULE_VERSION_00,
    directive,
    NULL};
