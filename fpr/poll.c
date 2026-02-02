#include <fpr.h>
#include <fpr_int.h>

int fpr_poll(struct fpr_pollfd* fds, int nfds, int timeout) {
#ifdef HAS_POLL
	struct pollfd* pfds = malloc(sizeof(*pfds) * nfds);
	int	       i;
	int	       st;

	for(i = 0; i < nfds; i++) {
		pfds[i].fd     = fds[i].fd;
		pfds[i].events = 0;
		fds[i].revents = 0;

		if(fds[i].events & FPR_POLLIN) pfds[i].events |= POLLIN;
		if(fds[i].events & FPR_POLLPRI) pfds[i].events |= POLLPRI;
		if(fds[i].events & FPR_POLLOUT) pfds[i].events |= POLLOUT;
	}

	st = poll(pfds, nfds, timeout);

	if(st > 0) {
		for(i = 0; i < nfds; i++) {
			if(pfds[i].revents & FPR_POLLIN) fds[i].revents |= POLLIN;
			if(pfds[i].revents & FPR_POLLPRI) fds[i].revents |= POLLPRI;
			if(pfds[i].revents & FPR_POLLOUT) fds[i].revents |= POLLOUT;
		}
	}

	free(pfds);

	return st;
#else
	fd_set	       rfds;
	fd_set	       wfds;
	int	       i;
	struct timeval tv;
	int	       st;

	FD_ZERO(&rfds);
	FD_ZERO(&wfds);

	for(i = 0; i < nfds; i++) {
		if(fds[i].events & FPR_POLLIN) FD_SET(fds[i].fd, &rfds);
		if(fds[i].events & FPR_POLLOUT) FD_SET(fds[i].fd, &wfds);

		fds[i].revents = 0;
	}

	tv.tv_sec  = timeout / 1000;
	tv.tv_usec = (timeout % 1000) * 1000;

	st = select(FD_SETSIZE, &rfds, &wfds, NULL, &tv);

	for(i = 0; i < nfds; i++) {
		if(FD_ISSET(fds[i].fd, &rfds)) fds[i].revents |= POLLIN;
		if(FD_ISSET(fds[i].fd, &wfds)) fds[i].revents |= POLLOUT;
	}

	return st;
#endif
}
