#include <fhttpd.h>

char* argv0;

static int    argc;
static char** argv;

static void main_stuff(void* args);

int main(int _argc, char** _argv) {
	argc = _argc;
	argv = _argv;

#if defined(PPR_IS_NETWARE)
	netware_start(main_stuff);
	return 0;
#else
	main_stuff(NULL);
	return 0;
#endif
}

static void main_stuff(void* args) {
	int	    i;
	const char* conf      = NULL;
	ppr_bool    daemonize = ppr_true;
#if defined(PPR_HAS_FORK)
	pid_t pid;
#endif
	int st;

	argv0 = argv[0];

	ppr_init();

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
			daemonize = ppr_false;
		} else {
			log_srv2("%s: %s -- unknown option", argv[0], argv[i]);
			EXIT(1);
		}
	}

	if((st = fhttpd_init(conf, daemonize)) != 0) {
		EXIT(st);
	}

#if defined(PPR_HAS_FORK)
	if(daemonize && (pid = fork()) != 0) {
		PPR_FILE* f = ppr_fopen(config_pidfile, "w");
		char	  buf[512];
		sprintf(buf, "%ld", (long)pid);

		ppr_fwrite(buf, 1, strlen(buf), f);
		ppr_fclose(f);

		EXIT(0);
	}
#endif

	fhttpd_loop();

	fhttpd_uninit();

	EXIT(0);
}
