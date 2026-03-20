#include <fpr.h>
#include <fpr_int.h>

#if defined(FPR_DL_IS_DUMMY)
void* fpr_dlopen(const char* path) {
	return NULL;
}

void* fpr_dlsym(void* handle, const char* symbol) {
	return NULL;
}

int fpr_dlclose(void* handle) {
	return 1;
}
#else

void* fpr_dlopen(const char* path) {
#if defined(FPR_IS_WIN32)
	return LoadLibrary(path);
#elif defined(FPR_IS_UNIX)
	return dlopen(path, RTLD_LAZY | RTLD_LOCAL);
#endif
}

void* fpr_dlsym(void* handle, const char* symbol) {
#if defined(FPR_IS_WIN32)
	return GetProcAddress(handle, symbol);
#elif defined(FPR_IS_UNIX)
	return dlsym(handle, symbol);
#endif
}

int fpr_dlclose(void* handle) {
#if defined(FPR_IS_WIN32)
	return FreeLibrary(handle) ? 0 : 1;
#elif defined(FPR_IS_UNIX)
	return dlclose(handle);
#endif
}
#endif
