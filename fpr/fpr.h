#ifndef __FPR_H__
#define __FPR_H__

#define FPR_POLLIN (1 << 0)
#define FPR_POLLPRI (1 << 1)
#define FPR_POLLOUT (1 << 2)

struct fpr_pollfd {
	int   fd;
	short events;
	short revents;
};

int fpr_poll(struct fpr_pollfd* fds, int nfds, int timeout);

#endif
