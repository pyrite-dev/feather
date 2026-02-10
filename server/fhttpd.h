#ifndef __FHTTPD_H__
#define __FHTTPD_H__

#include "../config.h"

#include <fpr.h>
#include <fr.h>

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#ifdef HAS_SSL
#include <openssl/ssl.h>
#endif

#define VERSION "0.0.0"
#define SERVER "Feather/" VERSION

enum client_state {
	CS_WANT_SSL = 0,
	CS_CONNECTED
};

typedef struct config	config_t;
typedef struct port	port_t;
typedef struct client	client_t;
typedef struct clientkv clientkv_t;

struct config {
	char* name;

	char* docroot;

	config_t*  parent;
	config_t** children;
};

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
};

struct clientkv {
	int	 key;
	client_t value;
};

/* main.c */
extern char* argv0;

/* config.c */
extern char*	 config_serverroot;
extern char*	 config_pidfile;
extern char*	 config_logfile;
extern config_t* config_root;
extern port_t*	 config_ports;

void	 config_init(void);
fpr_bool config_parse(const char* path);
void	 config_close(void);

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
extern fr_server_context_t server_context;
#if !defined(MULTITHREAD)
extern clientkv_t* server_clients;
#endif

fpr_bool server_init(void);
void	 server_close(void);
void	 server_loop(void);

/* ssl.c */
#if defined(HAS_SSL)
SSL_CTX* ssl_create_context(int port);
#endif

#endif
