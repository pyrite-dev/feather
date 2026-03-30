#include <fhttpd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static void send_packet(int fd, int type, int length, void* data) {
	int	       pad = 8 - (length % 8);
	unsigned char* pkt = malloc(8 + length + pad);

	pkt[0] = 1; /* version 1 */
	pkt[1] = type;
	pkt[2] = 0;
	pkt[3] = 0;
	pkt[4] = (length >> 8) & 0xff;
	pkt[5] = (length >> 0) & 0xff;
	pkt[6] = pad;
	pkt[7] = 0;
	memcpy(pkt + 8, data, length);
	memset(pkt + 8 + length, 0, pad);

	fpr_send(fd, pkt, 8 + length + pad, 0);

	free(pkt);
}

static int hook(fr_context_t* context, fr_request_t* req, fr_response_t* res) {
	struct fpr_stat st;

	if(req->path[0] != '/') return FR_MODULE_DECLINE;

	if(fpr_stat(req->path_translated3, &st) != 0 || FPR_S_ISDIR(st.st_mode)) return FR_MODULE_DECLINE;

	if(strstr(req->handler3, "fcgi|") == req->handler3) {
		fpr_url_t url;

		fpr_url_init(&url);
		if(fpr_url_parse(&url, req->handler3 + 5)) {
			if(strcmp(url.scheme, "unix") == 0) {
				int fd = fpr_socket(FPR_PF_UNIX, FPR_SOCK_STREAM, 0);

				if(fd < 0) {
					res->status_code = 500;
					strcpy(res->status_text, "Internal Server Error");

					return FR_MODULE_DECLINE;
				}
			} else {
				res->status_code = 500;
				strcpy(res->status_text, "Internal Server Error");

				return FR_MODULE_DECLINE;
			}
		}
		fpr_url_deinit(&url); /* just to be sure */
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
