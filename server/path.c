#include <fhttpd.h>

char* path_transform(const char* path) {
	char* p = ppr_strdup(path);
	char* r;
	int   i;

	for(i = 0; p[i] != 0; i++) {
		if(p[i] == '\\') p[i] = '/';
	}

	if(strlen(p) > 2 && (						 /**/
			     strstr(p, ":/") != NULL ||			 /**/
			     p[0] == '/' ||				 /**/
			     (p[0] == '.' && p[1] == '/') ||		 /**/
			     (p[0] == '.' && p[1] == '.' && p[2] == '/') /**/
			     )) {
		return p;
	}

	r = ppr_strvacat(config_serverroot, "/", p, NULL);
	free(p);

	for(i = 0; r[i] != 0; i++) {
		if(r[i] == '\\') r[i] = '/';
	}

	return r;
}
