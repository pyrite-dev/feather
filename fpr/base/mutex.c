#include <fpr.h>
#include <fpr_int.h>

void* fpr_mutex_create(void) {
#if defined(FPR_IS_WIN32)
	return CreateEvent(NULL, FALSE, TRUE, NULL);
#elif defined(FPR_USE_PTHREAD)
	pthread_mutex_t* mutex = malloc(sizeof(*mutex));

	pthread_mutex_init(mutex, NULL);

	return mutex;
#elif defined(FPR_IS_NETWARE)
	int n = rand() % 0x1000000;
	char name[128];

	sprintf(name, "mt%d", n);

	return MPKMutexAlloc(name);
#else
	return NULL;
#endif
}

void fpr_mutex_lock(void* handle) {
#if defined(FPR_IS_WIN32)
	WaitForSingleObject(handle, INFINITE);
#elif defined(FPR_USE_PTHREAD)
	pthread_mutex_lock(handle);
#elif defined(FPR_IS_NETWARE)
	MPKMutexLock(handle);
#else
	(void)handle;
#endif
}

void fpr_mutex_unlock(void* handle) {
#if defined(FPR_IS_WIN32)
	SetEvent(handle);
#elif defined(FPR_USE_PTHREAD)
	pthread_mutex_unlock(handle);
#elif defined(FPR_IS_NETWARE)
	MPKMutexUnlock(handle);
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
#elif defined(FPR_IS_NETWARE)
	MPKMutexFree(handle);
#else
	(void)handle;
#endif
}
