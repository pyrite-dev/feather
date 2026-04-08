#include <fpr.h>
#include <fpr_int.h>

typedef struct arg {
	void (*entry)(void* param);
	void* param;
#if defined(FPR_IS_NETWARE) || defined(FPR_DANGER_SEMAPHORE)
	LONG waitsem;
#endif
} arg_t;

#if defined(FPR_IS_NETWARE)
typedef struct thread {
	int thread;
#if defined(FPR_DANGER_SEMAPHORE)
	LONG waitsem;
#endif
} thread_t;
#endif

#if defined(FPR_IS_WIN32)
#if defined(FPR_USE_CREATETHREAD)
static DWORD WINAPI thread_entry(void* _param) {
#else
static unsigned int WINAPI thread_entry(void* _param) {
#endif
	arg_t* arg		   = _param;
	void (*entry)(void* param) = arg->entry;
	void* param		   = arg->param;

	free(arg);

	entry(param);

#if defined(FPR_USE_CREATETHREAD)
	ExitThread(0);
#else
	_endthreadex(0);
#endif

	return 0;
}
#elif defined(FPR_USE_PTHREAD)
static void* thread_entry(void* _param) {
	arg_t* arg		   = _param;
	void (*entry)(void* param) = arg->entry;
	void* param		   = arg->param;

	free(arg);

	entry(param);

	pthread_exit(NULL);

	return NULL;
}
#elif defined(FPR_IS_NETWARE)
static void thread_entry(void* _param) {
	arg_t* arg		   = _param;
	void (*entry)(void* param) = arg->entry;
	void* param		   = arg->param;
#if defined(FPR_DANGER_SEMAPHORE)
	LONG waitsem = arg->waitsem;
#endif

	free(arg);

	entry(param);

#if defined(FPR_DANGER_SEMAPHORE)
	SignalLocalSemaphore(waitsem);
#endif
	ExitThread(EXIT_THREAD, 0);
}
#endif

void* fpr_thread_create(void (*entry)(void* param), void* param) {
#if defined(FPR_IS_WIN32)
	arg_t* arg = malloc(sizeof(*arg));
#if defined(FPR_USE_CREATETHREAD)
	DWORD id;
#else
	unsigned int id;
#endif

	arg->entry = entry;
	arg->param = param;

	return (void*)
#if defined(FPR_USE_CREATETHREAD)
	    CreateThread
#else
	    _beginthreadex
#endif
	    (NULL, 0, thread_entry, arg, 0, &id);
#elif defined(FPR_USE_PTHREAD)
	pthread_t* t   = malloc(sizeof(*t));
	arg_t*	   arg = malloc(sizeof(*arg));

	arg->entry = entry;
	arg->param = param;

	pthread_create(t, NULL, thread_entry, arg);

	return t;
#elif defined(FPR_IS_NETWARE)
	thread_t* t   = malloc(sizeof(*t));
	arg_t*	  arg = malloc(sizeof(*arg));

	arg->entry = entry;
	arg->param = param;
#if defined(FPR_DANGER_SEMAPHORE)
	arg->waitsem = OpenLocalSemaphore(0);

	t->waitsem = arg->waitsem;
#endif
	t->thread = BeginThreadGroup(thread_entry, NULL, 8192, param);

	return t;
#else
	(void)entry;
	(void)param;

	return NULL;
#endif
}

void fpr_thread_join(void* handle) {
#if defined(FPR_IS_WIN32)
	WaitForSingleObject(handle, INFINITE);
#elif defined(FPR_USE_PTHREAD)
	void* ret;

	pthread_join(*(pthread_t*)handle, &ret);
#elif defined(FPR_IS_NETWARE)
#if defined(FPR_DANGER_SEMAPHORE)
	thread_t* t = handle;

	WaitOnLocalSemaphore(t->waitsem);
#else
	(void)handle;
#endif
#else
	(void)handle;
#endif
}

void fpr_thread_destroy(void* handle) {
#if defined(FPR_IS_WIN32)
	CloseHandle(handle);
#elif defined(FPR_USE_PTHREAD)
	free(handle);
#elif defined(FPR_IS_NETWARE)
#if defined(FPR_DANGER_SEMAPHORE)
	thread_t* t = handle;

	CloseLocalSemaphore(t->waitsem);
#endif

	free(handle);
#else
	(void)handle;
#endif
}
