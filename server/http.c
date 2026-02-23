#include <fhttpd.h>

#include <stb_ds.h>

void http_init(client_t* c) {
	sh_new_strdup(c->request.headers);
	shdefault(c->request.headers, NULL);
}

void http_end(client_t* c) {
	int i;
	for(i = 0; i < shlen(c->request.headers); i++) {
		if(c->request.headers[i].value != NULL) free(c->request.headers[i].value);
	}
	shfree(c->request.headers);
}

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
				c->state = CS_GOT_QUERY;
			} else if(buf[i] == '?') {
				c->state = CS_GOT_PATH;
			} else {
				if(strlen(c->request.path) == MAX_PATH_LENGTH) {
					return fpr_false;
				} else {
					c->request.path[strlen(c->request.path)] = buf[i] == '\\' ? '/' : buf[i];
				}
			}

			if(c->state != CS_GOT_METHOD) {
				/* poor but effective way to prevent path traversal
				 * and windows' reserved name :)
				 */
				const char* reserved[] = {"CON", "PRN", "AUX", "NUL", "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9", "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9", NULL};
				int	    j;

				if(c->request.path[0] != '/') return fpr_false;
				if(strlen(c->request.path) >= 4 && strstr(c->request.path, "/../") != NULL) return fpr_false;
				if(strlen(c->request.path) >= 3 && strcmp(c->request.path + strlen(c->request.path) - 3, "/..") == 0) return fpr_false;

				for(j = 0; reserved[j] != NULL; j++) {
					char*	 p   = fpr_strvacat_alloc("/", reserved[j], NULL);
					char*	 ps  = fpr_strvacat_alloc("/", reserved[j], "/", NULL);
					fpr_bool pm  = (strlen(c->request.path) >= strlen(p) && strcmp(c->request.path + strlen(c->request.path) - strlen(p), p) == 0);
					fpr_bool psm = (strlen(c->request.path) >= strlen(ps) && strstr(c->request.path, ps) != NULL);

					if(pm || psm) {
						free(p);
						free(ps);
						break;
					}

					free(p);
					free(ps);
				}

				if(reserved[j] != NULL) return fpr_false;
			}
		} else if(c->state == CS_GOT_PATH) {
			if(buf[i] == ' ') {
				c->state = CS_GOT_QUERY;
			} else {
				if(strlen(c->request.query) == MAX_QUERY_LENGTH) {
					return fpr_false;
				} else {
					c->request.query[strlen(c->request.query)] = buf[i];
				}
			}
		} else if(c->state == CS_GOT_QUERY) {
			if(buf[i] == '\n') {
				if(strcmp(c->request.version, "HTTP/1.0") == 0 || strcmp(c->request.version, "HTTP/1.1") == 0) {
					c->state = CS_GOT_VERSION;
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

static fpr_bool proc_hooks(fr_hook_t* hooks){
	return fpr_false;
}

void http_req(client_t* c) {
	if(proc_hooks(module_first_hooks)){
	}else if(proc_hooks(module_middle_hooks)){
	}else if(proc_hooks(module_last_hooks)){
	}else{

		return;
	}
}
