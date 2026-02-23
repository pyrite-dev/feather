#include <fhttpd.h>

#include <stb_ds.h>

#define LOAD(x) extern fr_module_t* x##_module;
MODULES
#undef LOAD

fr_module_t** module_modules = NULL;

fr_hook_t* module_first_hooks = NULL;
fr_hook_t* module_middle_hooks = NULL;
fr_hook_t* module_last_hooks = NULL;

void module_init(void) {
	int		   i	   = 0;
	struct fr_module** modules = NULL;

#define LOAD(x) i++;
	MODULES
#undef LOAD

	modules = malloc(sizeof(*modules) * (i + 1));
	i	= 0;

#define LOAD(x) modules[i++] = x##_module;
	MODULES
#undef LOAD
	modules[i] = NULL;

	for(i = 0; modules[i] != NULL; i++) {
		module_load(modules[i]);
	}
	free(modules);
}

void module_load(fr_module_t* module) {
	fr_context_t context;

	context_init(&context);
	SAFECALL(module->register_hooks)(&context);
	context_save(&context);

	arrput(module_modules, module);
}

void module_close(void){
	arrfree(module_modules);
	module_modules = NULL;

	arrfree(module_first_hooks);
	module_first_hooks = NULL;

	arrfree(module_middle_hooks);
	module_middle_hooks = NULL;
	
	arrfree(module_last_hooks);
	module_last_hooks = NULL;
}

void module_register_hook(fr_hook_t handler, int order){
	if(order == FR_MODULE_HOOK_FIRST){
		arrput(module_first_hooks, handler);
	}else if(order == FR_MODULE_HOOK_LAST){
		arrput(module_last_hooks, handler);
	}else{
		arrput(module_middle_hooks, handler);
	}
}
