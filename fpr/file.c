#include <fpr.h>
#include <fpr_int.h>
#include <fcntl.h>

FPR_FILE* fpr_fopen(const char* path, const char* mode) {
#if defined(FPR_IS_WIN32)
	DWORD  ac  = 0;
	DWORD  dis = 0;
	DWORD  fl  = FILE_ATTRIBUTE_NORMAL;
	HANDLE h;

	if(mode[0] == 'r') {
		ac  = GENERIC_READ;
		dis = OPEN_EXISTING;
	} else if(mode[0] == 'w') {
		ac  = GENERIC_WRITE;
		dis = CREATE_ALWAYS;
	} else if(mode[0] == 'a') {
		ac  = GENERIC_WRITE | FILE_APPEND_DATA;
		dis = OPEN_ALWAYS;
	}

	h = CreateFile(path, ac, FILE_SHARE_READ, NULL, dis, fl, NULL);
	if(h == INVALID_HANDLE_VALUE) return NULL;

	return (FPR_FILE*)h;
#else
	return (FPR_FILE*)fopen(path, mode);
#endif
}

int fpr_fread(void* ptr, int size, int nmemb, FPR_FILE* stream) {
#if defined(FPR_IS_WIN32)
	DWORD dw;

	if(!ReadFile((HANDLE)stream, ptr, size * nmemb, &dw, NULL)) return -1;

	return dw;
#else
	return fread(ptr, size, nmemb, (FILE*)stream);
#endif
}

int fpr_fwrite(const void* ptr, int size, int nmemb, FPR_FILE* stream) {
#if defined(FPR_IS_WIN32)
	DWORD dw;

	if(!WriteFile((HANDLE)stream, ptr, size * nmemb, &dw, NULL)) return -1;

	return dw;
#else
	return fwrite(ptr, size, nmemb, (FILE*)stream);
#endif
}

void fpr_fclose(FPR_FILE* stream) {
#if defined(FPR_IS_WIN32)
	CloseHandle((HANDLE)stream);
#else
	fclose((FILE*)stream);
#endif
}
