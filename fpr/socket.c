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

void fpr_socket(int domain, int type, int protocol) {
	int d = PF_UNSPEC, t = 0, p = 0;
	int s;

	if(domain == FPR_PF_INET) {
		d = PF_INET;
	} else if(domain == FPR_PF_INET6) {
#if defined(PF_INET6)
		d = PF_INET6;
#endif
	}

	if(type == FPR_SOCK_STREAM) {
		t = SOCK_STREAM;
	} else if(type == FPR_SOCK_DGRAM) {
		t = SOCK_DGRAM;
	}

	if(protocol == FPR_IPPROTO_TCP) {
		t = IPPROTO_TCP;
	} else if(protocol == FPR_IPPROTO_UDP) {
		t = IPPROTO_UDP;
	}

	s = socket(d, t, p);

#if defined(_WIN32)
	if(s < INVALID_SOCKET) s = -1;
#endif
}

/* we ignore flags for now */
int fpr_recv(int s, void* buf, int len, int flags) {
	return recv(s, buf, len, 0);
}

int fpr_send(int s, const void* msg, int len, int flags) {
	return send(s, msg, len, 0);
}

void fpr_socket_close(int d) {
#if defined(_WIN32)
	closesocket(d);
#else
	close(d);
#endif
}
