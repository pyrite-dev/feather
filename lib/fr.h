#ifndef __FR_H__
#define __FR_H__

typedef struct fr_module fr_module_t;
typedef struct fr_server_context fr_server_context_t;

enum fr_module_version {
	FR_MODULE_00 = 0
};

struct fr_server_context {
	fr_module_t** modules;
};

struct fr_module {
	int version;
};

/* server.c */
void fr_server_init(fr_server_context_t* ctx);
void fr_server_uninit(fr_server_context_t* ctx);
void fr_server_load_module(fr_server_context_t* ctx, fr_module_t* mod);

#endif
