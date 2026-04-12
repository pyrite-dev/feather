#include <fhttpd.h>

ppr_bool running = ppr_true;
char	 server[2048];

int fhttpd_init(const char* config, ppr_bool daemonize) {
	if(config == NULL) config = PREFIX "/etc/fhttpd/fhttpd.conf";

	setlocale(LC_ALL, "");

#if defined(PPR_IS_PSP)
	psp_init();
#elif defined(PPR_IS_PS2)
	ps2_init();
#elif defined(PPR_IS_NETWARE)
	netware_init();
#endif

	config_init();

	module_init();

	if(!config_parse(config)) EXIT(1);

	log_init();
	if(!daemonize) log_nofile();

	log_srv("This is Feather HTTPd%s, version %s%s",
#if defined(MULTITHREAD)
		" (multithread)",
#else
		"",
#endif
		FR_VERSION,
#if defined(HAS_SSL)
		" (with " OPENSSL_VERSION_TEXT ")"
#else
		""
#endif
	);

	if(!server_init()) return 1;

	return 0;
}

void fhttpd_loop(void) {
	log_srv("HTTPd is on the air: %s", server);

	server_loop();
}

void fhttpd_uninit(void) {
	fhttpd_loop();

	server_close();
	config_close();
	log_close();
}
