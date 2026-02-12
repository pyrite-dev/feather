#include <fhttpd.h>

#include <stb_ds.h>

char*	     config_serverroot = NULL;
char*	     config_pidfile    = NULL;
char*	     config_logfile    = NULL;
char*	     config_mimefile   = NULL;
fr_config_t* config_root       = NULL;
fr_config_t* config_current    = NULL;
port_t*	     config_ports      = NULL;

static void recursive_free(fr_config_t* config) {
	if(config != NULL) {
		int i;
		for(i = 0; i < arrlen(config->children); i++) {
			recursive_free(config->children[i]);
		}
		arrfree(config->children);

		for(i = 0; i < shlen(config->kv); i++) {
			free(config->kv[i].value);
		}
		shfree(config->kv);

		if(config->name != NULL) {
			if(strcmp(config->name, "VirtualHost") == 0) {
				for(i = 0; i < arrlen(config->section.vhost.entry); i++) free(config->section.vhost.entry[i]);
				arrfree(config->section.vhost.entry);
			}

			free(config->name);
		}

		free(config);
	}
}

static fr_config_t* new_config(fr_config_t* parent, const char* name) {
	fr_config_t* config = malloc(sizeof(*config));
	memset(config, 0, sizeof(*config));

	sh_new_strdup(config->kv);
	shdefault(config->kv, NULL);

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
	ALLOC_PROP(config_mimefile, PREFIX "/etc/mime.types");

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
\
			handled = fpr_true; \
		} else { \
			fprintf(stderr, "%s: %s: %s takes 1 argument\n", argv0, path, direc); \
\
			fail = fpr_true; \
		} \
	}

#define ELSEIF_PROP(arg, direc, var) else IF_PROP(arg, direc, var)

#define IF_KV(arg, direc) \
	if(strcmp(arg[0], direc) == 0) { \
		if(arg_len(arg) == 2) { \
			char* v = fpr_strdup(arg[1]); \
			if(shgeti(config_current->kv, arg[0]) != -1) { \
				free(shget(config_current->kv, arg[0])); \
			} \
			shput(config_current->kv, arg[0], v); \
\
			handled = fpr_true; \
		} else { \
			fprintf(stderr, "%s: %s: %s takes 1 argument\n", argv0, path, direc); \
\
			fail = fpr_true; \
		} \
	}

#define ELSEIF_KV(arg, direc) else IF_KV(arg, direc)

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
				char*	 line = linebuf;
				int	 j;
				fpr_bool fail	 = fpr_false;
				fpr_bool handled = fpr_false; /* this exists only for these IF macros */

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
						ELSEIF_PROP(arg, "MIMEFile", config_mimefile) /**/

						IF_KV(arg, "DocumentRoot")	     /**/
						ELSEIF_KV(arg, "SSLPrivateKeyFile")  /**/
						ELSEIF_KV(arg, "SSLCertificateFile") /**/
						ELSEIF_KV(arg, "DirectoryIndex")     /**/

						if(strcmp(arg[0], "ForceLog") == 0) {
							if(arg_len(arg) == 2) {
								fprintf(stderr, "%s: %s: %s", argv0, path, arg[1]);
							} else {
								fprintf(stderr, "%s: %s: ForceLog takes 1 argument\n", argv0, path);

								fail = fpr_true;
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

									fail = fpr_true;
								} else
#endif
								{
									arrput(config_ports, p);
								}
							} else {
								fprintf(stderr, "%s: %s: %s takes 1 argument\n", argv0, path, arg[0]);

								fail = 1;
							}
						} else if(!handled) {
							fr_context_t context;
							fpr_bool     x = fpr_false;

							for(j = 0; j < arrlen(module_modules); j++) {
								int j;

								context_init(&context);
								SAFECALL_RET(x, module_modules[j]->directive)(&context, arg_len(arg), arg);
								context_save(&context);

								if(x) break;
							}
						}
					} else {
						if(arg[0][0] == '/') {
							if(config_current->name != NULL && strcmp(config_current->name, arg[0] + 1) == 0) {
								config_current = config_current->parent;
								if(config_current == NULL) {
									fprintf(stderr, "%s: %s: Closing tag does not match\n", argv0, path);

									fail = fpr_true;
								}
							} else {
								fprintf(stderr, "%s: %s: Closing tag does not match\n", argv0, path);

								fail = fpr_true;
							}
						} else if(strcmp(arg[0], "VirtualHost") == 0) {
							if(arg_len(arg) > 1) {
								config_current = new_config(config_current, arg[0]);

								config_current->section.vhost.entry = NULL;
								for(j = 1; j < arg_len(arg); j++) {
									char* s = fpr_strdup(arg[j]);

									arrput(config_current->section.vhost.entry, s);
								}
							} else {
								fprintf(stderr, "%s: %s: VirtualHost takes 1 argument or more\n", argv0, path);

								fail = fpr_true;
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
	int st = parse(path);

	mime_parse();

	return st;
}

fr_config_t* config_vhost_match(const char* host, int port) {
	char* n = malloc(strlen(host) + 7);
	int   i;

	sprintf(n, "%s:%d", host, port);

	for(i = 0; i < arrlen(config_root->children); i++) {
		fr_config_t* c = config_root->children[i];
		if(strcmp(c->name, "VirtualHost") == 0) {
			int j;

			for(j = 0; j < arrlen(c->section.vhost.entry); j++) {
				if(fpr_wildcard(c->section.vhost.entry[j], n) || fpr_wildcard(c->section.vhost.entry[j], host)) {
					free(n);

					return c;
				}
			}
		}
	}

	free(n);

	return NULL;
}
