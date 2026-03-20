#include <fpr.h>
#include <fpr_int.h>

typedef struct process {
#if defined(FPR_IS_WIN32)

#elif defined(FPR_IS_UNIX)
	int pid;
	int fd_stdin;
	int fd_stdout;
	int fd_stderr;
#endif
} process_t;

void* fpr_process_create(const char* exec, char** env) {
#if defined(FPR_IS_WIN32)
#elif defined(FPR_IS_UNIX)
	process_t* proc = malloc(sizeof(*proc));
	int	   pipe_stdin[2];
	int	   pipe_stdout[2];
	int	   pipe_stderr[2];
	char**	   envs = NULL;
	int	   i;
	int	   c = 0;

	extern char** environ;

	for(i = 0; environ[i] != NULL; i++) c++;
	for(i = 0; env != NULL && env[i] != NULL; i++) c++;

	envs = malloc(sizeof(*envs) * (c + 1));

	c = 0;
	for(i = 0; environ[i] != NULL; i++) envs[c++] = environ[i];
	for(i = 0; env != NULL && env[i] != NULL; i++) envs[c++] = env[i];
	envs[c] = 0;

	pipe(pipe_stdin);
	pipe(pipe_stdout);
	pipe(pipe_stderr);

	if((proc->pid = fork()) == 0) {
		int basefd = pipe_stdout[1] > pipe_stderr[1] ? pipe_stdout[1] : pipe_stderr[1];
		int infd   = basefd + 3;
		int outfd  = basefd + 4;
		int errfd  = basefd + 5;

		free(proc);

		close(pipe_stdin[1]);
		close(pipe_stdout[0]);
		close(pipe_stderr[0]);

		dup2(pipe_stdin[0], infd);
		dup2(pipe_stdout[1], outfd);
		dup2(pipe_stderr[1], errfd);
		close(pipe_stdin[0]);
		close(pipe_stdout[1]);
		close(pipe_stderr[1]);
		dup2(infd, 0);
		dup2(outfd, 1);
		dup2(errfd, 2);
		close(infd);
		close(outfd);
		close(errfd);

		execlpe(exec, exec, NULL, envs);

		_exit(-1);
	} else {
		close(pipe_stdin[0]);
		close(pipe_stdout[1]);
		close(pipe_stderr[1]);

		proc->fd_stdin	= pipe_stdin[1];
		proc->fd_stdout = pipe_stdout[0];
		proc->fd_stderr = pipe_stderr[0];
	}

	free(envs);

	return proc;
#endif
}

void fpr_process_close(void* handle) {
	process_t* proc = handle;
#if defined(FPR_IS_WIN32)
#elif defined(FPR_IS_UNIX)
	close(proc->fd_stdin);
#endif
}

int fpr_process_write(void* handle, const void* data, int len) {
	process_t* proc = handle;
#if defined(FPR_IS_WIN32)
#elif defined(FPR_IS_UNIX)
	return write(proc->fd_stdin, data, len);
#endif
}

int fpr_process_read(void* handle, void* data, int len) {
	process_t* proc = handle;
#if defined(FPR_IS_WIN32)
#elif defined(FPR_IS_UNIX)
	return read(proc->fd_stdout, data, len);
#endif
}

void fpr_process_destroy(void* handle) {
	process_t* proc = handle;
#if defined(FPR_IS_WIN32)
#elif defined(FPR_IS_UNIX)
	int st;

	do {
		waitpid(proc->pid, &st, 0);
	} while(!WIFEXITED(st));

	close(proc->fd_stdin);
	close(proc->fd_stdout);
	close(proc->fd_stderr);
#endif
	free(proc);
}
