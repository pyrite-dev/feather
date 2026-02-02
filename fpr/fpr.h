#ifndef __FPR_H__
#define __FPR_H__

typedef unsigned char fpr_bool;
#define FPR_FALSE 0
#define FPR_TRUE 1

typedef unsigned char  fpr_uint8_t;
typedef unsigned short fpr_uint16_t;
typedef unsigned int   fpr_uint32_t;
typedef unsigned long  fpr_uint64_t;

typedef signed char fpr_int8_t;
typedef short	    fpr_int16_t;
typedef int	    fpr_int32_t;
typedef long	    fpr_int64_t;

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

struct fpr_sockaddr {
	unsigned short sa_family;
	char	       sa_data[14];
};

union fpr_in_addr_union {
	fpr_uint8_t  addr8[4];
	fpr_uint16_t addr16[2];
	fpr_uint32_t addr32[1];
};

struct fpr_in_addr {
	union fpr_in_addr_union u;
};

struct fpr_sockaddr_in {
	unsigned short	   sin_family;
	fpr_uint16_t	   sin_port;
	struct fpr_in_addr sin_addr;
};

union fpr_in6_addr_union {
	fpr_uint8_t  addr8[16];
	fpr_uint16_t addr16[8];
	fpr_uint32_t addr32[4];
};

struct fpr_in6_addr {
	union fpr_in6_addr_union u;
};

struct fpr_sockaddr_in6 {
	unsigned short	    sin6_family;
	fpr_uint16_t	    sin6_port;
	struct fpr_in6_addr sin6_addr;
};

struct fpr_sockddr_storage {
	unsigned short ss_family;
	char	       ss_pad[128];
};

/* poll.c */
int fpr_poll(struct fpr_pollfd* fds, int nfds, int timeout);

/* file.c */
FPR_FILE* fpr_fopen(const char* path, const char* mode);
int	  fpr_fread(void* ptr, int size, int nmemb, FPR_FILE* stream);
int	  fpr_fwrite(const void* ptr, int size, int nmemb, FPR_FILE* stream);
void	  fpr_fclose(FPR_FILE* stream);

/* socket.c */
extern struct fpr_in_addr  FPR_INADDR_ANY;
extern struct fpr_in6_addr FPR_IN6ADDR_ANY;

void fpr_socket_init(void);
int  fpr_socket(int domain, int type, int protocol);
int  fpr_recv(int s, void* buf, int len, int flags);
int  fpr_send(int s, const void* msg, int len, int flags);
int  fpr_bind(int s, const struct fpr_sockaddr* name, int namelen);
int  fpr_listen(int s, int backlog);
int  fpr_accept(int s, struct fpr_sockaddr* addr, int* addrlen);
void fpr_socket_close(int d);

/* arpa.c */
fpr_uint16_t fpr_htons(fpr_uint16_t host16);

/* wildcard.c */
fpr_bool fpr_wildcard(const char* wildcard, const char* target);

#endif
