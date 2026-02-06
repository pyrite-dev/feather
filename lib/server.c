#include <fr.h>

#include <stb_ds.h>

void fr_server_init(fr_server_context_t* ctx){
	ctx->modules = NULL;
}

void fr_server_uninit(fr_server_context_t* ctx){
	arrfree(ctx->modules);
	ctx->modules = NULL;
}

void fr_server_load_module(fr_server_context_t* ctx, fr_module_t* mod){
	if(mod->version != FR_MODULE_00){
	}

	arrput(ctx->modules, mod);
}
