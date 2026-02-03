#ifndef __FHTTPD_H__
#define __FHTTPD_H__

#include "../config.h"

#include <fpr.h>

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

#define VERSION "0.0.0"
#define SERVER "Feather/" VERSION

typedef struct config config_t;

struct config {
	char* name;

	char* docroot;

	config_t*  parent;
	config_t** children;
};

/* main.c */
extern char* argv0;

/* config.c */
extern char*	 config_serverroot;
extern char*	 config_pidfile;
extern char*	 config_logfile;
extern config_t* config_root;

void	 config_init(void);
fpr_bool config_parse(const char* path);

/* path.c */
char* path_transform(const char* path);

/* arg.c */
char** arg_parse(const char* str);
void   arg_free(char** args);
int    arg_len(char** args);

/* log.c */
void log_init(void);
void log_srv(const char* fmt, ...);

#endif
