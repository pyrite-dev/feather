#include <fhttpd.h>

#include <stb_ds.h>

#define LOAD(x) extern fr_module_t* x;
MODULES
#undef LOAD

fr_module_t** module_modules = NULL;

void module_init(void) {
	int		   i	   = 0;
	struct fr_module** modules = NULL;

#define LOAD(x) i++;
	MODULES
#undef LOAD

	modules = malloc(sizeof(*modules) * (i + 1));
	i	= 0;

#define LOAD(x) modules[i++] = x;
	MODULES
#undef LOAD
	modules[i] = NULL;

	for(i = 0; modules[i] != NULL; i++) {
		module_load(modules[i]);
	}
	free(modules);
}

void module_load(fr_module_t* module) {
	arrput(module_modules, module);
}
