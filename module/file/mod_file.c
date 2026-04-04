#define _FHTTPD
#include <fhttpd.h>

#include <string.h>
#include <stdlib.h>

#define TRY_LOOKUP(x, y) ((x) == NULL ? NULL : context->stringkv_lookup((x)->kv, (y)))

static int file_body_stream(fr_response_t* res, unsigned char* buffer, int size) {
	return fpr_fread(buffer, 1, size, res->body_opaque);
}

static void file_cleanup(fr_response_t* res) {
	fpr_fclose(res->body_opaque);
}

static void file_send(fr_context_t* context, fr_request_t* req, fr_response_t* res, const char* path) {
	struct fpr_stat st;
	char*		s;
	char*		ext;
	char*		mime  = NULL;
	const char*	day[] = {
		"Sun",
		"Mon",
		"Tue",
		"Wed",
		"Thu",
		"Fri",
		"Sat"};
	const char* mon[] = {
	    "Jan",
	    "Feb",
	    "Mar",
	    "Apr",
	    "May",
	    "Jun",
	    "Jul",
	    "Aug",
	    "Sep",
	    "Oct",
	    "Nov",
	    "Dec"};
	char	      date[128];
	struct fpr_tm tm;

	(void)req;

	if(fpr_stat(path, &st) != 0) return;

	s   = fpr_strdup(strrchr(path, '/')); /* this should be never NULL */
	ext = strrchr(s, '.');

	if(ext != NULL) {
		mime = context->stringkv_lookup(context->mime_types, ext + 1);
	}

	if(mime != NULL) {
		context->response_set_header(res, "Content-Type", mime);
	}

	if(res->status_code == 0) {
		res->status_code = 200;
		strcpy(res->status_text, "OK");
	}

	fpr_gmtime(&tm, st.st_modtime);

	res->body_stream = file_body_stream;
	res->body_opaque = fpr_fopen(path, "rb");
	res->body_size	 = st.st_size;

	sprintf(date, "%s, %02d %s %d %02d:%02d:%02d GMT", day[tm.tm_wday], tm.tm_mday, mon[tm.tm_mon], 1900 + tm.tm_year, tm.tm_hour, tm.tm_min, tm.tm_sec);
	context->response_set_header(res, "Last-Modified", date);

	res->cleanup = file_cleanup;

	free(s);
}

static int hook(fr_context_t* context, fr_request_t* req, fr_response_t* res) {
	struct fpr_stat st;

	if(req->path[0] != '/') return FR_MODULE_DECLINE;

	/* error from other module */
	if(context->loop2 == 0 && res->status_code != 200 && res->status_code != 0) return FR_MODULE_DECLINE;

	if(fpr_stat(req->path_translated, &st) == 0 && !FPR_S_ISDIR(st.st_mode)) {
		file_send(context, req, res, req->path_translated);
		return FR_MODULE_OK;
	}

	return FR_MODULE_DECLINE;
}

static void register_stuff(fr_context_t* context) {
	context->register_hook(hook, FR_MODULE_HOOK_LAST);
}

FR_MODULE_DATA fr_module_t file_module = {
    FR_MODULE_VERSION_00,
    NULL,	    /* directive */
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
