#ifndef __FHTTPD_H__
#define __FHTTPD_H__

#include "../config.h"

#include <fpr.h>

enum fr_module_version {
	FR_MODULE_00 = 0
};

typedef struct fr_module	       fr_module_t;
typedef struct fr_stringkv	       fr_stringkv_t;
typedef struct fr_request	       fr_request_t;
typedef union fr_config_section_union  fr_config_section_union_t;
typedef struct fr_config_section_vhost fr_config_section_vhost_t;
typedef struct fr_config	       fr_config_t;
typedef struct fr_context	       fr_context_t;

struct fr_stringkv {
	char* key;
	char* value;
};

struct fr_module {
	int version;

	fpr_bool (*directive)(fr_context_t* context, int argc, char** argv);
};

struct fr_request {
	char	       method[MAX_METHOD_LENGTH + 1];
	char	       path[MAX_PATH_LENGTH + 1];
	char	       version[MAX_VERSION_LENGTH + 1];
	fr_stringkv_t* headers;
};

struct fr_config_section_vhost {
	char** entry;
};

union fr_config_section_union {
	fr_config_section_vhost_t vhost;
};

struct fr_config {
	char* name;

	fr_stringkv_t* kv;

	fr_config_section_union_t section;

	fr_config_t*  parent;
	fr_config_t** children;
};

struct fr_context {
	fr_config_t* root;
	fr_config_t* current;
};

#if defined(_FHTTPD)
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <ctype.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#ifdef HAS_SSL
#include <openssl/ssl.h>
#include <openssl/opensslv.h>
#endif

#define VERSION "0.0.0"
#define SERVER "Feather/" VERSION

#define SAFECALL(x) \
	if(x != NULL) x
#define SAFECALL_RET(r, x) \
	if(x != NULL) r = x

enum client_state {
	CS_WANT_SSL = 0,
	CS_CONNECTED,
	CS_GOT_METHOD,
	CS_GOT_PATH,
	CS_GOT_VERSION,
	CS_GOT_HEADER
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

	int port;

	fr_request_t request;
	char	     header[LINE_SIZE + 1]; /* do not access this unless you know what this is ... */
};

struct clientkv {
	int	 key;
	client_t value;
};

/* main.c */
extern char* argv0;

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
int	 server_read(int fd, void* buffer, int len);
int	 server_write(int fd, void* buffer, int len);

/* ssl.c */
#if defined(HAS_SSL)
SSL_CTX* ssl_create_context(int port);
#endif

/* http.c */
fpr_bool http_got(client_t* c, void* buffer, int size);
void	 http_req(client_t* c);

/* mime.c */
extern fr_stringkv_t* mime_types;

void mime_parse(void);
void mime_close(void);

/* module.c */
extern fr_module_t** module_modules;

void module_init(void);
void module_load(fr_module_t* module);

/* context.c */
void context_init(fr_context_t* context);
void context_save(fr_context_t* context);
#endif

#endif
