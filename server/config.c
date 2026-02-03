#include <fhttpd.h>

char* config_serverroot = NULL;
char* config_pidfile	= NULL;
char* config_logfile	= NULL;

#define ALLOC_PROP(var, val) \
	if(var != NULL) free(var); \
	var = fpr_strdup(val);
void config_init(void) {
	ALLOC_PROP(config_serverroot, PREFIX);
	ALLOC_PROP(config_pidfile, "/var/run/fhttpd.pid");
	ALLOC_PROP(config_logfile, "/var/log/fhttpd.log");
}
#undef ALLOC_PROP

#define IF_PROP(arg, direc, var) \
	if(strcmp(arg[0], direc) == 0) { \
		if(arg_len(arg) == 2) { \
			free(var); \
			var = fpr_strdup(arg[1]); \
		} else { \
			fprintf(stderr, "%s: %s: %s takes 1 argument\n", argv0, path, direc); \
\
			fail = 1; \
		} \
	}
static fpr_bool parse(const char* path) {
	FPR_FILE* fp;
	char	  buffer[BUFFER_SIZE];
	char	  linebuf[LINE_SIZE + 1];
	int	  s;
	char*	  p;

	p = path_transform(path);
	if((fp = fpr_fopen(p, "r")) == NULL) {
		free(p);
		fprintf(stderr, "%s: %s does not exist\n", argv0, path);
		return fpr_false;
	}
	free(p);

	linebuf[0] = 0;

	while((s = fpr_fread(buffer, 1, BUFFER_SIZE, fp)) > 0) {
		int i;

		for(i = 0; i < s; i++) {
			if(buffer[i] == '\n') {
				char* line = linebuf;
				int   j;
				int   fail = 0;

				while((*line) != 0 && ((*line) == '\t' || (*line) == ' ')) line++;

				for(j = strlen(line) - 1; j >= 0; j--) {
					if(line[j] == '\t' || line[j] == ' ') {
						line[j] = 0;
					} else {
						break;
					}
				}

				if(strlen(line) > 0 && line[0] != '#') {
					char** arg;
					int    dir = 1;
					if(line[0] == '<' && line[strlen(line) - 1] == '>') {
						dir		       = 0;
						line[strlen(line) - 1] = 0;

						arg = arg_parse(line + 1);
					} else {
						arg = arg_parse(line);
					}
					if(dir) {
						IF_PROP(arg, "ServerRoot", config_serverroot)
						else IF_PROP(arg, "PIDFile", config_pidfile) else IF_PROP(arg, "LogFile", config_logfile) else if(strcmp(arg[0], "ForceLog") == 0) {
							if(arg_len(arg) == 2) {
								fprintf(stderr, "%s: %s: %s", argv0, path, arg[1]);
							} else {
								fprintf(stderr, "%s: %s: ForceLog takes 1 argument\n", argv0, path);

								fail = 1;
							}
						}
					}
					arg_free(arg);
				}

				linebuf[0] = 0;

				if(fail) goto cleanup;
			} else if(buffer[i] != '\r') {
				if(strlen(linebuf) == LINE_SIZE) {
					fprintf(stderr, "%s: %s: line too long; sorry\n", argv0, path);
					goto cleanup;
				}

				linebuf[strlen(linebuf) + 1] = 0;
				linebuf[strlen(linebuf)]     = buffer[i];
			}
		}
	}

cleanup:;
	fpr_fclose(fp);

	return fpr_true;
}

fpr_bool config_parse(const char* path) {
	return parse(path);
}
