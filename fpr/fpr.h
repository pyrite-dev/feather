#ifndef __FPR_H__
#define __FPR_H__

typedef unsigned char fpr_bool;
#define FPR_FALSE 0
#define FPR_TRUE 1

/* poll.c definitions */
#define FPR_POLLIN (1 << 0)
#define FPR_POLLPRI (1 << 1)
#define FPR_POLLOUT (1 << 2)

struct fpr_pollfd {
	int   fd;
	short events;
	short revents;
};

/* file.c definitions */
typedef void FPR_FILE;

/* socket.c definitions */
enum fpr_socket_protocol {
	FPR_PF_UNSPEC = 0,
	FPR_PF_INET,
	FPR_PF_INET6
};

enum fpr_socket_address {
	FPR_AF_UNSPEC = 0,
	FPR_AF_INET,
	FPR_AF_INET6
};

enum fpr_socket_type {
	FPR_SOCK_STREAM = 0,
	FPR_SOCK_DGRAM
};

enum fpr_socket_ip_protocol {
	FPR_IPPROTO_TCP = 0,
	FPR_IPPROTO_UDP
};

/* poll.c */
int fpr_poll(struct fpr_pollfd* fds, int nfds, int timeout);

/* file.c */
FPR_FILE* fpr_fopen(const char* path, const char* mode);
int	  fpr_fread(void* ptr, int size, int nmemb, FPR_FILE* stream);
int	  fpr_fwrite(const void* ptr, int size, int nmemb, FPR_FILE* stream);
void	  fpr_fclose(FPR_FILE* stream);

/* socket.c */
void fpr_socket_init(void);
void fpr_socket(int domain, int type, int protocol);
int  fpr_recv(int s, void* buf, int len, int flags);
int  fpr_send(int s, const void* msg, int len, int flags);
void fpr_socket_close(int d);

/* wildcard.c */
fpr_bool fpr_wildcard(const char* wildcard, const char* target);

#endif
