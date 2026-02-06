#include <fhttpd.h>

#include <stb_ds.h>

char*	  config_serverroot = NULL;
char*	  config_pidfile    = NULL;
char*	  config_logfile    = NULL;
config_t* config_root	    = NULL;
port_t*	  config_ports	    = NULL;

static config_t* config_current = NULL;

static void recursive_free(config_t* config) {
	if(config != NULL) {
		int i;
		for(i = 0; i < arrlen(config->children); i++) {
			recursive_free(config->children[i]);
		}
		arrfree(config->children);

		if(config->name != NULL) free(config->name);
		if(config->docroot != NULL) free(config->docroot);

		free(config);
	}
}

static config_t* new_config(config_t* parent, const char* name) {
	config_t* config = malloc(sizeof(*config));
	memset(config, 0, sizeof(*config));

	config->parent = parent;
	if(name != NULL) config->name = fpr_strdup(name);

	if(parent != NULL) arrput(parent->children, config);

	return config;
}

#define ALLOC_PROP(var, val) \
	if(var != NULL) free(var); \
	var = fpr_strdup(val);
void config_init(void) {
	ALLOC_PROP(config_serverroot, PREFIX);
	ALLOC_PROP(config_pidfile, "/var/run/fhttpd.pid");
	ALLOC_PROP(config_logfile, "/var/log/fhttpd.log");

	recursive_free(config_root);
	config_root    = new_config(NULL, NULL);
	config_current = config_root;

	if(config_ports != NULL) arrfree(config_ports);
	config_ports = NULL;
}
#undef ALLOC_PROP

#define FREE_PROP(var) \
	if(var != NULL) { \
		free(var); \
		var = NULL; \
	}
void config_close(void) {
	FREE_PROP(config_serverroot);
	FREE_PROP(config_pidfile);
	FREE_PROP(config_logfile);

	recursive_free(config_root);

	if(config_ports != NULL) arrfree(config_ports);
	config_ports = NULL;
}
#undef FREE_PROP

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

#define ELSEIF_PROP(arg, direc, var) else IF_PROP(arg, direc, var)

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
						IF_PROP(arg, "ServerRoot", config_serverroot) /**/
						ELSEIF_PROP(arg, "PIDFile", config_pidfile)   /**/
						ELSEIF_PROP(arg, "LogFile", config_logfile)   /**/

						if(strcmp(arg[0], "ForceLog") == 0) {
							if(arg_len(arg) == 2) {
								fprintf(stderr, "%s: %s: %s", argv0, path, arg[1]);
							} else {
								fprintf(stderr, "%s: %s: ForceLog takes 1 argument\n", argv0, path);

								fail = 1;
							}
						} else if(strcmp(arg[0], "Listen") == 0 || strcmp(arg[0], "ListenSSL") == 0) {
							if(arg_len(arg) == 2) {
								port_t p;
								p.port = atoi(arg[1]);
								p.ssl  = strcmp(arg[0], "ListenSSL") == 0;
								p.fd   = -1;

#if !defined(HAS_SSL)
								if(p.ssl) {
									fprintf(stderr, "%s: %s: HTTPd is missing SSL/TLS support\n", argv0, path);

									fail = 1;
								} else
#endif
								{
									arrput(config_ports, p);
								}
							} else {
								fprintf(stderr, "%s: %s: %s takes 1 argument\n", argv0, path, arg[0]);

								fail = 1;
							}
						}
					} else {
						if(arg[0][0] == '/') {
							if(config_current->name != NULL && strcmp(config_current->name, arg[0] + 1) == 0) {
								config_current = config_current->parent;
								if(config_current == NULL) {
									fprintf(stderr, "%s: %s: Closing tag does not match\n", argv0, path);

									fail = 1;
								}
							} else {
								fprintf(stderr, "%s: %s: Closing tag does not match\n", argv0, path);

								fail = 1;
							}
						} else if(strcmp(arg[0], "VirtualHost") == 0) {
							if(arg_len(arg) > 1) {
								config_current = new_config(config_current, arg[0]);

								for(j = 1; j < arg_len(arg); j++) {
								}
							} else {
								fprintf(stderr, "%s: %s: VirtualHost takes 1 argument or more\n", argv0, path);

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
