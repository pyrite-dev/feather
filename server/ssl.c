#include <fhttpd.h>

#include <stb_ds.h>

#if defined(HAS_SSL)
static int sni_callback(SSL* s, int* al, void* arg) {
	const char*  str = SSL_get_servername(s, TLSEXT_NAMETYPE_host_name);
	char*	     host;
	size_t	     port = (size_t)arg;
	fr_config_t* c;
	int	     st = SSL_TLSEXT_ERR_ALERT_FATAL;
	const char*  tk = "SSLPrivateKeyFile";
	const char*  tc = "SSLCertificateFile";

	if(str == NULL) {
		char tmp[4096];

		fpr_gethostname(tmp, 4096);

		host = malloc(strlen(tmp) + 1);
		strcpy(host, tmp);
	} else {
		host = malloc(strlen(str) + 7);
		strcpy(host, str);
	}

	if((c = config_vhost_match(host, port)) == NULL) c = config_root;
	if(shgeti(c->kv, tk) == -1 || shgeti(c->kv, tc) == -1) c = config_root;

	if(shgeti(c->kv, tk) != -1 && shgeti(c->kv, tc) != -1) {
		char* ttk = path_transform(shget(c->kv, tk));
		char* ttc = path_transform(shget(c->kv, tc));

		SSL_use_PrivateKey_file(s, ttk, SSL_FILETYPE_PEM);
		SSL_use_certificate_file(s, ttc, SSL_FILETYPE_PEM);

		free(ttk);
		free(ttc);

		st = SSL_TLSEXT_ERR_OK;
	}

	free(host);

	return st;
}

SSL_CTX* ssl_create_context(int port) {
	SSL_CTX* ctx = SSL_CTX_new(TLS_server_method());
	SSL_CTX_set_tlsext_servername_callback(ctx, sni_callback);
	SSL_CTX_set_tlsext_servername_arg(ctx, (void*)(size_t)port);

	return ctx;
}
#endif
