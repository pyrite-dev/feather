#include <fhttpd.h>

#include <stb_ds.h>

void http_init(client_t* c) {
	memset(&c->request, 0, sizeof(c->request));
	memset(&c->response, 0, sizeof(c->response));

	sh_new_strdup(c->request.headers);
	shdefault(c->request.headers, NULL);

	sh_new_strdup(c->request.params);
	shdefault(c->request.params, NULL);

	c->request.body	     = NULL;
	c->request.body_seek = 0;
	c->request.body_size = -1;

	c->request.server_name = NULL;

	sh_new_strdup(c->response.headers);
	shdefault(c->response.headers, NULL);

	c->response.status_code = 0;

	c->response.body	= NULL;
	c->response.body_stream = NULL;
	c->response.body_opaque = NULL;
	c->response.body_seek	= 0;
	c->response.body_size	= -1;

	c->response.cleanup = NULL;
}

void http_end(client_t* c) {
	int i;
	for(i = 0; i < shlen(c->request.headers); i++) {
		if(c->request.headers[i].value != NULL) free(c->request.headers[i].value);
	}
	shfree(c->request.headers);

	for(i = 0; i < shlen(c->request.params); i++) {
		if(c->request.params[i].value != NULL) free(c->request.params[i].value);
	}
	shfree(c->request.params);

	if(c->request.server_name != NULL) {
		free(c->request.server_name);
		c->request.server_name = NULL;
	}

	if(c->request.body != NULL) {
		free(c->request.body);
		c->request.body = NULL;
	}

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

ppr_bool http_got(client_t* c, void* buffer, int size, int* last) {
	int   i;
	char* buf = buffer;

	*last = 0;

	for(i = 0; i < size; i++) {
		if(c->state == CS_CONNECTED) {
			if(buf[i] == ' ') {
				c->state = CS_GOT_METHOD;
			} else if(buf[i] == '\n') {
				return ppr_false;
			} else {
				if(strlen(c->request.method) == MAX_METHOD_LENGTH) {
					return ppr_false;
				} else {
					c->request.method[strlen(c->request.method)] = buf[i];
				}
			}
		} else if(c->state == CS_GOT_METHOD) {
			ppr_bool http09 = ppr_false;
			
			if(buf[i] == ' ') {
				c->state = CS_GOT_QUERY;
			} else if(buf[i] == '?') {
				c->state = CS_GOT_PATH;
			} else if(buf[i] == '\n') {
				c->state = CS_GOT_PATH;
				
				http09 = ppr_true;
			} else if(buf[i] != '\r') {
				if(strlen(c->request.path_raw) == MAX_PATH_LENGTH) {
					return ppr_false;
				} else {
					c->request.path_raw[strlen(c->request.path_raw)] = buf[i] == '\\' ? '/' : buf[i];
				}
			}

			if(c->state != CS_GOT_METHOD) {
				const char* reserved[] = {"CON", "PRN", "AUX", "NUL", "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9", "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9", NULL};
				int	    j;

				for(j = 0; c->request.path_raw[j] != 0; j++) {
					if(c->request.path_raw[j] == '\\') c->request.path_raw[j] = '/';
				}

				if(!ppr_url_decode(c->request.path, c->request.path_raw, MAX_PATH_LENGTH)) return ppr_false;
				strcpy(c->request.path_virtual, c->request.path);
				strcpy(c->request.path_virtual2, c->request.path);

				for(j = 0; c->request.path[j] != 0; j++) {
					if(c->request.path[j] == '\\') c->request.path[j] = '/';
				}

				/* poor but effective way to prevent path traversal
				 * and windows' reserved name :)
				 */

				if(c->request.path[0] != '/') return ppr_false;
				if(strlen(c->request.path) >= 4 && strstr(c->request.path, "/../") != NULL) return ppr_false;
				if(strlen(c->request.path) >= 3 && strcmp(c->request.path + strlen(c->request.path) - 3, "/..") == 0) return ppr_false;

				for(j = 0; reserved[j] != NULL; j++) {
					char*	 p   = ppr_strvacat("/", reserved[j], NULL);
					char*	 ps  = ppr_strvacat("/", reserved[j], "/", NULL);
					ppr_bool pm  = (strlen(c->request.path) >= strlen(p) && strcmp(c->request.path + strlen(c->request.path) - strlen(p), p) == 0);
					ppr_bool psm = (strlen(c->request.path) >= strlen(ps) && strstr(c->request.path, ps) != NULL);

					if(pm || psm) {
						free(p);
						free(ps);
						break;
					}

					free(p);
					free(ps);
				}

				if(reserved[j] != NULL) return ppr_false;
				
				if(http09){
					strcpy(c->request.version, "HTTP/0.9");
					
					http_req(c);
					
					return ppr_true;
				}
			}
		} else if(c->state == CS_GOT_PATH) {
			if(buf[i] == ' ') {
				c->state = CS_GOT_QUERY;
			} else {
				if(strlen(c->request.query) == MAX_QUERY_LENGTH) {
					return ppr_false;
				} else {
					c->request.query[strlen(c->request.query)] = buf[i];
				}
			}
		} else if(c->state == CS_GOT_QUERY) {
			if(buf[i] == '\n') {
				if(strcmp(c->request.version, "HTTP/1.0") == 0 || strcmp(c->request.version, "HTTP/1.1") == 0) {
					c->state = CS_GOT_VERSION;
				} else {
					return ppr_false;
				}
			} else if(buf[i] != '\r') {
				if(strlen(c->request.version) == MAX_VERSION_LENGTH) {
					return ppr_false;
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
						return ppr_false;
					}

					if(v != NULL && strlen(v) == 0) v = NULL;
					if(v == NULL) v = "";

					k = ppr_strdup(k);
					v = ppr_strdup(v);

					for(j = 0; k[j] != 0; j++) k[j] = tolower((int)k[j]);

					shput(c->request.headers, k, v);
					free(k);

					memset(c->header, 0, len);
				} else {
					const char* k = "content-length";
					char*	    v = http_req_get_header(&c->request, k);

					if(v == NULL) {
						*last = i + 1;

						http_req(c);

						return ppr_true;
					} else {
						/* content-type exists */
						c->state = CS_GOT_HEADER;

						c->request.body	     = malloc(atoi(v));
						c->request.body_seek = 0;
						c->request.body_size = atoi(v);
					}
				}
			} else if(buf[i] != '\r') {
				if(strlen(c->header) == LINE_SIZE) {
					return ppr_false;
				} else {
					c->header[strlen(c->header)] = buf[i];
				}
			}
		} else if(c->state == CS_GOT_HEADER) {
			if(c->request.body_size > 0) {
				((unsigned char*)c->request.body)[c->request.body_seek] = buf[i];
				c->request.body_seek++;
			}

			if(c->request.body_seek == c->request.body_size) {
				*last = i + 1;

				http_req(c);

				return ppr_true;
			}
		}
	}

	return ppr_true;
}

static ppr_bool proc_hooks(fr_hook_t* hooks, client_t* c, fr_context_t* context, int* loop) {
	int i;

	*loop = 0;

	for(i = 0; i < arrlen(hooks); i++) {
		int st = hooks[i](context, &c->request, &c->response);

		if(st == FR_MODULE_ERROR) return ppr_false;
		if(st == FR_MODULE_DECLINE) continue;
		if(st == FR_MODULE_OK) return ppr_true;
		if(st == FR_MODULE_LOOP) {
			*loop = 1;
			return ppr_true;
		}
	}

	return ppr_false;
}

void http_req(client_t* c) {
	int		loop = 1;
	const char*	host = http_req_get_header(&c->request, "host");
	char		hostname[1024];
	const char*	s;
	int		ok = 1;
	fr_context_t	context;
	int		first = 1;
	struct ppr_stat st;
	char		path1[MAX_PATH_LENGTH + 1];
	int		i;

	ppr_gethostname(hostname, 1024);

	c->request.handler[0] = 0;

	http_res_set_header(&c->response, "Server", server);

	c->request.server_name = ppr_strdup(host == NULL ? hostname : host);
	c->request.port	       = c->port;

	if(ppr_inet_ntop((struct ppr_sockaddr*)&c->address, c->request.realip) == NULL) {
		c->request.realip[0] = 0;
	}

	context_init(&context);
	context.config_vhost = config_vhost_match(c->request.server_name, c->port);

	s = NULL;
	if(context.config_vhost != NULL && s == NULL) s = util_stringkv_lookup(context.config_vhost->kv, "DocumentRoot");
	if(context.config_root != NULL && s == NULL) s = util_stringkv_lookup(context.config_root->kv, "DocumentRoot");

#define PATH_THING(to, from) \
	if(s == NULL || (strlen(s) + 1 + strlen(c->request.from)) >= MAX_PATH_LENGTH) { \
		ok = 0; \
	} else if(c->request.from[0] != '/') { \
		strcpy(c->request.to, c->request.from); \
	} else { \
		char* p = path_transform(s); \
\
		strcpy(c->request.to, p); \
		strcat(c->request.to, c->request.from + (p[strlen(p) - 1] == '/' ? 1 : 0)); \
\
		free(p); \
	}

#define PROCESS_PATH \
	PATH_THING(path_translated, path_virtual); \
	PATH_THING(path_translated2, path_virtual2); \
	if(strlen(c->request.path_info) > 0) { \
		PATH_THING(path_translated_info, path_info); \
	} \
\
	context_match(&context, &c->request);

#define STRIP_PATH_INFO(to) \
	if(strlen(c->request.to) > strlen(c->request.path_info) && strcmp(&c->request.to[strlen(c->request.to) - strlen(c->request.path_info)], c->request.path_info) == 0) { \
		c->request.to[strlen(c->request.to) - strlen(c->request.path_info)] = 0; \
	}

#define REWRITE \
	do { \
		PROCESS_PATH; \
\
		http_req_assume_handler(&c->request, &context); \
\
		proc_hooks(module_rewrite_hooks, c, &context, &loop); \
\
		strcpy(c->request.path_virtual2, c->request.path_virtual); \
\
		STRIP_PATH_INFO(path_virtual2); \
	} while(loop);

	for(i = 0; i < 2; i++) {
		REWRITE;

		if(i == 0) {
			if(strlen(c->request.path_info) == 0) {
				ppr_bool cond;
				char*	 n = NULL;

				strcpy(path1, c->request.path_virtual);
				while((cond = ((ppr_stat(c->request.path_translated, &st) != 0 || PPR_S_ISDIR(st.st_mode)) && strlen(c->request.path_virtual) > 1))) {
					n = strrchr(c->request.path_virtual, '/');

					if(n == NULL || n == c->request.path_virtual) {
						break;
					}

					n[0] = 0;

					REWRITE;
				}

				if(!cond && n != NULL) {
					strcpy(c->request.path_info, &path1[n - c->request.path_virtual]);
				}

				strcpy(c->request.path_virtual, path1);
			}
		}
	}

	do {
		if(first) {
			first = 0;
		} else {
			PROCESS_PATH;
		}

		if(ok && proc_hooks(module_first_hooks, c, &context, &loop)) {
		} else if(ok && proc_hooks(module_middle_hooks, c, &context, &loop)) {
		} else if(ok && proc_hooks(module_last_hooks, c, &context, &loop)) {
			if(loop) context.loop2++;
		} else {
			c->response.status_code = 500;
			strcpy(c->response.status_text, "Internal Server Error");

			http_res_set_header(&c->response, "Content-Type", "text/html");

			c->response.body = ppr_strdup(
			    "<html>\n"					  /**/
			    "	<head>\n"				  /**/
			    "		<title>Oops</title>\n"		  /**/
			    "	</head>\n"				  /**/
			    "	<body>\n"				  /**/
			    "		Something went horribly wrong!\n" /**/
			    "	</body>\n"				  /**/
			    "<html>\n"					  /**/
			);
			c->response.body_size = strlen(c->response.body);
		}

		context.loop++;
	} while(loop);

#undef PROCESS_PATH
#undef PATH_THING

	context_save(&context);

	log_srv("%s - \"%s %s %s\" %d", c->request.realip, c->request.method, c->request.path_raw, c->request.version, c->response.status_code);

	c->state = CS_GOT_BODY;
}

void http_req_set_header(fr_request_t* req, const char* key, const char* value) {
	char* v = ppr_strdup(value);
	int   ind;
	if((ind = shgeti(req->headers, key)) != -1) free(req->headers[ind].value);
	shdel(req->headers, key);
	shput(req->headers, key, v);
}

char* http_req_get_header(fr_request_t* req, const char* key) {
	return shget(req->headers, key);
}

void http_res_set_header(fr_response_t* res, const char* key, const char* value) {
	char* v = ppr_strdup(value);
	int   ind;
	if((ind = shgeti(res->headers, key)) != -1) free(res->headers[ind].value);
	shdel(res->headers, key);
	shput(res->headers, key, v);
}

char* http_res_get_header(fr_response_t* res, const char* key) {
	return shget(res->headers, key);
}

ppr_bool http_send(client_t* c) {
	ppr_bool is_chunked = strcmp(c->request.version, "HTTP/1.1") == 0 && c->response.body_size == -1;

	if(c->state == CS_GOT_BODY) {
		char*	    txt;
		const char* h;
		int	    i;

		if(strcmp(c->request.version, "HTTP/0.9") != 0){
			txt = malloc(8 + 1 + 3 + 1 + strlen(c->response.status_text) + 2 + 1);
			sprintf(txt, "%s %d %s\r\n", c->request.version, c->response.status_code, c->response.status_text);
			server_write(c, txt, strlen(txt));
			free(txt);

			for(i = 0; i < shlen(c->response.headers); i++) {
				txt = malloc(strlen(c->response.headers[i].key) + 2 + strlen(c->response.headers[i].value) + 2 + 1);
				sprintf(txt, "%s: %s\r\n", c->response.headers[i].key, c->response.headers[i].value);
				if(server_write(c, txt, strlen(txt)) < strlen(txt)) {
					free(txt);
					return ppr_false;
				}
				free(txt);
			}

			if((h = http_req_get_header(&c->request, "connection")) != NULL) {
				char* txt = malloc(strlen("Connection: ") + strlen(h) + 2 + 1);
				sprintf(txt, "Connection: %s\r\n", h);
				if(server_write(c, txt, strlen(txt)) < strlen(txt)) {
					free(txt);
					return ppr_false;
				}
				free(txt);
			}

			if(is_chunked) {
				char* txt = malloc(strlen("Transfer-Encoding: chunked") + 2 + 1);
				strcpy(txt, "Transfer-Encoding: chunked\r\n");
				if(server_write(c, txt, strlen(txt)) < strlen(txt)) {
					free(txt);
					return ppr_false;
				}
				free(txt);
			}

			if(c->response.body_size != -1 || (c->response.body_stream == NULL && c->response.body == NULL)) {
				txt = malloc(128);
				sprintf(txt, "Content-Length: %d\r\n", c->response.body_size < 0 ? 0 : c->response.body_size);
				if(server_write(c, txt, strlen(txt)) < strlen(txt)) {
					free(txt);
					return ppr_false;
				}
				free(txt);
			}

			if(server_write(c, "\r\n", 2) < 2) return ppr_false;
		}

		c->state = CS_SENT_HEADER;

		if(c->response.body_stream == NULL && c->response.body == NULL) {
			c->state = CS_CONNECTED;
		}
	} else if(c->state == CS_SENT_HEADER) {
		unsigned char chunk[BUFFER_SIZE];
		char*	      r = c->response.body;
		int	      l, n;
		char	      num[512];

		if(strcmp(c->request.method, "HEAD") == 0) {
			c->state = CS_CONNECTED;
			return ppr_true;
		}

		if(c->response.body_size == -1) {
			l = BUFFER_SIZE;
		} else {
			r += c->response.body_seek;
			l = c->response.body_size - c->response.body_seek;
			if(l > BUFFER_SIZE) l = BUFFER_SIZE;
		}

		n = 0;
		if(l > 0) {
			if(c->response.body != NULL) {
				memcpy(chunk, r, l);

				n = l;
			} else if(c->response.body_stream != NULL) {
				if((n = c->response.body_stream(&c->response, chunk, l)) <= 0) {
					c->state = CS_CONNECTED;
				}
			}
		} else {
			c->state = CS_CONNECTED;
		}

		if(n < 0) n = 0;

		sprintf(num, "%x\r\n", n);

		if(is_chunked && server_write(c, num, strlen(num)) < strlen(num)) return ppr_false;
		if(n > 0 && server_write(c, chunk, n) < n) return ppr_false;
		if(is_chunked && server_write(c, "\r\n", 2) < 2) return ppr_false;

		if(n > 0) c->response.body_seek += n;
		if(c->response.body_size != -1 && c->response.body_seek == c->response.body_size) c->state = CS_CONNECTED;
	}

	return ppr_true;
}

static void http_assume_handler(char* handler, const char* path, fr_context_t* context) {
	char* n;

	handler[0] = 0;

	if((n = strrchr(path, '/')) != NULL) {
		char* dot;

		n = ppr_strdup(n + 1);

		if((dot = strrchr(n, '.')) != NULL) {
			char* p = ppr_strvacat("Handler_", dot, NULL);
			char* chandler;

			if((chandler = context_config_lookup(context, p)) != NULL && strlen(handler) <= MAX_HANDLER_LENGTH) {
				strcpy(handler, chandler);
			}

			free(p);
		}

		free(n);
	}
}

void http_req_assume_handler(fr_request_t* req, fr_context_t* context) {
	char* n;

	if((n = context->config_lookup(context, "Handler")) == NULL) {
		http_assume_handler(req->handler, req->path_translated, context);
		http_assume_handler(req->handler2, req->path_translated2, context);
	} else {
		strcpy(req->handler, n);
		strcpy(req->handler2, n);
	}
}
