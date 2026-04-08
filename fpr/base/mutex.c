#include <fpr.h>
#include <fpr_int.h>

void* fpr_mutex_create(void) {
#if defined(FPR_IS_WIN32)
	return CreateEvent(NULL, FALSE, TRUE, NULL);
#elif defined(FPR_USE_PTHREAD)
	pthread_mutex_t* mutex = malloc(sizeof(*mutex));

	pthread_mutex_init(mutex, NULL);

	return mutex;
#elif defined(FPR_IS_NETWARE) && defined(FPR_DANGER_SEMAPHORE)
	LONG* mutex = malloc(sizeof(*mutex));

	*mutex = OpenLocalSemaphore(1);

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
#elif defined(FPR_IS_NETWARE) && defined(FPR_DANGER_SEMAPHORE)
	WaitOnLocalSemaphore(*(LONG*)handle);
#else
	(void)handle;
#endif
}

void fpr_mutex_unlock(void* handle) {
#if defined(FPR_IS_WIN32)
	SetEvent(handle);
#elif defined(FPR_USE_PTHREAD)
	pthread_mutex_unlock(handle);
#elif defined(FPR_IS_NETWARE) && defined(FPR_DANGER_SEMAPHORE)
	SignalLocalSemaphore(*(LONG*)handle);
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
#elif defined(FPR_IS_NETWARE) && defined(FPR_DANGER_SEMAPHORE)
	CloseLocalSemaphore(*(LONG*)handle);

	free(handle);
#else
	(void)handle;
#endif
}
