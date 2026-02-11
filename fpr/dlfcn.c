#include <fpr.h>
#include <fpr_int.h>

void* fpr_dlopen(const char* path) {
#if defined(_WIN32)
	return LoadLibrary(path);
#else
	return dlopen(path, RTLD_LAZY | RTLD_LOCAL);
#endif
}

void* fpr_dlsym(void* handle, const char* symbol) {
#if defined(_WIN32)
	return GetProcAddress(handle, symbol);
#else
	return dlsym(handle, symbol);
#endif
}

int fpr_dlclose(void* handle) {
#if defined(_WIN32)
	return FreeLibrary(handle) ? 0 : 1;
#else
	return dlclose(handle);
#endif
}
