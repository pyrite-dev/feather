#include <fpr.h>
#include <fpr_int.h>

typedef struct arg {
	void (*entry)(void* param);
	void* param;
#if defined(FPR_IS_NETWARE)
	LONG waitsem;
#endif
} arg_t;

#if defined(FPR_IS_NETWARE)
typedef struct thread {
	int  thread;
	LONG waitsem;
} thread_t;

struct fpr_thread_usage {
	fpr_bool  used;
	thread_t* thread;
};

struct fpr_semaphore_usage fpr_semaphores[1024];
struct fpr_thread_usage	   fpr_threads[1024];
#endif

#if defined(FPR_IS_NETWARE)
long fpr_thread_open_semaphore(long initial) {
	long sem = OpenLocalSemaphore(initial);
	int  i;

	for(i = 0; i < sizeof(fpr_semaphores) / sizeof(fpr_semaphores[0]); i++) {
		if(!fpr_semaphores[i].used) {
			fpr_semaphores[i].used = fpr_true;
			fpr_semaphores[i].sem  = sem;
			break;
		}
	}

	return sem;
}

void fpr_thread_close_semaphore(long sem) {
	int i;

	for(i = 0; i < sizeof(fpr_semaphores) / sizeof(fpr_semaphores[0]); i++) {
		if(fpr_semaphores[i].used && fpr_semaphores[i].sem == sem) {
			fpr_semaphores[i].used = fpr_false;
			break;
		}
	}

	CloseLocalSemaphore(sem);
}
#endif

void fpr_thread_init(void) {
#if defined(FPR_IS_NETWARE)
	int i;

	for(i = 0; i < sizeof(fpr_semaphores) / sizeof(fpr_semaphores[0]); i++) {
		fpr_semaphores[i].used = fpr_false;
	}
	for(i = 0; i < sizeof(fpr_threads) / sizeof(fpr_threads[0]); i++) {
		fpr_threads[i].used = fpr_false;
	}
#endif
}

void fpr_thread_uninit(void) {
#if defined(FPR_IS_NETWARE)
	int i;

	for(i = 0; i < sizeof(fpr_threads) / sizeof(fpr_threads[0]); i++) {
		if(fpr_threads[i].used) {
			fpr_thread_join(fpr_threads[i].thread);
			fpr_thread_destroy(fpr_threads[i].thread);
		}
	}
	for(i = 0; i < sizeof(fpr_semaphores) / sizeof(fpr_semaphores[0]); i++) {
		if(fpr_semaphores[i].used) {
			fpr_thread_close_semaphore(fpr_semaphores[i].sem);
		}
	}
#endif
}

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
	LONG  waitsem		   = arg->waitsem;

	free(arg);

	entry(param);

	SignalLocalSemaphore(waitsem);
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

	arg->entry   = entry;
	arg->param   = param;
	arg->waitsem = fpr_thread_open_semaphore(0);

	t->waitsem = arg->waitsem;
	t->thread  = BeginThread(thread_entry, NULL, 8192, param);

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
	thread_t* t = handle;

	WaitOnLocalSemaphore(t->waitsem);
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
	thread_t* t = handle;
	int	  i;

	fpr_thread_close_semaphore(t->waitsem);

	for(i = 0; i < sizeof(fpr_threads) / sizeof(fpr_threads[0]); i++) {
		if(fpr_threads[i].used && fpr_threads[i].thread == handle) {
			fpr_threads[i].used = fpr_false;
			break;
		}
	}

	free(handle);
#else
	(void)handle;
#endif
}
