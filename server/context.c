#include <fhttpd.h>

void context_init(fr_context_t* context) {
	memset(context, 0, sizeof(*context));

	context->config_root	= config_root;
	context->config_current = config_current;

	context->argv0 = argv0;

	context->log		= log_srv;
	context->path_transform = path_transform;
}

void context_save(fr_context_t* context) {
}
