#include <fhttpd.h>

char* serverroot = NULL;

void config_init(void) {
	if(serverroot != NULL) free(serverroot);
	serverroot = fpr_strdup(PREFIX);
}

static fpr_bool parse(const char* path) {
	FPR_FILE* fp;
	char	  buffer[BUFFER_SIZE];
	int	  s;
	char*	  p;

	p = path_transform(path);
	if((fp = fpr_fopen(p, "r")) == NULL) {
		free(p);
		fprintf(stderr, "%s: %s does not exist\n", argv0, path);
		return fpr_false;
	}
	free(p);

	while((s = fpr_fread(buffer, 1, BUFFER_SIZE, fp)) > 0) {
		int i;

		for(i = 0; i < s; i++) {
			if(buffer[i] == '\n') {
			}
		}
	}

	fpr_fclose(fp);

	return fpr_true;
}

fpr_bool config_parse(const char* path) {
	return parse(path);
}
