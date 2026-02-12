#include <fhttpd.h>

static fpr_bool directive(fr_context_t* context, int argc, char** argv){
	printf("%s\n", argv[0]);
}

static fr_module_t module = {
	FR_MODULE_00,
	directive
};
fr_module_t* dso_module = &module;
