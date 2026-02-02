#ifndef __FHTTPD_H__
#define __FHTTPD_H__

#include "../config.h"

#include <fpr.h>

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define VERSION "0.0.0"
#define SERVER "Feather/" VERSION

typedef struct config config_t;

struct config {
};

/* main.c */
extern char* argv0;

/* config.c */
extern char* serverroot;

void	 config_init(void);
fpr_bool config_parse(const char* path);

/* path.c */
char* path_transform(const char* path);

/* arg.c */
char** arg_parse(const char* str);
void   arg_free(char** args);
int    arg_len(char** args);

#endif
