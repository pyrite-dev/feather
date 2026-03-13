#include <fpr.h>
#include <fpr_int.h>

typedef struct arg {
	void (*entry)(void* param);
	void* param;
} arg_t;

#if defined(_WIN32)
#if defined(USE_CREATETHREAD)
static DWORD WINAPI thread_entry(void* _param) {
#else
static unsigned int WINAPI thread_entry(void* _param) {
#endif
	arg_t* arg		   = _param;
	void (*entry)(void* param) = arg->entry;
	void* param		   = arg->param;

	free(arg);

	entry(param);

	return 0;
}
#else
static void* thread_entry(void* _param) {
	arg_t* arg		   = _param;
	void (*entry)(void* param) = arg->entry;
	void* param		   = arg->param;

	free(arg);

	entry(param);

	return NULL;
}
#endif

void* fpr_thread_create(void (*entry)(void* param), void* param) {
#if defined(_WIN32)
	arg_t* arg = malloc(sizeof(*arg));
#if defined(USE_CREATETHREAD)
	DWORD id;
#else
	unsigned int id;
#endif

	arg->entry = entry;
	arg->param = param;

#if defined(USE_CREATETHREAD)
	CreateThread
#else
	_beginthreadex
#endif
	    (NULL, 0, thread_entry, arg, 0, &id);
#else
	pthread_t* t   = malloc(sizeof(*t));
	arg_t*	   arg = malloc(sizeof(*arg));

	arg->entry = entry;
	arg->param = param;

	pthread_create(t, NULL, thread_entry, arg);

	return t;
#endif
}

void fpr_thread_detach(void* handle) {
#if defined(_WIN32)
	CloseHandle(handle);
#else
	pthread_detach(*(pthread_t*)handle);
	free(handle);
#endif
}

void fpr_thread_join(void* handle) {
#if defined(_WIN32)
	WaitForSingleObject(handle, INFINITE);
#else
	void* ret;

	pthread_join(*(pthread_t*)handle, &ret);
#endif
}

void fpr_thread_destroy(void* handle) {
#if defined(_WIN32)
	CloseHandle(handle);
#else
	free(handle);
#endif
}
