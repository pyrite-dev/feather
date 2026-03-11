#define _FHTTPD
#include <fhttpd.h>

#define TRY_LOOKUP(x,y) ((x) == NULL ? NULL : context->stringkv_lookup((x)->kv, (y)))
#define TRY_LOOKUPARR(x,y) ((x) == NULL ? NULL : context->stringarraykv_lookup((x)->arraykv, (y)))

static int file_body_stream(fr_response_t* res, unsigned char* buffer, int size){
	return fread(buffer, 1, size, res->body_opaque);
}

static void file_cleanup(fr_response_t* res){
	fclose(res->body_opaque);
}

static void file_send(fr_context_t* context, fr_request_t* req, fr_response_t* res, const char* path){
	struct fpr_stat st;
	char* s;
	char* ext;
	char* mime = NULL;

	if(fpr_stat(path, &st) != 0) return;

	s = fpr_strdup(strrchr(path, '/')); /* this should be never NULL */
	ext = strrchr(s, '.');

	if(ext != NULL){
		mime = context->stringkv_lookup(context->mime_types, ext + 1);
	}

	if(mime != NULL){
		context->stringkv_set(&res->headers, "Content-Type", mime);
	}

	if(res->status_code == 0){
		res->status_code = 200;
		strcpy(res->status_text, "OK");
	}

	res->body_stream = file_body_stream;
	res->body_opaque = fpr_fopen(path, "rb");
	res->body_size = st.st_size;

	res->cleanup = file_cleanup;

	free(s);
}

static int hook(fr_context_t* context, fr_request_t* req, fr_response_t* res){
	char* s = NULL;
	char* p;
	char* p2;
	char** arr = NULL;
	int len;
	int i;
	struct fpr_stat st;
	
	if(s == NULL) s = TRY_LOOKUP(context->config_vhost, "DocumentRoot");
	if(s == NULL) s = TRY_LOOKUP(context->config_root, "DocumentRoot");

	if(arr == NULL){
		if((arr = TRY_LOOKUPARR(context->config_vhost, "DirectoryIndex")) != NULL) len = context->stringarraykv_length(context->config_vhost->arraykv, "DirectoryIndex");
	}

	if(arr == NULL){
		if((arr = TRY_LOOKUPARR(context->config_root, "DirectoryIndex")) != NULL) len = context->stringarraykv_length(context->config_root->arraykv, "DirectoryIndex");
	}

	if(s == NULL || arr == NULL) return FR_MODULE_DECLINE;

	p = context->path_transform(s);
	p2 = fpr_strvacat(p, req->path, NULL);

	if(fpr_stat(p2, &st) == 0 && !FPR_S_ISDIR(st.st_mode)){
		file_send(context, req, res, p2);

		free(p2);
		free(p);
		return FR_MODULE_OK;
	}

	for(i = 0; i < len; i++){
		char* p3 = fpr_strvacat(p2, p2[strlen(p2) - 1] == '/' ? "" : "/", arr[i], NULL);

		if(fpr_stat(p3, &st) == 0 && !FPR_S_ISDIR(st.st_mode)){
			file_send(context, req, res, p3);

			free(p3);
			free(p2);
			free(p);
			return FR_MODULE_OK;
		}

		free(p3);
	}

	free(p2);
	free(p);

	res->status_code = 404;
	strcpy(res->status_text, "Not Found");

	return FR_MODULE_OK;
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
