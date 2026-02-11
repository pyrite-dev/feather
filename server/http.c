#include <fhttpd.h>

#include <stb_ds.h>

fpr_bool http_got(client_t* c, void* buffer, int size) {
	int   i;
	char* buf = buffer;

	for(i = 0; i < size; i++) {
		if(c->state == CS_CONNECTED) {
			if(buf[i] == ' ') {
				c->state = CS_GOT_METHOD;
			} else {
				if(strlen(c->request.method) == MAX_METHOD_LENGTH) {
					return fpr_false;
				} else {
					c->request.method[strlen(c->request.method)] = buf[i];
				}
			}
		} else if(c->state == CS_GOT_METHOD) {
			if(buf[i] == ' ') {
				c->state = CS_GOT_PATH;
			} else {
				if(strlen(c->request.path) == MAX_PATH_LENGTH) {
					return fpr_false;
				} else {
					c->request.path[strlen(c->request.path)] = buf[i];
				}
			}
		} else if(c->state == CS_GOT_PATH) {
			if(buf[i] == '\n') {
				if(strcmp(c->request.version, "HTTP/1.1") == 0 || strcmp(c->request.version, "HTTP/1.1") == 0) {
					int j;

					c->state = CS_GOT_VERSION;

					for(j = 0; j < shlen(c->request.headers); j++) {
						if(c->request.headers[j].value != NULL) free(c->request.headers[j].value);
					}
					shfree(c->request.headers);
					sh_new_strdup(c->request.headers);
					shdefault(c->request.headers, NULL);
				} else {
					return fpr_false;
				}
			} else if(buf[i] != '\r') {
				if(strlen(c->request.version) == MAX_VERSION_LENGTH) {
					return fpr_false;
				} else {
					c->request.version[strlen(c->request.version)] = buf[i];
				}
			}
		} else if(c->state == CS_GOT_VERSION) {
			if(buf[i] == '\n') {
				if(strlen(c->header) > 0) {
					int   len = strlen(c->header);
					char* k	  = c->header;
					char* v	  = NULL;
					char* t	  = strchr(k, ':');
					int   j;

					if(t != NULL) {
						t[0] = 0;

						for(v = t + 1; v[0] != 0 && v[0] == ' '; v++);
					} else {
						for(j = 0; j < shlen(c->request.headers); j++) {
							if(c->request.headers[j].value != NULL) free(c->request.headers[j].value);
						}
						shfree(c->request.headers);
						return fpr_false;
					}

					if(v != NULL && strlen(v) == 0) v = NULL;

					k = fpr_strdup(k);
					if(v != NULL) v = fpr_strdup(v);

					for(j = 0; k[j] != 0; j++) k[j] = tolower((int)k[j]);

					shput(c->request.headers, k, v);

					memset(c->header, 0, len);
				} else {
					const char* k = "content-length";
					char*	    v = shget(c->request.headers, k);

					if(v == NULL) {
						http_req(c);

						c->state = CS_CONNECTED;
					} else {
						/* content-type exists */
						c->state = CS_GOT_HEADER;
					}
				}
			} else if(buf[i] != '\r') {
				if(strlen(c->header) == LINE_SIZE) {
					return fpr_false;
				} else {
					c->header[strlen(c->header)] = buf[i];
				}
			}
		} else if(c->state == CS_GOT_HEADER) {
		}
	}

	return fpr_true;
}

void http_req(client_t* c) {
}
