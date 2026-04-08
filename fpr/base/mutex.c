#include <fpr.h>
#include <fpr_int.h>

void* fpr_mutex_create(void) {
#if defined(FPR_IS_WIN32)
	return CreateEvent(NULL, FALSE, TRUE, NULL);
#elif defined(FPR_USE_PTHREAD)
	pthread_mutex_t* mutex = malloc(sizeof(*mutex));

	pthread_mutex_init(mutex, NULL);

	return mutex;
#else
	return NULL;
#endif
}

void fpr_mutex_lock(void* handle) {
#if defined(FPR_IS_WIN32)
	WaitForSingleObject(handle, INFINITE);
#elif defined(FPR_USE_PTHREAD)
	pthread_mutex_lock(handle);
#else
	(void)handle;
#endif
}

void fpr_mutex_unlock(void* handle) {
#if defined(FPR_IS_WIN32)
	SetEvent(handle);
#elif defined(FPR_USE_PTHREAD)
	pthread_mutex_unlock(handle);
#else
	(void)handle;
#endif
}

void fpr_mutex_destroy(void* handle) {
#if defined(FPR_IS_WIN32)
	CloseHandle(handle);
#elif defined(FPR_USE_PTHREAD)
	pthread_mutex_destroy(handle);

	free(handle);
#else
	(void)handle;
#endif
}
