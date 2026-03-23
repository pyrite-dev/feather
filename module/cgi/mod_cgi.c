#define _FHTTPD
#include <fhttpd.h>

#include <stb_ds.h>

#include <stdlib.h>
#include <string.h>

static int body_stream(fr_response_t* res, unsigned char* buffer, int size) {
	int n = fpr_process_read(res->body_opaque, buffer, size);

	return n;
}

static void cleanup(fr_response_t* res) {
	fpr_process_destroy(res->body_opaque);
}

static int create_cgi(fr_context_t* context, fr_request_t* req, fr_response_t* res) {
	char** envs = NULL;
	int    i;
	void*  proc;
	char*  s;
	char   buf[512];
	int    n;
	char   ch[2];
	char*  h;
	int    nl = 0;

	/* ref: http://hoohoo.ncsa.uiuc.edu/cgi/env.html */

	s = fpr_strvacat("SERVER_SOFTWARE=", FR_VERSION_TEXT, NULL);
	arrput(envs, s);

	s = fpr_strvacat("SERVER_NAME=", req->server_name, NULL);
	arrput(envs, s);

	s = fpr_strdup("GATEWAY_INTERFACE=CGI/1.1");
	arrput(envs, s);

	s = fpr_strvacat("SERVER_PROTOCOL=", req->version, NULL);
	arrput(envs, s);

	sprintf(buf, "%d", req->port, NULL);
	s = fpr_strvacat("SERVER_PORT=", buf, NULL);
	arrput(envs, s);

	s = fpr_strvacat("REQUEST_METHOD=", req->method, NULL);
	arrput(envs, s);

	s = fpr_strvacat("PATH_INFO=", req->path, NULL);
	arrput(envs, s);

	s = fpr_strvacat("PATH_TRANSLATED=", req->path_translated2, NULL);
	arrput(envs, s);

	s = fpr_strvacat("SCRIPT_NAME=", req->path_virtual, NULL);
	arrput(envs, s);

	if(strlen(req->query) > 0){
		s = fpr_strvacat("QUERY_STRING=", &req->query[1], NULL);
		arrput(envs, s);
	}

	if((h = context->request_get_header(req, "content-type")) != NULL){
		s = fpr_strvacat("CONTENT_TYPE=", h, NULL);
		arrput(envs, s);
	}

	if(req->body_size > 0){
		sprintf(buf, "%d", req->body_size, NULL);
		s = fpr_strvacat("CONTENT_LENGTH=", buf, NULL);
		arrput(envs, s);
	}

	/* Apache extension, needed to make PHP work */
	sprintf(buf, "%d", res->status_code == 0 ? 200 : res->status_code);
	s = fpr_strvacat("REDIRECT_STATUS=", buf, NULL);
	arrput(envs, s);

	s = NULL;
	arrput(envs, s);

	if((proc = fpr_process_create(req->path_translated, envs)) == NULL) {
		for(i = 0; i < arrlen(envs) - 1; i++) free(envs[i]);
		arrfree(envs);

		res->status_code = 500;
		strcpy(res->status_text, "Internal Server Error");

		return FR_MODULE_DECLINE;
	}

	for(i = 0; i < arrlen(envs) - 1; i++) free(envs[i]);
	arrfree(envs);

	context->request_set_header(req, "connection", "close");

	res->body_opaque = proc;

	if(req->body_size > 0) fpr_process_write(res->body_opaque, req->body, req->body_size);

	fpr_process_close(res->body_opaque);

	res->cleanup = cleanup;

	h    = malloc(1);
	h[0] = 0;

	ch[1] = 0;
	while(1) {
		n = fpr_process_read(res->body_opaque, ch, 1);

		if(n <= 0) {
			res->status_code = 500;
			strcpy(res->status_text, "Internal Server Error");

			free(h);

			return FR_MODULE_DECLINE;
		}

		if(ch[0] == '\n') {
			if(strlen(h) > 0) {
				char* colon = strchr(h, ':');
				char* v	    = NULL;

				if(colon != NULL) colon[0] = 0;

				if(colon != NULL) {
					for(v = colon + 1; ((*v) == ' ' || (*v) == '\t') && ((*v) != 0); v++);

					if((*v) != 0) {
						if(strcmp(h, "Status") == 0) {
							char* v2 = strchr(v, ' ');
							if(v2 != NULL) {
								v2[0] = 0;

								res->status_code = atoi(v);
								if(strlen(v2) <= MAX_STATUS_TEXT_LENGTH) strcpy(res->status_text, v2);
							}
						} else {
							context->response_set_header(res, h, v);
						}
					}
				}
			}

			free(h);
			h    = malloc(1);
			h[0] = 0;
		}

		if(ch[0] == '\n') nl++;

		if(nl == 2) break;

		if(ch[0] != '\r' && ch[0] != '\n') {
			char* old;

			nl = 0;

			old = h;
			h   = fpr_strvacat(old, ch, NULL);
			free(old);
		}
	}

	free(h);

	if(res->status_code == 0) {
		res->status_code = 200;
		strcpy(res->status_text, "OK");
	}

	res->body_stream = body_stream;

	return FR_MODULE_OK;
}

static int hook(fr_context_t* context, fr_request_t* req, fr_response_t* res) {
	struct fpr_stat st;

	if(req->path[0] != '/') return FR_MODULE_DECLINE;

	if(fpr_stat(req->path_translated, &st) != 0 || FPR_S_ISDIR(st.st_mode)) return FR_MODULE_DECLINE;

	if(strcmp(req->handler, "cgi-script") == 0) {
		return create_cgi(context, req, res);
	}

	return FR_MODULE_DECLINE;
}

static void register_stuff(fr_context_t* context) {
	context->register_hook(hook, FR_MODULE_HOOK_MIDDLE);
}

static fr_module_t module = {
    FR_MODULE_VERSION_00,
    NULL,
    register_stuff};
FR_MODULE_DATA fr_module_t* cgi_module = &module;
