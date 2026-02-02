#include <fhttpd.h>

char* serverroot = NULL;

void config_init(void) {
	if(serverroot != NULL) free(serverroot);
	serverroot = fpr_strdup(PREFIX);
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
						if(strcmp(arg[0], "ServerRoot") == 0) {
							if(arg_len(arg) == 2) {
								free(serverroot);
								serverroot = fpr_strdup(arg[1]);
							} else {
								fprintf(stderr, "%s: %s: ServerRoot takes 1 argument\n", argv0, path);

								fail = 1;
							}
						} else if(strcmp(arg[0], "ForceLog") == 0) {
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
