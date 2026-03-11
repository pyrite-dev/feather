#include <fhttpd.h>

#include <stb_ds.h>

void http_init(client_t* c) {
	memset(&c->request, 0, sizeof(c->request));
	memset(&c->response, 0, sizeof(c->response));

	sh_new_strdup(c->request.headers);
	shdefault(c->request.headers, NULL);

	sh_new_strdup(c->response.headers);
	shdefault(c->response.headers, NULL);

	c->response.status_code = 0;

	c->response.body	= NULL;
	c->response.body_stream = NULL;
	c->response.body_opaque = NULL;
	c->response.body_seek	= 0;
	c->response.body_size	= 0;

	c->response.cleanup = NULL;
}

void http_end(client_t* c) {
	int i;
	for(i = 0; i < shlen(c->request.headers); i++) {
		if(c->request.headers[i].value != NULL) free(c->request.headers[i].value);
	}
	shfree(c->request.headers);
	for(i = 0; i < shlen(c->response.headers); i++) {
		if(c->response.headers[i].value != NULL) free(c->response.headers[i].value);
	}
	shfree(c->response.headers);

	if(c->response.body != NULL) {
		free(c->response.body);
		c->response.body = NULL;
	}

	SAFECALL(c->response.cleanup)(&c->response);
}

fpr_bool http_got(client_t* c, void* buffer, int size, int* last) {
	int   i;
	char* buf = buffer;

	*last = 0;

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
					char*	 p   = fpr_strvacat("/", reserved[j], NULL);
					char*	 ps  = fpr_strvacat("/", reserved[j], "/", NULL);
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
						*last = i + 1;

						http_req(c);

						return fpr_true;
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

static fpr_bool proc_hooks(fr_hook_t* hooks, client_t* c) {
	int	     i;
	fr_context_t context;
	const char*  host = http_req_get_header(&c->request, "host");

	context_init(&context);

	context.config_vhost = config_vhost_match(host, c->port);

	for(i = 0; i < arrlen(hooks); i++) {
		int st = hooks[i](&context, &c->request, &c->response);

		if(st == FR_MODULE_ERROR || st == FR_MODULE_OK) {
			context_save(&context);
		}

		if(st == FR_MODULE_ERROR) return fpr_false;
		if(st == FR_MODULE_DECLINE) continue;
		if(st == FR_MODULE_OK) return fpr_true;
	}
	context_save(&context);

	return fpr_false;
}

void http_req(client_t* c) {
	http_res_set_header(&c->response, "Server", FR_SERVER);

	if(proc_hooks(module_first_hooks, c)) {
	} else if(proc_hooks(module_middle_hooks, c)) {
	} else if(proc_hooks(module_last_hooks, c)) {
	} else {
		c->response.status_code = 500;
		strcpy(c->response.status_text, "Internal Server Error");

		http_res_set_header(&c->response, "Content-Type", "text/html");

		c->response.body = fpr_strdup(
		    "<html>\n"								 /**/
		    "	<head>\n"							 /**/
		    "		<title>Oops</title>\n"					 /**/
		    "	</head>\n"							 /**/
		    "	<body>\n"							 /**/
		    "		No one handled the request... this should not happen!\n" /**/
		    "	</body>\n"							 /**/
		    "<html>\n"								 /**/
		);
		c->response.body_size = strlen(c->response.body);
	}

	c->state = CS_GOT_BODY;
}

void http_req_set_header(fr_request_t* req, const char* key, const char* value) {
	char* v = fpr_strdup(value);
	int   ind;
	if((ind = shgeti(req->headers, key)) != -1) free(req->headers[ind].value);
	shdel(req->headers, key);
	shput(req->headers, key, v);
}

char* http_req_get_header(fr_request_t* req, const char* key) {
	return shget(req->headers, key);
}

void http_res_set_header(fr_response_t* res, const char* key, const char* value) {
	char* v = fpr_strdup(value);
	int   ind;
	if((ind = shgeti(res->headers, key)) != -1) free(res->headers[ind].value);
	shdel(res->headers, key);
	shput(res->headers, key, v);
}

char* http_res_get_header(fr_response_t* res, const char* key) {
	return shget(res->headers, key);
}

void http_send(client_t* c) {
	if(c->state == CS_GOT_BODY) {
		char*	    txt;
		const char* h;
		int	    i;

		txt = malloc(8 + 1 + 3 + 1 + strlen(c->response.status_text) + 2 + 1);
		sprintf(txt, "HTTP/1.1 %d %s\r\n", c->response.status_code, c->response.status_text);
		server_write(c->fd, txt, strlen(txt));
		free(txt);

		for(i = 0; i < shlen(c->response.headers); i++) {
			txt = malloc(strlen(c->response.headers[i].key) + 2 + strlen(c->response.headers[i].key) + 2 + 1);
			sprintf(txt, "%s: %s\r\n", c->response.headers[i].key, c->response.headers[i].value);
			server_write(c->fd, txt, strlen(txt));
			free(txt);
		}

		if((h = http_req_get_header(&c->request, "connection")) != NULL) {
			char* txt = malloc(strlen("Connection: ") + strlen(h) + 2 + 1);
			sprintf(txt, "Connection: %s\r\n", h);
			server_write(c->fd, txt, strlen(txt));
			free(txt);
		}

		if(c->response.body != NULL) {
			txt = malloc(128);
			sprintf(txt, "Content-Length: %d\r\n", c->response.body_size);
			server_write(c->fd, txt, strlen(txt));
			free(txt);
		}

		server_write(c->fd, "\r\n", 2);

		c->state = CS_SENT_HEADER;
	} else if(c->state == CS_SENT_HEADER) {
		char  chunk[BUFFER_SIZE];
		char* r = c->response.body;
		int   l;

		r += c->response.body_seek;

		l = c->response.body_size - c->response.body_seek;
		if(l > BUFFER_SIZE) l = BUFFER_SIZE;

		if(c->response.body != NULL) {
			memcpy(chunk, r, l);
		} else if(c->response.body_stream != NULL) {
			c->response.body_stream(&c->response, chunk, l);
		}
		server_write(c->fd, chunk, l);

		c->response.body_seek += l;
		if(c->response.body_seek == c->response.body_size) c->state = CS_CONNECTED;
	}
}
