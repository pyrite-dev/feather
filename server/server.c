#include <fhttpd.h>

#include <stb_ds.h>

fr_server_context_t server_context;

#define LOAD(x) extern fr_module_t* x;
MODULES
#undef LOAD

fpr_bool server_init(void) {
	int i;
#define LOAD(x) x,
	struct fr_module* modules[] = {MODULES NULL};
#undef LOAD

	fpr_socket_init();

	for(i = 0; i < arrlen(config_ports); i++) {
		if(config_ports[i].fd == -1) {
			struct fpr_sockaddr_in addr;

			if((config_ports[i].fd = fpr_socket(FPR_PF_INET, FPR_SOCK_STREAM, FPR_IPPROTO_TCP)) < 0) {
				log_srv("Failed to create socket");
				return fpr_false;
			}

			addr.sin_family = FPR_AF_INET;
			addr.sin_addr	= fpr_inaddr_any;
			addr.sin_port	= fpr_htons(config_ports[i].port);

			if(fpr_bind(config_ports[i].fd, (struct fpr_sockaddr*)&addr, sizeof(addr)) < 0) {
				log_srv("Failed to bind socket");
				return fpr_false;
			}

			if(fpr_listen(config_ports[i].fd, 128) < 0) {
				log_srv("Failed to listen to socket");
				return fpr_false;
			}

			log_srv("Listening to port %d", config_ports[i].port);
		}
	}

	fr_server_init(&server_context);
	for(i = 0; modules[i] != NULL; i++) {
		fr_server_load_module(&server_context, modules[i]);
	}

	return fpr_true;
}

void server_close(void) {
	int i;

	fr_server_uninit(&server_context);
	for(i = 0; i < arrlen(config_ports); i++) {
		if(config_ports[i].fd != -1) {
			fpr_socket_close(config_ports[i].fd);
		}
	}
	arrfree(config_ports);

	fpr_socket_uninit();
}

typedef struct client {
	int key;
} client_t;

void server_loop(void) {
	int		   srv_count = 0;
	struct fpr_pollfd* pfd	     = NULL;

	while(1) {
		fpr_bool changed = fpr_false;

		if(pfd != NULL) {
			int s = fpr_poll(pfd, arrlen(pfd), 100);

			if(s > 0) {
				int i;

				/* server sockets */
				for(i = 0; i < srv_count; i++) {
				}

#if !defined(MULTITHREAD)
				/* client sockets */
				for(i = srv_count; i < arrlen(pfd); i++) {
				}
#endif
			}
		}

		if(srv_count != arrlen(config_ports)) {
			srv_count = arrlen(config_ports);
			changed	  = fpr_true;
		}

		if(changed) {
			int i;

			arrfree(pfd);
			for(i = 0; i < srv_count; i++) {
				struct fpr_pollfd fd;

				fd.fd	  = config_ports[i].fd;
				fd.events = FPR_POLLIN | FPR_POLLPRI;

				arrput(pfd, fd);
			}
		}
	}
	arrfree(pfd);
}
