#define _FHTTPD
#include <fhttpd.h>

#include <string.h>
#include <stdlib.h>

typedef struct error {
	int   key;
	char* value;
} error_t;

static error_t errors[] =
#include <fhttpd_status.h>
    ;

static int hook(fr_context_t* context, fr_request_t* req, fr_response_t* res) {
	char  lookup[512];
	char* s = NULL;

	if(res->status_code == 0) {
		res->status_code = 404;
		strcpy(res->status_text, "Not Found");
	}

	sprintf(lookup, "ErrorDocument%d", res->status_code);

	s = context->config_lookup(context, lookup);

	if(s == NULL || strcmp(req->path_virtual, s) == 0) {
		int i;

		for(i = 0; i < sizeof(errors) / sizeof(errors[0]); i++) {
			if(errors[i].key == res->status_code) {
				/* clang-format off */
				res->body = fpr_strvacat(
					"<html>\n"									/**/
					"	<head>\n",								/**/
					"		<title>", errors[i].value, "</title>\n" 			/**/
					"	</head>\n"								/**/
					"	<body>\n"								/**/
					"		<h1>", errors[i].value, "</h1>\n"				/**/
					"		<hr>\n"								/**/
					"		<i>", context->response_get_header(res, "Server"), "</i>\n",	/**/
					"	</body>\n"								/**/
					"</html>\n",									/**/
					NULL										/**/
				);
				/* clang-format on */
				res->body_size = strlen(res->body);

				context->response_set_header(res, "Content-Type", "text/html");

				return FR_MODULE_OK;
			}
		}
	} else {
		strcpy(req->path_virtual, s);
		strcpy(req->path_virtual2, s);

		return FR_MODULE_LOOP;
	}

	return FR_MODULE_DECLINE;
}

static int directive(fr_context_t* context, int argc, char** argv) {
	if(strcmp(argv[0], "ErrorDocument") == 0) {
		if(argc == 3) {
			char name[512];

			sprintf(name, "ErrorDocument%d", atoi(argv[1]));

			context->stringkv_set(&context->config_current->kv, name, argv[2]);
		} else {
			fprintf(stderr, "%s: %s: ErrorDocument takes 2 arguments\n", context->argv0, context->config_path);

			return FR_MODULE_ERROR;
		}

		return FR_MODULE_OK;
	}
	return FR_MODULE_DECLINE;
}

static void register_stuff(fr_context_t* context) {
	context->register_hook(hook, FR_MODULE_HOOK_LAST);
}

FR_MODULE_DATA fr_module_t error_module = {
    FR_MODULE_VERSION_00,
    directive,	    /* directive */
    register_stuff, /* register_stuff */
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL};
