#ifndef __FHTTPD_H__
#define __FHTTPD_H__

#include "../config.h"

#if defined(_WIN32)
#define FR_PLATFORM "Win32"
#elif defined(_PSP)
#define FR_PLATFORM "PSP"
#else
#define FR_PLATFORM "Unix"
#endif

#define FR_VERSION "0.0.0"
#define FR_VERSION_TEXT "Feather/" FR_VERSION
#define FR_SERVER FR_VERSION_TEXT " (" FR_PLATFORM ")"

#if !defined(RESOURCE)
#include <fpr.h>

#if defined(FPR_IS_WIN32)
#define FR_MODULE_DATA __declspec(dllexport)
#else
#define FR_MODULE_DATA
#endif

enum fr_module_version {
	FR_MODULE_VERSION_00 = 0
};

enum fr_module_return {
	FR_MODULE_OK	  = 0,	/* Module handled the request */
	FR_MODULE_DECLINE = -1, /* Module declined to handle the request */
	FR_MODULE_ERROR	  = -2, /* Module raised an error */
	FR_MODULE_LOOP	  = -3	/* Module says server should run hook again */
};

enum fr_module_hook_order {
	FR_MODULE_HOOK_FIRST = 0,
	FR_MODULE_HOOK_MIDDLE,
	FR_MODULE_HOOK_LAST
};

typedef struct fr_module	       fr_module_t;
typedef struct fr_stringkv	       fr_stringkv_t;
typedef struct fr_stringarraykv	       fr_stringarraykv_t;
typedef struct fr_request	       fr_request_t;
typedef struct fr_response	       fr_response_t;
typedef union fr_config_section_union  fr_config_section_union_t;
typedef struct fr_config_section_vhost fr_config_section_vhost_t;
typedef struct fr_config	       fr_config_t;
typedef struct fr_context	       fr_context_t;

typedef int (*fr_hook_t)(fr_context_t* context, fr_request_t* req, fr_response_t* res);

struct fr_stringkv {
	char* key;
	char* value;
};

struct fr_stringarraykv {
	char*  key;
	char** value;
};

struct fr_module {
	int version;

	int (*directive)(fr_context_t* context, int argc, char** argv);
	void (*register_stuff)(fr_context_t* context);
};

/* read path_translated if you want to read "alternative" path or "normal" path, which is most likely what you want
 * read path_translated2 if you want to read "normal" path (e.g. Script (not executable) Action wants to run)
 *
 * AddHandler handlers are decided from path_translated2
 */
struct fr_request {
	char	       method[MAX_METHOD_LENGTH + 1];
	char	       path[MAX_PATH_LENGTH + 1];	      /* decoded HTTP path, do not modify */
	char	       path_translated[MAX_PATH_LENGTH + 1];  /* physical path */
	char	       path_translated2[MAX_PATH_LENGTH + 1]; /* physical path, but calculated from path_virtual2 */
	char	       path_raw[MAX_PATH_LENGTH + 1];	      /* raw HTTP path */
	char	       path_virtual[MAX_PATH_LENGTH + 1];     /* virtual path */
	char	       path_virtual2[MAX_PATH_LENGTH + 1];    /* virtual path */
	char	       query[MAX_QUERY_LENGTH + 1];
	char	       version[MAX_VERSION_LENGTH + 1];
	fr_stringkv_t* headers;

	char* server_name;
	int   port;

	char handler[MAX_HANDLER_LENGTH + 1];

	void* body;
	int   body_seek;
	int   body_size;
};

struct fr_response {
	int	       status_code;
	char	       status_text[MAX_STATUS_TEXT_LENGTH + 1];
	fr_stringkv_t* headers;

	int (*body_stream)(fr_response_t* res, unsigned char* buffer, int size);
	void* body;
	void* body_opaque;
	int   body_seek;
	int   body_size;

	void (*cleanup)(fr_response_t* res);
};

struct fr_config_section_vhost {
	char** entry;
};

union fr_config_section_union {
	fr_config_section_vhost_t vhost;
};

struct fr_config {
	char* name;

	fr_stringkv_t*	    kv;
	fr_stringarraykv_t* arraykv;

	fr_config_section_union_t section;

	fr_config_t*  parent;
	fr_config_t** children;
};

struct fr_context {
	fr_config_t* config_root;
	fr_config_t* config_current;
	fr_config_t* config_vhost;

	const char* config_path;
	const char* argv0;

	char* server;

	fr_stringkv_t* mime_types;

	int loop;
	int loop2; /* incremented when one of last modules return FR_MODULE_LOOP */

	char* (*path_transform)(const char* path);
	void (*log)(const char* fmt, ...);
	void (*register_hook)(fr_hook_t handler, int order);

	void (*request_set_header)(fr_request_t* req, const char* key, const char* value);
	char* (*request_get_header)(fr_request_t* req, const char* key);
	void (*request_assume_handler)(fr_request_t* req, fr_context_t* context);

	void (*response_set_header)(fr_response_t* res, const char* key, const char* value);
	char* (*response_get_header)(fr_response_t* res, const char* key);

	char* (*stringkv_lookup)(fr_stringkv_t* kv, const char* key);
	void (*stringkv_set)(fr_stringkv_t** kv, const char* key, const char* value);

	char** (*stringarraykv_lookup)(fr_stringarraykv_t* arraykv, const char* key);
	void (*stringarraykv_push)(fr_stringarraykv_t* kv, const char* key, const char* value);
	int (*stringarraykv_length)(fr_stringarraykv_t* arraykv, const char* key);

	char* (*config_lookup)(fr_context_t* context, const char* key);
	char** (*config_lookup_array)(fr_context_t* context, const char* key, int* len);
};

#if defined(_FHTTPD)
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <ctype.h>
#if !defined(_WIN32)
#include <unistd.h>
#include <signal.h>
#endif

#if defined(HAS_SSL)
#include <openssl/ssl.h>
#include <openssl/opensslv.h>
#endif

#define SAFECALL(x) \
	if(x != NULL) x
#define SAFECALL_RET(r, x) \
	if(x != NULL) r = x

enum client_state {
	CS_WANT_SSL = 0,
	CS_CONNECTED,
	CS_GOT_METHOD,
	CS_GOT_PATH,
	CS_GOT_QUERY, /* also applies to case where query does not exist */
	CS_GOT_VERSION,
	CS_GOT_HEADER,
	CS_GOT_BODY,
	CS_SENT_HEADER
};

typedef struct port	port_t;
typedef struct client	client_t;
typedef struct clientkv clientkv_t;

struct port {
	int	 port;
	fpr_bool ssl;
	int	 fd;
};

struct client {
	struct fpr_sockaddr_storage address;
#if defined(HAS_SSL)
	SSL_CTX* ctx;
	SSL*	 ssl;
#endif
	time_t last;
	int    state;

	int fd;

	int port;

	fr_request_t  request;
	fr_response_t response;
	char	      header[LINE_SIZE + 1]; /* do not access this unless you know what this is ... */

	unsigned char leftover[BUFFER_SIZE];
	int	      leftover_seek;
	int	      leftover_size;
};

struct clientkv {
	int	 key;
	client_t value;
};

/* main.c */
extern char*	argv0;
extern fpr_bool running;
extern char	server[];

/* config.c */
extern char*	    config_serverroot;
extern char*	    config_pidfile;
extern char*	    config_logfile;
extern char*	    config_mimefile;
extern fr_config_t* config_root;
extern fr_config_t* config_current;
extern port_t*	    config_ports;

void	     config_init(void);
fpr_bool     config_parse(const char* path);
void	     config_close(void);
fr_config_t* config_vhost_match(const char* host, int port);

/* path.c */
char* path_transform(const char* path);

/* arg.c */
char** arg_parse(const char* str);
void   arg_free(char** args);
int    arg_len(char** args);

/* log.c */
void log_init(void);
void log_srv(const char* fmt, ...);
void log_vasrv(const char* fmt, va_list va);
void log_nofile(void);
void log_close(void);

/* server.c */
#if !defined(MULTITHREAD)
extern clientkv_t* server_clients;
#endif

fpr_bool server_init(void);
void	 server_close(void);
void	 server_loop(void);
int	 server_read(client_t* c, void* buffer, int len);
int	 server_write(client_t* c, void* buffer, int len);

/* ssl.c */
#if defined(HAS_SSL)
SSL_CTX* ssl_create_context(int port);
#endif

/* http.c */
void	 http_init(client_t* c);
void	 http_end(client_t* c);
fpr_bool http_got(client_t* c, void* buffer, int size, int* last);
void	 http_req(client_t* c);
fpr_bool http_send(client_t* c);
void	 http_req_set_header(fr_request_t* req, const char* key, const char* value);
char*	 http_req_get_header(fr_request_t* req, const char* key);
void	 http_req_assume_handler(fr_request_t* req, fr_context_t* context);
void	 http_res_set_header(fr_response_t* res, const char* key, const char* value);
char*	 http_res_get_header(fr_response_t* res, const char* key);

/* mime.c */
extern fr_stringkv_t* mime_types;

void mime_parse(void);
void mime_close(void);

/* module.c */
extern fr_module_t** module_modules;
extern fr_hook_t*    module_first_hooks;
extern fr_hook_t*    module_middle_hooks;
extern fr_hook_t*    module_last_hooks;

void module_init(void);
void module_load(fr_module_t* module);
void module_register_hook(fr_hook_t handler, int order);

/* context.c */
void   context_init(fr_context_t* context);
void   context_save(fr_context_t* context);
char*  context_config_lookup(fr_context_t* context, const char* key);
char** context_config_lookup_array(fr_context_t* context, const char* key, int* len);

/* util.c */
char*  util_stringkv_lookup(fr_stringkv_t* kv, const char* key);
void   util_stringkv_set(fr_stringkv_t** kv, const char* key, const char* value);
char** util_stringarraykv_lookup(fr_stringarraykv_t* arraykv, const char* key);
void   util_stringarraykv_push(fr_stringarraykv_t* kv, const char* key, const char* value);
int    util_stringarraykv_length(fr_stringarraykv_t* arraykv, const char* key);

/* machdep_psp.c */
#if defined(_PSP)
void psp_init(void);
void psp_wait(void);
void psp_exit(int x);
#endif

/* wait routine for exit */
#if defined(_PSP)
#define EXIT(x) \
	{ \
		psp_wait(); \
		psp_exit((x)); \
	}
#else
#define EXIT(x) \
	{ \
		exit((x)); \
	}
#endif

#endif
#endif

#endif
