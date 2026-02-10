#include <fhttpd.h>

#if defined(HAS_SSL)
static int sni_callback(SSL* s, int* al, void* arg) {
	return SSL_TLSEXT_ERR_OK;
}

SSL_CTX* ssl_create_context(int port) {
	SSL_CTX* ctx = SSL_CTX_new(TLS_server_method());
	SSL_CTX_set_tlsext_servername_callback(ctx, sni_callback);
	SSL_CTX_set_tlsext_servername_arg(ctx, (void*)(size_t)port);

	return ctx;
}
#endif
