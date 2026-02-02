#include <fpr.h>
#include <fpr_int.h>

FPR_FILE* fpr_fopen(const char* path, const char* mode) {
#if defined(_WIN32)
#else
	return (FILE*)fopen(path, mode);
#endif
}

int fpr_fread(void* ptr, int size, int nmemb, FPR_FILE* stream) {
#if defined(_WIN32)
#else
	return fread(ptr, size, nmemb, (FILE*)stream);
#endif
}

int fpr_fwrite(const void* ptr, int size, int nmemb, FPR_FILE* stream) {
#if defined(_WIN32)
#else
	return fwrite(ptr, size, nmemb, (FILE*)stream);
#endif
}

void fpr_fclose(FPR_FILE* stream) {
#if defined(_WIN32)
#else
	fclose((FILE*)stream);
#endif
}
