#include <fhttpd.h>

#include <stb_ds.h>

fpr_bool server_init(void) {
	int i;

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

	return fpr_true;
}

void server_close(void) {
	int i;

	for(i = 0; i < arrlen(config_ports); i++) {
		if(config_ports[i].fd != -1) {
			fpr_socket_close(config_ports[i].fd);
		}
	}
}
