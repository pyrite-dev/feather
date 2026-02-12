#include <fhttpd.h>

void context_init(fr_context_t* context) {
	context->root	 = config_root;
	context->current = config_current;
}

void context_save(fr_context_t* context) {
}
