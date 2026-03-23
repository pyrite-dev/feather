#include <fhttpd.h>

#include <stb_ds.h>
#include <hsregex.h>

#define LOOKUP(x) \
	if(context->x != NULL && (v = util_stringkv_lookup(context->x->kv, key)) != NULL) return v;
char* context_config_lookup(fr_context_t* context, const char* key) {
	char* v;
	int   i;

	for(i = 0; context->config_matches[i] != NULL; i++) {
		LOOKUP(config_matches[i]);
	}

	LOOKUP(config_vhost);

	LOOKUP(config_root);

	return NULL;
}
#undef LOOKUP

#define LOOKUP(x) \
	if(context->x != NULL && (v = util_stringarraykv_lookup(context->x->arraykv, key)) != NULL) { \
		*len = util_stringarraykv_length(context->x->arraykv, key); \
		return v; \
	}
char** context_config_lookup_array(fr_context_t* context, const char* key, int* len) {
	char** v = NULL;
	int    i;

	for(i = 0; context->config_matches[i] != NULL; i++) {
		LOOKUP(config_matches[i]);
	}

	LOOKUP(config_vhost);

	LOOKUP(config_root);

	return NULL;
}
#undef LOOKUP

void context_init(fr_context_t* context) {
	memset(context, 0, sizeof(*context));

	context->config_root	= config_root;
	context->config_current = config_current;
	context->config_matches = NULL;

	arrput(context->config_matches, NULL);

	context->argv0 = argv0;

	context->server = server;

	context->mime_types = mime_types;

	context->loop = 0;

	context->log		= log_srv;
	context->path_transform = path_transform;
	context->register_hook	= module_register_hook;

	context->request_set_header	= http_req_set_header;
	context->request_get_header	= http_req_get_header;
	context->request_assume_handler = http_req_assume_handler;

	context->response_set_header = http_res_set_header;
	context->response_get_header = http_res_get_header;

	context->stringkv_lookup = util_stringkv_lookup;
	context->stringkv_set	 = util_stringkv_set;
	context->stringkv_keys	 = util_stringkv_keys;

	context->stringarraykv_lookup = util_stringarraykv_lookup;
	context->stringarraykv_push   = util_stringarraykv_push;
	context->stringarraykv_length = util_stringarraykv_length;

	context->config_lookup		= context_config_lookup;
	context->config_lookup_array	= context_config_lookup_array;
	context->config_children_length = config_children_length;
}

void context_save(fr_context_t* context) {
	arrfree(context->config_matches);
}

static void context_match_config(fr_context_t* context, fr_request_t* req, fr_config_t* config) {
	int i;

	arrfree(context->config_matches);

	for(i = 0; i < context->config_children_length(config); i++) {
		if(strcmp(config->children[i]->name, "FilesMatch") == 0) {
			regex_t re;
			regmatch_t match;

			if(hsregcomp(&re, config->children[i]->section.match.pattern, REG_EXTENDED) != 0){
				continue;
			}

			re.re_nsub = 1;

			if(hsregexec(&re, req->path_translated, 1, &match, 0) == 0){	
				arrput(context->config_matches, config->children[i]);
			}
			
			hsregfree(&re);
		}
	}

	arrput(context->config_matches, NULL);
}

void context_match(fr_context_t* context, fr_request_t* req) {
	if(context->config_vhost != NULL) context_match_config(context, req, context->config_vhost);
	context_match_config(context, req, context->config_root);
}
