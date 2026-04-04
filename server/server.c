#include <fhttpd.h>

#include <stb_ds.h>

#define Workers 16

#if defined(MULTITHREAD)
static void* global_mutex;
worker_t*    server_workers = NULL;
int	     server_worker  = 0;

static void thread_main(void* param);
#else
clientkv_t* server_clients = NULL;
#endif

fpr_bool server_init(void) {
	int i;

#if defined(MULTITHREAD)
	global_mutex = fpr_mutex_create();

	fpr_mutex_lock(global_mutex);
	for(i = 0; i < Workers; i++) {
		worker_t worker;
		int*	 n = malloc(sizeof(*n));

		*n = i;

		worker.shutdown = fpr_false;
		worker.thread	= fpr_thread_create(thread_main, n);
		worker.mutex	= fpr_mutex_create();
		worker.clients	= NULL;

		arrput(server_workers, worker);
	}
	fpr_mutex_unlock(global_mutex);
#endif

	for(i = 0; i < arrlen(config_ports); i++) {
		if(config_ports[i].fd == -1) {
			struct fpr_sockaddr_in	addr;
			struct fpr_sockaddr_in6 addr6;
			struct fpr_sockaddr*	sa;
			int			sz;

			if((config_ports[i].fd = fpr_socket(config_ports[i].ipv6 ? FPR_PF_INET6 : FPR_PF_INET, FPR_SOCK_STREAM, FPR_IPPROTO_TCP)) < 0) {
				log_srv("Failed to create socket");
				return fpr_false;
			}

			if(config_ports[i].ipv6) {
				addr6.sin6_family = FPR_AF_INET6;
				addr6.sin6_addr	  = fpr_in6addr_any;
				addr6.sin6_port	  = fpr_htons(config_ports[i].port);
				sa		  = (struct fpr_sockaddr*)&addr6;
				sz		  = sizeof(addr6);
			} else {
				addr.sin_family = FPR_AF_INET;
				addr.sin_addr	= fpr_inaddr_any;
				addr.sin_port	= fpr_htons(config_ports[i].port);
				sa		= (struct fpr_sockaddr*)&addr;
				sz		= sizeof(addr);
			}

			if(fpr_bind(config_ports[i].fd, sa, sz) < 0) {
				log_srv("Failed to bind socket");
				return fpr_false;
			}

			if(fpr_listen(config_ports[i].fd, 128) < 0) {
				log_srv("Failed to listen to socket");
				return fpr_false;
			}

			log_srv("Listening to port %d%s%s", config_ports[i].port, config_ports[i].ssl ? " (SSL)" : "", config_ports[i].ipv6 ? " (IPv6)" : "");
		}
	}

	return fpr_true;
}

static void kill_client(client_t* c) {
	int i;

	http_end(c);

#if defined(HAS_SSL)
	if(c->ssl != NULL) {
		if(c->state > CS_WANT_SSL) {
			/* if it's larger than CS_WANT_SSL, client should be shut down using SSL_shutdown - probably */
			SSL_shutdown(c->ssl);
		}

		SSL_free(c->ssl);
	}

	if(c->ctx != NULL) {
		SSL_CTX_free(c->ctx);
	}
#endif

	fpr_socket_close(c->fd);

#if defined(MULTITHREAD)
	for(i = 0; i < arrlen(server_workers); i++) {
		int j;

		fpr_mutex_lock(server_workers[i].mutex);
		for(j = 0; j < arrlen(server_workers[i].clients); j++) {
			if(server_workers[i].clients[j] == c) {
				arrdel(server_workers[i].clients, j);
				break;
			}
		}

		if(j != arrlen(server_workers[i].clients)) {
			fpr_mutex_unlock(server_workers[i].mutex);
			break;
		}
		fpr_mutex_unlock(server_workers[i].mutex);
	}
#else
	hmdel(server_clients, c->fd);
#endif

	free(c);
}

void server_close(void) {
	int i;

	for(i = 0; i < arrlen(config_ports); i++) {
		if(config_ports[i].fd != -1) {
			fpr_socket_close(config_ports[i].fd);
			config_ports[i].fd = -1;
		}
	}

#if defined(MULTITHREAD)
	for(i = 0; i < Workers; i++) {
		int j;

		fpr_mutex_lock(server_workers[i].mutex);
		server_workers[i].shutdown = fpr_true;
		fpr_mutex_unlock(server_workers[i].mutex);

		fpr_thread_join(server_workers[i].thread);

		for(j = 0; j < arrlen(server_workers[i].clients); j++) {
			kill_client(server_workers[i].clients[j]);
		}
		arrfree(server_workers[i].clients);

		fpr_mutex_destroy(server_workers[i].mutex);
	}
	arrfree(server_workers);

	fpr_mutex_destroy(global_mutex);
#else
	for(i = 0; i < arrlen(server_clients); i++) {
		kill_client(server_clients[i].value);
	}
	arrfree(server_clients);
	server_clients = NULL;
#endif
}

int server_read(client_t* c, void* buffer, int len) {
#if defined(HAS_SSL)
	if(c->ssl != NULL) {
		return SSL_read(c->ssl, buffer, len);
	} else
#endif
	{
		return fpr_recv(c->fd, buffer, len, 0);
	}
}

int server_write(client_t* c, void* buffer, int len) {
#if defined(HAS_SSL)
	if(c->ssl != NULL) {
		return SSL_write(c->ssl, buffer, len);
	} else
#endif
	{
		return fpr_send(c->fd, buffer, len, 0);
	}
}

static int socket_recv(client_t* c, fpr_bool* changed) {
#if defined(HAS_SSL)
	/* probably handshake */
	if(c->state == CS_WANT_SSL) {
		if(SSL_accept(c->ssl) > 0) {
			c->state = CS_CONNECTED;
		} else {
			return 1;
		}
	} else
#endif
	{
		int	      len;
		unsigned char buf[BUFFER_SIZE];

		len = server_read(c, buf, BUFFER_SIZE);

		if(len <= 0) {
			return 1;
		} else {
			/* handle data */
			int st = c->state;
			int last;

			if(!http_got(c, buf, len, &last)) {
				c->state = CS_GOT_BODY;
				last	 = 0;

				strcpy(c->request.method, "GET");
				strcpy(c->request.path, "bad_request");
				strcpy(c->request.path_raw, c->request.path);
				strcpy(c->request.version, "HTTP/1.1");

				c->response.status_code = 400;
				strcpy(c->response.status_text, "Bad Request");

				http_req(c);
			}

			if(last > 0 && last < len) {
				memcpy(c->leftover, buf, len);
				c->leftover_seek = last;
				c->leftover_size = len;
			} else {
				c->leftover_seek = 0;
			}

			if(st != c->state && c->state == CS_GOT_BODY && changed != NULL) *changed = fpr_true;

			c->last = time(NULL);
		}
	}

	return 0;
}

static int socket_send(client_t* c, fpr_bool* changed) {
	int st = c->state;

	if(!http_send(c)) {
		return 1;
	}

	if(st != c->state && c->state == CS_CONNECTED) {
#if 1
		const char* t = http_req_get_header(&c->request, "connection");

		if(t != NULL && strcmp(t, "keep-alive") == 0) {
			if(changed != NULL) *changed = fpr_true;

			http_end(c);
			http_init(c);

			if(c->leftover_seek > 0) {
				int st = c->state;
				int last;

				http_got(c, &c->leftover[c->leftover_seek], c->leftover_size - c->leftover_seek, &last);
				if(last > 0 && last < (c->leftover_size - c->leftover_seek)) {
					c->leftover_seek += last;
				} else {
					c->leftover_seek = 0;
				}

				if(st != c->state && c->state == CS_GOT_BODY && changed != NULL) *changed = fpr_true;
			}
		} else {
			return 1;
		}
#else
		return 1;
#endif
	}

	return 0;
}

static int socket_main(client_t* c, fpr_bool* changed, struct fpr_pollfd* pfd) {
	if((pfd->revents & FPR_POLLIN) && socket_recv(c, changed)) {
		kill_client(c);
		return 1;
	}
	if((pfd->revents & FPR_POLLOUT) && socket_send(c, changed)) {
		kill_client(c);
		return 1;
	}

	return 0;
}

#if defined(MULTITHREAD)
static void thread_main(void* param) {
	int		   n	= *(int*)param;
	struct fpr_pollfd* pfds = NULL;
	struct fpr_pollfd  pfd;
	int		   i;

	free(param);

	fpr_mutex_lock(global_mutex);
	fpr_mutex_unlock(global_mutex);

	while(!server_workers[n].shutdown) {
		int s;

		arrfree(pfds);

		fpr_mutex_lock(server_workers[n].mutex);

		for(i = 0; i < arrlen(server_workers[n].clients); i++) {
			pfd.fd	   = server_workers[n].clients[i]->fd;
			pfd.events = FPR_POLLIN | FPR_POLLPRI;

			if(server_workers[n].clients[i]->state >= CS_GOT_BODY) pfd.events |= FPR_POLLOUT;

			pfd.user = server_workers[n].clients[i];

			arrput(pfds, pfd);
		}

		fpr_mutex_unlock(server_workers[n].mutex);

		if(arrlen(pfds) > 0) {
			s = fpr_poll(pfds, arrlen(pfds), 100);
		} else {
			fpr_msleep(10);
			s = 0;
		}

		if(s < 0) {
			arrfree(pfds);
			break;
		}

		for(i = 0; i < arrlen(pfds); i++) {
			client_t* c = pfds[i].user;

			if((time(NULL) - c->last) >= 10) {
				kill_client(c);
				continue;
			} else if(socket_main(c, NULL, &pfds[i])) {
				continue;
			}

			if(pfds[i].revents & (FPR_POLLIN | FPR_POLLOUT)) {
				c->last = time(NULL);
			}
		}
	}
}
#endif

void server_loop(void) {
	int srv_count = 0;
#if !defined(MULTITHREAD)
	int cli_count = 0;
#endif
	struct fpr_pollfd* pfd = NULL;

	while(running) {
		fpr_bool changed = fpr_false;

		if(pfd != NULL) {
			int s = fpr_poll(pfd, arrlen(pfd), 100);
			int i;

			if(s > 0) {
				/* server sockets */
				for(i = 0; i < srv_count; i++) {
					if(pfd[i].revents & FPR_POLLIN) {
						client_t* c;
						int	  l = sizeof(c->address);
						int	  fd;
#if defined(MULTITHREAD)
						int w;
#endif

						c = malloc(sizeof(*c));

						memset(c, 0, sizeof(*c));
						fd	 = fpr_accept(pfd[i].fd, (struct fpr_sockaddr*)&c->address, &l);
						c->last	 = time(NULL);
						c->state = config_ports[i].ssl ? CS_WANT_SSL : CS_CONNECTED;
						c->fd	 = fd;

						c->port = config_ports[i].port;

#if defined(HAS_SSL)
						if(config_ports[i].ssl) {
							c->ctx = ssl_create_context(config_ports[i].port);
							c->ssl = SSL_new(c->ctx);

							SSL_set_fd(c->ssl, fd);
						}
#endif

						http_init(c);

#if defined(MULTITHREAD)
						w = server_worker++;

						if(server_worker >= arrlen(server_workers)) server_worker = 0;

						fpr_mutex_lock(server_workers[w].mutex);
						arrput(server_workers[w].clients, c);
						fpr_mutex_unlock(server_workers[w].mutex);
#else
						hmput(server_clients, fd, c);
#endif
					}
				}

#if !defined(MULTITHREAD)
				/* client sockets */
				for(i = srv_count; i < arrlen(pfd); i++) {
					int ind = hmgeti(server_clients, pfd[i].fd);

					if(socket_main(server_clients[ind].value, &changed, &pfd[i])) {
						continue;
					}
				}
#endif
			}
#if !defined(MULTITHREAD)
			for(i = srv_count; i < arrlen(pfd); i++) {
				int ind = hmgeti(server_clients, pfd[i].fd);

				if(ind == -1) continue;

				if((time(NULL) - server_clients[ind].value->last) >= 10) {
					kill_client(server_clients[ind].value);
				}
			}
#endif
		}
		if(srv_count != arrlen(config_ports)) {
			srv_count = arrlen(config_ports);
			changed	  = fpr_true;
		}

#if !defined(MULTITHREAD)
		if(cli_count != hmlen(server_clients)) {
			cli_count = hmlen(server_clients);
			changed	  = fpr_true;
		}
#endif

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

				if(server_clients[i].value->state >= CS_GOT_BODY) fd.events |= FPR_POLLOUT;

				arrput(pfd, fd);
			}
#endif
		}
	}
	arrfree(pfd);
}
