#ifndef __FPR_BASE_H__
#define __FPR_BASE_H__

#include <fpr_machdep.h>

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
	FPR_PF_INET6,
	FPR_PF_UNIX
};

enum fpr_socket_address {
	FPR_AF_UNSPEC = 0,
	FPR_AF_INET,
	FPR_AF_INET6,
	FPR_AF_UNIX
};

enum fpr_socket_type {
	FPR_SOCK_STREAM = 0,
	FPR_SOCK_DGRAM
};

enum fpr_socket_ip_protocol {
	FPR_IPPROTO_TCP = 1,
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

struct fpr_sockaddr_storage {
	unsigned short ss_family;
	char	       ss_pad[128];
};

struct fpr_sockaddr_un {
	unsigned short sun_family;
	char	       sun_path[104];
};

/* stat.c definitions */
#define FPR_S_IFREG (1 << 0)
#define FPR_S_IFDIR (1 << 1)
#define FPR_S_ISREG(x) ((x) & FPR_S_IFREG)
#define FPR_S_ISDIR(x) ((x) & FPR_S_IFDIR)

struct fpr_stat {
	long st_size;
	long st_modtime;
	int  st_mode;
};

/* poll.c */
int fpr_poll(struct fpr_pollfd* fds, int nfds, int timeout);

/* file.c */
FPR_FILE* fpr_fopen(const char* path, const char* mode);
int	  fpr_fread(void* ptr, int size, int nmemb, FPR_FILE* stream);
int	  fpr_fwrite(const void* ptr, int size, int nmemb, FPR_FILE* stream);
void	  fpr_fclose(FPR_FILE* stream);

/* socket.c */
extern struct fpr_in_addr  fpr_inaddr_any;
extern struct fpr_in6_addr fpr_in6addr_any;

void	 fpr_socket_init(void);
int	 fpr_socket(int domain, int type, int protocol);
int	 fpr_recv(int s, void* buf, int len, int flags);
int	 fpr_send(int s, const void* msg, int len, int flags);
int	 fpr_bind(int s, const struct fpr_sockaddr* name, int namelen);
int	 fpr_connect(int s, const struct fpr_sockaddr* name, int namelen);
int	 fpr_listen(int s, int backlog);
int	 fpr_accept(int s, struct fpr_sockaddr* addr, int* addrlen);
void	 fpr_socket_close(int d);
void	 fpr_socket_uninit(void);
fpr_bool fpr_socket_has_ipv6(void);

/* arpa.c */
fpr_uint16_t	     fpr_htons(fpr_uint16_t host16);
const char*	     fpr_inet_ntop(struct fpr_sockaddr* src, char* dst);
struct fpr_sockaddr* fpr_inet_addr(const char* addr, int* len);

/* string.c */
char* fpr_strdup(const char* str);
char* fpr_strvacat(const char* a, ...);

/* dlfcn.c */
void* fpr_dlopen(const char* path);
void* fpr_dlsym(void* handle, const char* symbol);
int   fpr_dlclose(void* handle);

/* unistd.c */
int fpr_gethostname(char* name, int namelen);

/* stat.c */
int fpr_stat(const char* path, struct fpr_stat* s);

/* thread.c */
void* fpr_thread_create(void (*entry)(void* param), void* param);
void  fpr_thread_detach(void* handle); /* this also frees the thread */
void  fpr_thread_join(void* handle);
void  fpr_thread_destroy(void* handle);

/* process.c */
void* fpr_process_create(const char* exec, char** env);
void  fpr_process_close(void* handle);
int   fpr_process_write(void* handle, const void* data, int len);
int   fpr_process_read(void* handle, void* data, int len);
void  fpr_process_destroy(void* handle);

#endif
