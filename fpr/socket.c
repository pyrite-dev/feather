#include <fpr.h>
#include <fpr_int.h>

#if !defined(IPPROTO_TCP)
#define IPPROTO_TCP 0
#endif

#if !defined(IPPROTO_UDP)
#define IPPROTO_UDP 0
#endif

void fpr_socket_init(void) {
#if defined(_WIN32)
	WSADATA wsa;

	WSAStartup(MAKEWORD(2, 0), &wsa);
#else
#endif
}

void fpr_socket_uninit(void) {
#if defined(_WIN32)
	WSACleanup();
#else
#endif
}

int fpr_socket(int domain, int type, int protocol) {
	int d = PF_UNSPEC, t = 0, p = 0;
	int s;

	if(domain == FPR_PF_INET) {
		d = PF_INET;
	} else if(domain == FPR_PF_INET6) {
#if defined(PF_INET6)
		d = PF_INET6;
#endif
	} else if(domain == FPR_PF_UNIX) {
#if defined(PF_UNIX)
		d = PF_UNIX;
#endif
	}

	if(type == FPR_SOCK_STREAM) {
		t = SOCK_STREAM;
	} else if(type == FPR_SOCK_DGRAM) {
		t = SOCK_DGRAM;
	}

	if(protocol == FPR_IPPROTO_TCP) {
		p = IPPROTO_TCP;
	} else if(protocol == FPR_IPPROTO_UDP) {
		p = IPPROTO_UDP;
	}

	s = socket(d, t, p);

#if defined(_WIN32)
	if(s < INVALID_SOCKET) s = -1;
#endif

	return s;
}

/* we ignore flags for now */
int fpr_recv(int s, void* buf, int len, int flags) {
	return recv(s, buf, len, 0);
}

int fpr_send(int s, const void* msg, int len, int flags) {
	return send(s, msg, len, 0);
}

int fpr_bind(int s, const struct fpr_sockaddr* name, int namelen) {
	int st = -1;

	if(name->sa_family == FPR_AF_INET && namelen == sizeof(struct fpr_sockaddr_in)) {
		struct fpr_sockaddr_in* addr = (struct fpr_sockaddr_in*)name;
		struct sockaddr_in	addr4;

		addr4.sin_family      = AF_INET;
		addr4.sin_addr.s_addr = addr->sin_addr.u.addr32[0];
		addr4.sin_port	      = addr->sin_port;

		st = bind(s, (struct sockaddr*)&addr4, sizeof(addr4));
	} else if(name->sa_family == FPR_AF_INET6 && namelen == sizeof(struct fpr_sockaddr_in6)) {
#if defined(HAS_IPV6)
		struct fpr_sockaddr_in6* addr = (struct fpr_sockaddr_in6*)name;
		struct sockaddr_in6	 addr6;

		addr6.sin6_family = AF_INET6;
		memcpy(addr6.sin6_addr.s6_addr, &addr->sin6_addr.u.addr8, 16);
		addr6.sin6_port = addr->sin6_port;

		st = bind(s, (struct sockaddr*)&addr6, sizeof(addr6));
#else
		st = -1;
#endif
	} else if(name->sa_family == FPR_AF_UNIX && namelen == sizeof(struct fpr_sockaddr_un)) {
#if defined(HAS_UNIX_SOCKET)
		struct fpr_sockaddr_un* addr = (struct fpr_sockaddr_un*)name;
		struct sockaddr_un	addru;

		addru.sun_family = AF_UNIX;
		strcpy(addru.sun_path, addr->sun_path);

		st = bind(s, (struct sockaddr*)&addru, sizeof(addru));
#else
		st = -1;
#endif
	}

	return st;
}

int fpr_listen(int s, int backlog) {
	return listen(s, backlog);
}

int fpr_accept(int s, struct fpr_sockaddr* addr, int* addrlen) {
	unsigned char	 buffer[256];
	int		 len = 256;
	struct sockaddr* sa  = (struct sockaddr*)buffer;
	int		 r;

	r = accept(s, sa, &len);

	if(r < 0) return r;

	if(sa->sa_family == AF_INET) {
		struct fpr_sockaddr_in* taddr = (struct fpr_sockaddr_in*)addr;
		struct sockaddr_in*	addr4 = (struct sockaddr_in*)buffer;

		taddr->sin_family	    = FPR_AF_INET;
		taddr->sin_addr.u.addr32[0] = addr4->sin_addr.s_addr;
		taddr->sin_port		    = addr4->sin_port;

		*addrlen = sizeof(*taddr);
	} else if(sa->sa_family == AF_INET6) {
#if defined(HAS_IPV6)
		struct fpr_sockaddr_in6* taddr = (struct fpr_sockaddr_in6*)addr;
		struct sockaddr_in6*	 addr6 = (struct sockaddr_in6*)buffer;

		taddr->sin6_family = FPR_AF_INET6;
		memcpy(taddr->sin6_addr.u.addr32, addr6->sin6_addr.s6_addr, 16);
		taddr->sin6_port = addr6->sin6_port;

		*addrlen = sizeof(*taddr);
#endif
	} else if(sa->sa_family == AF_UNIX) {
#if defined(HAS_UNIX_SOCKET)
		struct fpr_sockaddr_un* taddr = (struct fpr_sockaddr_un*)addr;
		struct sockaddr_un*	addru = (struct sockaddr_un*)buffer;

		taddr->sun_family = FPR_AF_UNIX;
		strcpy(taddr->sun_path, addru->sun_path);

		*addrlen = sizeof(*taddr);
#endif
	}

	return r;
}

void fpr_socket_close(int d) {
#if defined(_WIN32)
	closesocket(d);
#else
	close(d);
#endif
}

struct fpr_in_addr fpr_inaddr_any = {
    {0, 0, 0, 0}};

struct fpr_in6_addr fpr_in6addr_any = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
