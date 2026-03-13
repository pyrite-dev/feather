#include <fhttpd.h>

#include <stb_ds.h>

#if !defined(MULTITHREAD)
clientkv_t* server_clients = NULL;
#endif

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

			log_srv("Listening to port %d%s", config_ports[i].port, config_ports[i].ssl ? " (SSL)" : "");
		}
	}

	return fpr_true;
}

void server_close(void) {
	int i;

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

void kill_client(int fd) {
	int ind = hmgeti(server_clients, fd);
	int j;

	http_end(&server_clients[ind].value);

#if defined(HAS_SSL)
	if(server_clients[ind].value.ssl != NULL) {
		if(server_clients[ind].value.state > CS_WANT_SSL) {
			/* if it's larger than CS_WANT_SSL, client should be shut down using SSL_shutdown - probably */
			SSL_shutdown(server_clients[ind].value.ssl);
		}

		SSL_free(server_clients[ind].value.ssl);
	}

	if(server_clients[ind].value.ctx != NULL) {
		SSL_CTX_free(server_clients[ind].value.ctx);
	}
#endif

	for(j = 0; j < shlen(server_clients[ind].value.request.headers); j++) {
		if(server_clients[ind].value.request.headers[j].value != NULL) free(server_clients[ind].value.request.headers[j].value);
	}
	shfree(server_clients[ind].value.request.headers);

	fpr_socket_close(fd);
	hmdel(server_clients, fd);
}

int server_read(int fd, void* buffer, int len) {
#if defined(HAS_SSL)
	int ind = hmgeti(server_clients, fd);

	if(server_clients[ind].value.ssl != NULL) {
		return SSL_read(server_clients[ind].value.ssl, buffer, len);
	} else
#endif
	{
		return fpr_recv(fd, buffer, len, 0);
	}
}

int server_write(int fd, void* buffer, int len) {
#if defined(HAS_SSL)
	int ind = hmgeti(server_clients, fd);

	if(server_clients[ind].value.ssl != NULL) {
		return SSL_write(server_clients[ind].value.ssl, buffer, len);
	} else
#endif
	{
		return fpr_send(fd, buffer, len, 0);
	}
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
						int	 j;

						memset(&c, 0, sizeof(c));
						fd	= fpr_accept(pfd[i].fd, (struct fpr_sockaddr*)&c.address, &l);
						c.last	= time(NULL);
						c.state = config_ports[i].ssl ? CS_WANT_SSL : CS_CONNECTED;
						c.fd	= fd;

						c.port = config_ports[i].port;

#if defined(HAS_SSL)
						if(config_ports[i].ssl) {
							c.ctx = ssl_create_context(config_ports[i].port);
							c.ssl = SSL_new(c.ctx);

							SSL_set_fd(c.ssl, fd);
						}
#endif

						http_init(&c);

						hmput(server_clients, fd, c);
					}
				}

#if !defined(MULTITHREAD)
				/* client sockets */
				for(i = srv_count; i < arrlen(pfd); i++) {
					if(pfd[i].revents & FPR_POLLIN) {
						int ind = hmgeti(server_clients, pfd[i].fd);

#if defined(HAS_SSL)
						/* probably handshake */
						if(server_clients[ind].value.state == CS_WANT_SSL) {
							if(SSL_accept(server_clients[ind].value.ssl) > 0) {
								server_clients[ind].value.state = CS_CONNECTED;
							} else {
								kill_client(pfd[i].fd);
							}
						} else
#endif
						{
							int	      len;
							unsigned char buf[BUFFER_SIZE];

							len = server_read(pfd[i].fd, buf, BUFFER_SIZE);

							if(len <= 0) {
								kill_client(pfd[i].fd);
							} else {
								/* handle data */
								int ind = hmgeti(server_clients, pfd[i].fd);
								int st	= server_clients[ind].value.state;
								int last;

								http_got(&server_clients[ind].value, buf, len, &last);
								if(last > 0 && last < len) {
									memcpy(server_clients[ind].value.leftover, buf, len);
									server_clients[ind].value.leftover_seek = last;
									server_clients[ind].value.leftover_size = len;
								} else {
									server_clients[ind].value.leftover_seek = 0;
									fwrite(buf, 1, len, stdout);
								}

								if(st != server_clients[ind].value.state && server_clients[ind].value.state == CS_GOT_BODY) changed = fpr_true;

								server_clients[ind].value.last = time(NULL);
							}
						}
					}
				}
#endif
			}
#if !defined(MULTITHREAD)
			for(i = srv_count; i < arrlen(pfd); i++) {
				int ind = hmgeti(server_clients, pfd[i].fd);
				int st	= server_clients[ind].value.state;

				if(pfd[i].revents & FPR_POLLOUT) {
					http_send(&server_clients[ind].value);

					if(st != server_clients[ind].value.state && server_clients[ind].value.state == CS_CONNECTED) {
#if 1
						const char* t = http_req_get_header(&server_clients[ind].value.request, "connection");

						if(t != NULL && strcmp(t, "keep-alive") == 0) {
							changed = fpr_true;

							http_end(&server_clients[ind].value);
							http_init(&server_clients[ind].value);

							if(server_clients[ind].value.leftover_seek > 0) {
								int st = server_clients[ind].value.state;
								int last;

								http_got(&server_clients[ind].value, &server_clients[ind].value.leftover[server_clients[ind].value.leftover_seek], server_clients[ind].value.leftover_size - server_clients[ind].value.leftover_seek, &last);
								if(last > 0 && last < (server_clients[ind].value.leftover_size - server_clients[ind].value.leftover_seek)) {
									server_clients[ind].value.leftover_seek += last;
								} else {
									server_clients[ind].value.leftover_seek = 0;
								}

								if(st != server_clients[ind].value.state && server_clients[ind].value.state == CS_GOT_BODY) changed = fpr_true;
							}
						} else {
							kill_client(pfd[i].fd);
							continue;
						}
#else
						kill_client(pfd[i].fd);
						continue;
#endif
					}

					server_clients[ind].value.last = time(NULL);
				}

				if((time(NULL) - server_clients[ind].value.last) >= 10) {
					kill_client(pfd[i].fd);
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

				if(server_clients[i].value.state >= CS_GOT_BODY) fd.events |= FPR_POLLOUT;

				arrput(pfd, fd);
			}
#endif
		}
	}
	arrfree(pfd);
}
