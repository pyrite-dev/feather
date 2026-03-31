#include <fhttpd.h>

#include <stb_ds.h>

#define LOAD(x) extern fr_module_t x##_module;
MODULES
#undef LOAD

fr_module_t** module_modules = NULL;

fr_hook_t* module_first_hooks	= NULL;
fr_hook_t* module_middle_hooks	= NULL;
fr_hook_t* module_last_hooks	= NULL;
fr_hook_t* module_rewrite_hooks = NULL;

void module_init(void) {
#define LOAD(x) module_load(&x##_module);
	MODULES
#undef LOAD

	/* this is done here so it doesn't get appended everytime module gets reloaded */
	strcpy(server, FR_SERVER);
#if defined(HAS_SSL)
	strcat(server, " OpenSSL/" OPENSSL_FULL_VERSION_STR);
#endif
}

void module_load(fr_module_t* module) {
	fr_context_t context;

	context_init(&context);
	SAFECALL(module->register_stuff)(&context);
	context_save(&context);

	arrput(module_modules, module);
}

void module_close(void) {
	arrfree(module_modules);
	module_modules = NULL;

	arrfree(module_first_hooks);
	module_first_hooks = NULL;

	arrfree(module_middle_hooks);
	module_middle_hooks = NULL;

	arrfree(module_last_hooks);
	module_last_hooks = NULL;

	arrfree(module_rewrite_hooks);
	module_rewrite_hooks = NULL;
}

void module_register_hook(fr_hook_t handler, int order) {
	if(order == FR_MODULE_HOOK_REWRITE) {
		arrput(module_rewrite_hooks, handler);
	} else if(order == FR_MODULE_HOOK_FIRST) {
		arrput(module_first_hooks, handler);
	} else if(order == FR_MODULE_HOOK_LAST) {
		arrput(module_last_hooks, handler);
	} else {
		arrput(module_middle_hooks, handler);
	}
}
