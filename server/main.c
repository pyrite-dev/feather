#include <fhttpd.h>

int main(int argc, char** argv) {
	int	    i;
	const char* conf = PREFIX "/etc/feather.conf";

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
		}
	}
}
