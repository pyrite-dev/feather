#include <fhttpd.h>

void context_init(fr_context_t* context) {
	memset(context, 0, sizeof(*context));

	context->config_root	= config_root;
	context->config_current = config_current;

	context->argv0 = argv0;

	context->server = server;

	context->mime_types = mime_types;

	context->log		= log_srv;
	context->path_transform = path_transform;
	context->register_hook	= module_register_hook;

	context->request_set_header  = http_req_set_header;
	context->request_get_header  = http_req_get_header;
	context->response_set_header = http_res_set_header;
	context->response_get_header = http_res_get_header;

	context->stringkv_lookup = util_stringkv_lookup;
	context->stringkv_set	 = util_stringkv_set;

	context->stringarraykv_lookup = util_stringarraykv_lookup;
	context->stringarraykv_push   = util_stringarraykv_push;
	context->stringarraykv_length = util_stringarraykv_length;
}

void context_save(fr_context_t* context) {
}
