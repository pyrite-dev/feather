#include <fhttpd.h>

char*	 argv0;
fpr_bool running = fpr_true;
char	 server[2048];

int main(int argc, char** argv) {
	int	    i;
	const char* conf      = PREFIX "/etc/fhttpd/fhttpd.conf";
	fpr_bool    daemonize = fpr_true;
#if defined(FPR_HAS_FORK)
	pid_t pid;
#endif

	argv0 = argv[0];

	for(i = 1; i < argc; i++) {
		if(strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "-h") == 0) {
			printf("Feather HTTPd %s\n", FR_SERVER);
			printf("Copyright (C) 2026       Pyrite development team\n");
			printf("\n");
			printf("Documentation online at http://moomoo.nishi.boats\n");
			printf("\n");
			printf("Compiled in Options:\n");
#if defined(HAS_SSL)
			printf("     HAS_SSL\n");
#endif
			printf("     PREFIX = %s\n", PREFIX);
			printf("\n");
			printf("Usage: %s [-C file] [-dVh]\n", argv[0]);
			printf("-C file     : Specify config file\n");
			printf("-d          : Do not daemonize\n");
			printf("-V -h       : Version/help information\n");
			EXIT(0);
		} else if(strcmp(argv[i], "-C") == 0) {
			if((conf = argv[++i]) == NULL) {
				log_srv2("%s: -C needs argument", argv[0]);
				EXIT(1);
			}
		} else if(strcmp(argv[i], "-d") == 0) {
			daemonize = fpr_false;
		} else {
			log_srv2("%s: %s -- unknown option", argv[0], argv[i]);
			EXIT(1);
		}
	}

	setlocale(LC_ALL, "");

#if defined(FPR_IS_PSP)
	psp_init();
#elif defined(FPR_IS_PS2)
	ps2_init();
#endif

#if !defined(_WIN32)
	signal(SIGPIPE, SIG_IGN);
#endif

	config_init();

	module_init();

	if(!config_parse(conf)) EXIT(1);

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

	if(!server_init()) EXIT(1);

#if defined(FPR_HAS_FORK)
	if(daemonize && (pid = fork()) != 0) {
		FPR_FILE* f = fpr_fopen(config_pidfile, "w");
		char	  buf[512];
		sprintf(buf, "%ld", (long)pid);

		fpr_fwrite(buf, 1, strlen(buf), f);
		fpr_fclose(f);

		EXIT(0);
	}
#endif

	log_srv("HTTPd is on the air");

	server_loop();

	server_close();
	config_close();
	log_close();

	EXIT(0);

	return 0;
}
