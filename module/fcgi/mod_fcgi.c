#include <fhttpd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* https://www.mit.edu/~yandros/doc/specs/fcgi-spec.html */

#define FCGI_HEADER_LEN 8

#define FCGI_VERSION_1 1

#define FCGI_BEGIN_REQUEST 1
#define FCGI_ABORT_REQUEST 2
#define FCGI_END_REQUEST 3
#define FCGI_PARAMS 4
#define FCGI_STDIN 5
#define FCGI_STDOUT 6
#define FCGI_STDERR 7
#define FCGI_DATA 8
#define FCGI_GET_VALUES 9
#define FCGI_GET_VALUES_RESULT 10
#define FCGI_UNKNOWN_TYPE 11
#define FCGI_MAXTYPE (FCGI_UNKNOWN_TYPE)

#define FCGI_NULL_REQUEST_ID 0

#define FCGI_KEEP_CONN 1

#define FCGI_RESPONDER 1
#define FCGI_AUTHORIZER 2
#define FCGI_FILTER 3

#define FCGI_REQUEST_COMPLETE 0
#define FCGI_CANT_MPX_CONN 1
#define FCGI_OVERLOADED 2
#define FCGI_UNKNOWN_ROLE 3

typedef struct fcgi fcgi_t;

struct fcgi {
	int fd;

	unsigned char* buf;
	int	       seek;
	int	       size;
};

static int send_packet(int fd, int type, void* data, int length) {
	int	       seek  = 0;
	unsigned char* input = data;

	do {
		unsigned char* pkt;
		int	       pktsz = length > BUFFER_SIZE ? BUFFER_SIZE : length;
		int	       pad   = 8 - (pktsz % 8);
		int	       n;

		if(pad == 8) pad = 0;

		pkt = malloc(8 + pktsz + pad);

		pkt[0] = FCGI_VERSION_1;
		pkt[1] = type;
		pkt[2] = 0;
		pkt[3] = 0;
		pkt[4] = (pktsz >> 8) & 0xff;
		pkt[5] = (pktsz >> 0) & 0xff;
		pkt[6] = pad;
		pkt[7] = 0;
		if(pktsz > 0) memcpy(pkt + 8, input + seek, pktsz);
		memset(pkt + 8 + pktsz, 0, pad);

		n = fpr_send(fd, pkt, 8 + pktsz + pad, 0);

		free(pkt);

		if(n < (8 + pktsz + pad)) return -1;

		length -= pktsz;
		seek += pktsz;
	} while(length > 0);

	return 0;
}

static void* recv_packet(int fd, int* type, int* size) {
	unsigned char  header[8];
	unsigned char* buf;
	void*	       pad;
	int	       seek = 0;
	int	       sz;
	int	       n = 0;
	int	       l = 0;

	while(l < 8) {
		if((n = fpr_recv(fd, header + l, 8 - l, 0)) <= 0) return NULL;

		l += n;
	}

	if(header[0] != FCGI_VERSION_1) return NULL;
	*type = header[1];

	*size = header[4];
	*size = (*size) << 8;
	*size = (*size) | header[5];

	if(header[6] > 0) pad = malloc(header[6]);

	if((*size) == 0) {
		buf    = malloc(1);
		buf[0] = 0;
		return buf;
	}

	buf = malloc(*size);

	sz = *size;
	while(sz > 0) {
		int pktsz = sz > BUFFER_SIZE ? BUFFER_SIZE : sz;

		if((n = fpr_recv(fd, buf + seek, pktsz, 0)) <= 0) {
			free(buf);
			return NULL;
		}

		sz -= n;
		seek += n;
	}

	if(header[6] > 0) {
		l = 0;
		while(l < header[6]) {
			if((n = fpr_recv(fd, pad, header[6] - l, 0)) <= 0) {
				free(buf);
				return NULL;
			}

			l += n;
		}
	}

	return buf;
}

static void big2(unsigned char* out, unsigned int n) {
	out[0] = (n >> 8) & 0xff;
	out[1] = (n >> 0) & 0xff;
}

static void big4(unsigned char* out, unsigned int n) {
	out[0] = (n >> 24) & 0xff;
	out[1] = (n >> 16) & 0xff;
	out[2] = (n >> 8) & 0xff;
	out[3] = (n >> 0) & 0xff;
}

static int send_param(int fd, const char* key, const char* value) {
	int st;
	if(strlen(key) == 0) {
		st = send_packet(fd, FCGI_PARAMS, "", 0);
	} else if(strlen(key) <= 127 && strlen(value) <= 127) {
		int	       len = 1 + 1 + strlen(key) + strlen(value);
		unsigned char* pkt = malloc(len);

		pkt[0] = strlen(key);
		pkt[1] = strlen(value);
		memcpy(pkt + 2, key, strlen(key));
		memcpy(pkt + 2 + strlen(key), value, strlen(value));

		st = send_packet(fd, FCGI_PARAMS, pkt, len);
		free(pkt);
	} else if(strlen(key) <= 127 && strlen(value) > 127) {
		int	       len = 1 + 4 + strlen(key) + strlen(value);
		unsigned char* pkt = malloc(len);

		pkt[0] = strlen(key);
		big4(pkt + 1, strlen(value) | (1 << 31));
		memcpy(pkt + 5, key, strlen(key));
		memcpy(pkt + 5 + strlen(key), value, strlen(value));

		st = send_packet(fd, FCGI_PARAMS, pkt, len);
		free(pkt);
	} else if(strlen(key) > 127 && strlen(value) <= 127) {
		int	       len = 4 + 1 + strlen(key) + strlen(value);
		unsigned char* pkt = malloc(len);

		big4(pkt, strlen(key) | (1 << 31));
		pkt[4] = strlen(value);
		memcpy(pkt + 5, key, strlen(key));
		memcpy(pkt + 5 + strlen(key), value, strlen(value));

		st = send_packet(fd, FCGI_PARAMS, pkt, len);
		free(pkt);
	} else {
		int	       len = 4 + 4 + strlen(key) + strlen(value);
		unsigned char* pkt = malloc(len);

		big4(pkt, strlen(key) | (1 << 31));
		big4(pkt + 4, strlen(value) | (1 << 31));
		memcpy(pkt + 8, key, strlen(key));
		memcpy(pkt + 8 + strlen(key), value, strlen(value));

		st = send_packet(fd, FCGI_PARAMS, pkt, len);
		free(pkt);
	}

	return st;
}

static int body_stream(fr_response_t* res, unsigned char* buffer, int size) {
	fcgi_t*	       f = res->body_opaque;
	int	       n;
	int	       ans = 0;
	int	       type, psize;
	unsigned char* pkt;

repeat:;
	if(f->buf != NULL) {
		n = f->size - f->seek;
		if(n > (size - ans)) n = size - ans;

		memcpy(buffer + ans, f->buf + f->seek, n);

		f->seek += n;
		if(f->seek == f->size) {
			free(f->buf);
			f->buf = NULL;
		}

		ans += n;
	}

	if((size - ans) > 0) {
		while(1) {
			if((pkt = recv_packet(f->fd, &type, &psize)) == NULL) {
				return 0;
			}

			if(type == FCGI_END_REQUEST) {
				free(pkt);
				break;
			} else if(type == FCGI_STDOUT) {
				f->buf	= pkt;
				f->seek = 0;
				f->size = psize;

				goto repeat;
			}
		}
	}

	return ans;
}

static void cleanup(fr_response_t* res) {
	fcgi_t* f = res->body_opaque;

	if(f->buf != NULL) free(f->buf);
	fpr_socket_close(f->fd);

	free(f);
}

static int connect_fcgi(fr_context_t* context, fr_request_t* req, fr_response_t* res, const char* input) {
	fpr_url_t url;
	int	       fd = -1;

	fpr_url_init(&url);
	if(fpr_url_parse(&url, input)) {
		unsigned char  begin[8];
		char	       buf[128];
		char*	       h;
		char**	       headers;
		int	       i;
		int	       type;
		int	       size;
		unsigned char* p;
		fcgi_t*	       f;
		unsigned char* savebuf = NULL;
		int	       seek    = 0;
		int	       nl      = 0;

		if(strcmp(url.scheme, "unix") == 0) {
			struct fpr_sockaddr_un addr;

			fd = fpr_socket(FPR_PF_UNIX, FPR_SOCK_STREAM, 0);

			if(fd < 0) goto error;

			addr.sun_family = FPR_AF_UNIX;
			strcpy(addr.sun_path, url.path);

			if(fpr_connect(fd, (struct fpr_sockaddr*)&addr, sizeof(addr)) < 0) goto error;
		} else if(strcmp(url.scheme, "tcp") == 0 && url.host != NULL && url.port != 0) {
			int		     len;
			struct fpr_sockaddr* addr = fpr_inet_addr(url.host, &len);
			if(addr == NULL) goto error;

			fd = fpr_socket(addr->sa_family == FPR_AF_INET ? FPR_PF_INET : FPR_PF_INET6, FPR_SOCK_STREAM, 0);

			if(fd < 0) goto error;

			if(addr->sa_family == FPR_AF_INET) {
				((struct fpr_sockaddr_in*)addr)->sin_port = fpr_htons(url.port);
			} else {
				((struct fpr_sockaddr_in6*)addr)->sin6_port = fpr_htons(url.port);
			}

			if(fpr_connect(fd, addr, len) < 0) goto error;
		} else {
			goto error;
		}

		memset(begin, 0, 8);
		big2(begin, FCGI_RESPONDER);
		begin[2] = 0;

		if(send_packet(fd, FCGI_BEGIN_REQUEST, begin, 8) < 0) goto error;

		/* ref: http://hoohoo.ncsa.uiuc.edu/cgi/env.html */

		if(send_param(fd, "SERVER_SOFTWARE", FR_VERSION) < 0) goto error;
		if(send_param(fd, "SERVER_NAME", req->server_name) < 0) goto error;
		if(send_param(fd, "GATEWAY_INTERFACE", "CGI/1.1") < 0) goto error;
		if(send_param(fd, "SERVER_PROTOCOL", req->version) < 0) goto error;

		sprintf(buf, "%d", req->port);
		if(send_param(fd, "SERVER_PORT", buf) < 0) goto error;

		if(send_param(fd, "REQUEST_METHOD", req->method) < 0) goto error;
		if(send_param(fd, "PATH_INFO", strlen(req->path_info) == 0 ? req->path : req->path_info) < 0) goto error;
		if(send_param(fd, "PATH_TRANSLATED", req->path_translated4) < 0) goto error;
		if(send_param(fd, "SCRIPT_NAME", req->path_virtual3) < 0) goto error;
		if(strlen(req->query) > 0 && send_param(fd, "QUERY_STRING", req->query) < 0) goto error;

		if((h = context->request_get_header(req, "content-type")) != NULL && send_param(fd, "CONTENT_TYPE", h) < 0) {
			free(h);
			goto error;
		}
		free(h);

		sprintf(buf, "%d", req->body_size);
		if(req->body_size > 0 && send_param(fd, "BODY_SIZE", buf) < 0) goto error;

		headers = context->stringkv_keys(req->headers);
		for(i = 0; headers[i] != NULL; i++) {
			int   j;
			char* s;

			if(strcmp(headers[i], "content-type") == 0) continue;

			h = malloc(strlen(headers[i]) + 1);
			for(j = 0; headers[i][j] != 0; j++) {
				if(headers[i][j] == '-') {
					h[j] = '_';
				} else {
					h[j] = toupper(headers[i][j]);
				}
			}
			h[j] = 0;

			s = fpr_strvacat("HTTP_", h, NULL);

			if(send_param(fd, s, context->request_get_header(req, headers[i])) < 0) {
				free(s);
				free(h);

				goto error;
			}

			free(s);
			free(h);
		}

		/* Apache extension, needed to make PHP work */
		sprintf(buf, "%d", res->status_code == 0 ? 200 : res->status_code);
		if(send_param(fd, "REDIRECT_STATUS", buf) < 0) goto error;

		if(send_param(fd, "REDIRECT_URL", req->path_virtual2) < 0) goto error;
		if(send_param(fd, "SCRIPT_FILENAME", req->path_translated3) < 0) goto error;

		h = fpr_strvacat(req->path_raw, strlen(req->query) > 0 ? "?" : "", req->query, NULL);
		if(send_param(fd, "REQUEST_URI", h) < 0) {
			free(h);
			goto error;
		}
		free(h);

		if((h = context->config_lookup(context, "DocumentRoot")) != NULL) {
			char* h2 = context->path_transform(h);

			if(send_param(fd, "DOCUMENT_ROOT", h2) < 0) goto error;

			free(h2);
		}

		send_packet(fd, FCGI_PARAMS, "", 0);

		if(req->body_size > 0) {
			send_packet(fd, FCGI_STDIN, req->body, req->body_size);
		}
		send_packet(fd, FCGI_STDIN, "", 0);

		savebuf = NULL;

		h    = malloc(1);
		h[0] = 0;

		while(1) {
			p = recv_packet(fd, &type, &size);

			if(p == NULL) {
				free(p);
				free(h);
				goto error;
			} else if(type == FCGI_END_REQUEST) {
				free(p);
				free(h);
				goto error;
			} else if(type == FCGI_STDOUT) {
				for(i = 0; i < size; i++) {
					if(p[i] == '\n') {
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

						nl++;

						if(nl == 2) {
							if((i + 1) != size) {
								savebuf = p;
								seek	= i + 1;
							}
							goto pass;
						}
					} else if(p[i] != '\r') {
						char* old = h;
						char  s[2];

						s[0] = p[i];
						s[1] = 0;

						h = fpr_strvacat(old, s, NULL);
						free(old);

						nl = 0;
					}
				}
			}

			free(p);
		}

	pass:;
		free(h);

		f = malloc(sizeof(*f));

		f->fd	= fd;
		f->buf	= savebuf;
		f->seek = seek;
		f->size = size;

		res->body_opaque = f;
		res->body_stream = body_stream;
		res->cleanup	 = cleanup;

		context->request_set_header(req, "connection", "close");

		fpr_url_deinit(&url);

		if(res->status_code == 0) {
			res->status_code = 200;
			strcpy(res->status_text, "OK");
		}

		return FR_MODULE_OK;
	}
	fpr_url_deinit(&url); /* just to be sure */

	return FR_MODULE_DECLINE;

error:;
	if(fd >= 0) fpr_socket_close(fd);

	fpr_url_deinit(&url);

	res->status_code = 500;
	strcpy(res->status_text, "Internal Server Error");

	return FR_MODULE_DECLINE;
}

static int hook(fr_context_t* context, fr_request_t* req, fr_response_t* res) {
	struct fpr_stat st;

	if(req->path[0] != '/') return FR_MODULE_DECLINE;

	if(fpr_stat(req->path_translated3, &st) != 0 || FPR_S_ISDIR(st.st_mode)) return FR_MODULE_DECLINE;

	if(strstr(req->handler3, "fcgi|") == req->handler3) {
		return connect_fcgi(context, req, res, req->handler3 + 5);
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
FR_MODULE_DATA fr_module_t* fcgi_module = &module;
