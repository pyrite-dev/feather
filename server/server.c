#include <fhttpd.h>

#include <stb_ds.h>

fr_server_context_t server_context;

#if !defined(MULTITHREAD)
clientkv_t* server_clients = NULL;
#endif

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
			config_ports[i].fd = -1;
		}
	}

#if !defined(MULTITHREAD)
	for(i = 0; i < hmlen(server_clients); i++) {
		fpr_socket_close(config_ports[i].fd);
	}
	arrfree(server_clients);
	server_clients = NULL;
#endif

	fpr_socket_uninit();
}

void server_loop(void) {
	int		   srv_count = 0;
	int		   cli_count = 0;
	struct fpr_pollfd* pfd	     = NULL;

	while(1) {
		fpr_bool changed = fpr_false;

		if(pfd != NULL) {
			int s = fpr_poll(pfd, arrlen(pfd), 100);
			int i;

			if(s > 0) {
				/* server sockets */
				for(i = 0; i < srv_count; i++) {
					if(pfd[i].revents & FPR_POLLIN) {
						client_t c;
						int	 l = sizeof(c.address);
						int	 fd;

						memset(&c, 0, sizeof(c));
						fd = fpr_accept(pfd[i].fd, (struct fpr_sockaddr*)&c.address, &l);
						c.last = time(NULL);

						hmput(server_clients, fd, c);
					}
				}

#if !defined(MULTITHREAD)
				/* client sockets */
				for(i = srv_count; i < arrlen(pfd); i++) {
					if(pfd[i].revents & FPR_POLLIN) {
						unsigned char buf[BUFFER_SIZE];
						int	      len = fpr_recv(pfd[i].fd, buf, BUFFER_SIZE, 0);

						if(len <= 0){
							hmdel(server_clients, pfd[i].fd);
						}else{
							/* handle data */
							int ind = hmgeti(server_clients, pfd[i].fd);

							server_clients[ind].value.last = time(NULL);
						}
					}
				}
#endif
			}
#if !defined(MULTITHREAD)
			for(i = srv_count; i < arrlen(pfd); i++) {
				int ind = hmgeti(server_clients, pfd[i].fd);

				if((time(NULL) - server_clients[ind].value.last) >= 10){
					fpr_socket_close(pfd[i].fd);
					hmdel(server_clients, pfd[i].fd);
				}
			}
#endif
		}

		if(srv_count != arrlen(config_ports)) {
			srv_count = arrlen(config_ports);
			changed	  = fpr_true;
		}

		if(cli_count != hmlen(server_clients)) {
			cli_count = arrlen(server_clients);
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

#if !defined(MULTITHREAD)
			for(i = 0; i < hmlen(server_clients); i++) {
				struct fpr_pollfd fd;

				fd.fd	  = server_clients[i].key;
				fd.events = FPR_POLLIN | FPR_POLLPRI;

				arrput(pfd, fd);
			}
#endif
		}
	}
	arrfree(pfd);
}
