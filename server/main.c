#include <fhttpd.h>

char* argv0;

int main(int argc, char** argv) {
	int	    i;
	const char* conf      = PREFIX "/etc/fhttpd.conf";
	fpr_bool    daemonize = fpr_true;
#ifdef HAS_FORK
	pid_t pid;
#endif

	argv0 = argv[0];

	for(i = 1; i < argc; i++) {
		if(strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "-h") == 0) {
			printf("Feather HTTPd %s\n", SERVER);
			printf("Copyright (C) 2026       Pyrite development team\n");
			printf("\n");
			printf("Documentation online at http://nishi.boats/feather\n");
			printf("\n");
			printf("Compiled in Options:\n");
#ifdef HAS_SSL
			printf("     HAS_SSL\n");
#endif
			printf("     PREFIX = %s\n", PREFIX);
			printf("\n");
			printf("Usage: %s [-C file] [-dVh]\n", argv[0]);
			printf("-C file     : Specify config file\n");
			printf("-d          : Do not daemonize\n");
			printf("-V -h       : Version/help information\n");
			return 0;
		} else if(strcmp(argv[i], "-C") == 0) {
			if((conf = argv[i + 1]) == NULL) {
				fprintf(stderr, "%s: -C needs argument\n", argv[0]);
				return 1;
			}
		} else if(strcmp(argv[i], "-d") == 0) {
			daemonize = fpr_false;
		} else {
			fprintf(stderr, "%s: %s -- unknown option\n", argv[0], argv[i]);
			return 1;
		}
	}

	config_init();

	if(!config_parse(conf)) {
		return 1;
	}

	log_init();
	if(!daemonize) log_nofile();

	log_srv("This is Feather HTTPd, version %s", VERSION);

	if(!server_init()) {
		return 1;
	}

#ifdef HAS_FORK
	if(daemonize && (pid = fork()) != 0) {
		FPR_FILE* f = fpr_fopen(config_pidfile, "w");
		char	  buf[512];
		sprintf(buf, "%ld", (long)pid);

		fpr_fwrite(buf, 1, strlen(buf), f);
		fpr_fclose(f);

		return 0;
	}
#endif

	log_srv("HTTPd is on the air");

	server_close();
	config_close();
	log_close();
}
