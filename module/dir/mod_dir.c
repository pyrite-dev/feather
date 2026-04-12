#define _FHTTPD
#include <fhttpd.h>

#include <string.h>
#include <stdlib.h>

static int hook_rewrite(fr_context_t* context, fr_request_t* req, fr_response_t* res) {
	char**		arr = NULL;
	int		len;
	struct ppr_stat st;
	int		i;

	(void)res;

	if(req->path[0] != '/') return FR_MODULE_DECLINE;

	if(ppr_stat(req->path_translated, &st) == 0 && PPR_S_ISDIR(st.st_mode) && req->path[strlen(req->path) - 1] != '/') {
		char* p = ppr_strvacat(req->path, "/", NULL);

		res->status_code = 301;
		strcpy(res->status_text, "Moved Permanently");

		context->response_set_header(res, "Location", p);

		free(p);

		return FR_MODULE_DECLINE;
	}

	arr = context->config_lookup_array(context, "DirectoryIndex", &len);

	for(i = 0; i < len; i++) {
		char* p = ppr_strvacat(req->path_translated, req->path_translated[strlen(req->path_translated) - 1] == '/' ? "" : "/", arr[i], NULL);

		if(ppr_stat(p, &st) == 0 && !PPR_S_ISDIR(st.st_mode) && (strlen(req->path_virtual) + strlen(arr[i])) <= MAX_PATH_LENGTH) {
			strcpy(req->path_virtual, req->path);
			strcat(req->path_virtual, arr[i]);

			strcpy(req->path_virtual2, req->path_virtual);

			free(p);
			return FR_MODULE_LOOP;
		}

		free(p);
	}

	return FR_MODULE_DECLINE;
}

static int sort_dir(const struct ppr_dirent** d1, const struct ppr_dirent** d2) {
	if(PPR_S_ISDIR((*d1)->d_stat.st_mode) && !PPR_S_ISDIR((*d2)->d_stat.st_mode)) return -1;
	if(!PPR_S_ISDIR((*d1)->d_stat.st_mode) && PPR_S_ISDIR((*d2)->d_stat.st_mode)) return 1;

	return ppr_alphasort(d1, d2);
}

static int hook(fr_context_t* context, fr_request_t* req, fr_response_t* res) {
	struct ppr_stat st;

	if(ppr_stat(req->path_translated, &st) == 0 && PPR_S_ISDIR(st.st_mode) && req->path[strlen(req->path) - 1] != '/') return FR_MODULE_DECLINE;

	if(ppr_stat(req->path_translated, &st) == 0 && PPR_S_ISDIR(st.st_mode)) {
		char*		    table = malloc(1);
		struct ppr_dirent** namelist;
		int		    n;

		table[0] = 0;

		ppr_strappend(&table, "		<tr>\n");
		ppr_strappend(&table, "			<th width=\"24\"></th>\n");
		ppr_strappend(&table, "			<th>Name</th>\n");
		ppr_strappend(&table, "			<th>Last modified</th>\n");
		ppr_strappend(&table, "			<th>Size</th>\n");
		ppr_strappend(&table, "		</tr>\n");

		if((n = ppr_scandir(req->path_translated, &namelist, NULL, sort_dir)) >= 0) {
			int i;
			for(i = 0; i < n; i++) {
				if(strcmp(namelist[i]->d_name, ".") != 0) {
					char*	      name;
					char*	      s;
					char	      date[64];
					char	      size[16];
					struct ppr_tm tm;
					ppr_size_t    sz = namelist[i]->d_stat.st_size;
					char*	      tmp;
					char*	      img = NULL;
					char*	      img_esc;
					char*	      ext = strrchr(namelist[i]->d_name, '.');

					tmp = malloc(16 + strlen(namelist[i]->d_name) + 1);

					sprintf(tmp, "Icon_%s", namelist[i]->d_name);
					if(img == NULL) img = context->config_lookup(context, tmp);

					if(ext != NULL) {
						sprintf(tmp, "Icon_%s", ext);
						if(img == NULL) img = context->config_lookup(context, tmp);
					}

					if(PPR_S_ISDIR(namelist[i]->d_stat.st_mode)) {
						if(img == NULL) img = context->config_lookup(context, "Icon_^^DIRECTORY^^");
					}

					if(img == NULL) {
						int    len;
						char** arr = context->config_lookup_array(context, "IconMatch", &len);
						int    j;

						if(arr != NULL) {
							for(j = 0; j < len; j += 2) {
								if(ppr_wildcard(arr[j + 0], namelist[i]->d_name)) {
									img = arr[j + 1];
									break;
								}
							}
						}
					}

					if(img == NULL && ext != NULL) {
						int    len;
						char** arr = context->config_lookup_array(context, "IconType", &len);
						int    j;
						char*  mime = context->stringkv_lookup(context->mime_types, ext + 1);

						if(arr != NULL && mime != NULL) {
							for(j = 0; j < len; j += 2) {
								if(ppr_wildcard(arr[j + 0], mime)) {
									img = arr[j + 1];
									break;
								}
							}
						}
					}

					if(img != NULL) img_esc = ppr_strsafehtml(img);

					name = ppr_strvacat(namelist[i]->d_name, PPR_S_ISDIR(namelist[i]->d_stat.st_mode) ? "/" : "", NULL);
					s    = ppr_strsafehtml(name);
					free(name);

					ppr_gmtime(&tm, namelist[i]->d_stat.st_modtime);

					date[0] = 0;
					sprintf(date, "%d-%02d-%02d %02d:%02d UTC", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min);

					size[0] = 0;
					if(PPR_S_ISDIR(namelist[i]->d_stat.st_mode)) {
					} else if(sz < 1024) {
						sprintf(size, "%d", (int)sz);
					} else if(sz < 1024 * 1024) {
						if((sz / 1024) < 10) {
							sprintf(size, "%.1fK", (double)sz / 1024);
						} else {
							sprintf(size, "%dK", (int)sz / 1024);
						}
					} else if(sz < 1024 * 1024 * 1024) {
						if((sz / 1024 / 1024) < 10) {
							sprintf(size, "%.1fM", (double)sz / 1024 / 1024);
						} else {
							sprintf(size, "%dM", (int)sz / 1024 / 1024);
						}
					} else {
						if((sz / 1024 / 1024 / 1024) < 10) {
							sprintf(size, "%.1fG", (double)sz / 1024 / 1024 / 1024);
						} else {
							sprintf(size, "%dG", (int)sz / 1024 / 1024 / 1024);
						}
					}

					ppr_strappend(&table, "		<tr>\n");
					ppr_strappend(&table, "			<td width=\"24\">\n");
					if(img != NULL) {
						ppr_strappend(&table, "				<img src=\"");
						ppr_strappend(&table, img_esc);
						ppr_strappend(&table, "\">\n");
					}
					ppr_strappend(&table, "			</td>\n");
					ppr_strappend(&table, "			<td>\n");
					ppr_strappend(&table, "				<a href=\"");
					ppr_strappend(&table, s);
					ppr_strappend(&table, "\">");
					ppr_strappend(&table, strcmp(namelist[i]->d_name, "..") == 0 ? "Parent Directory" : s);
					ppr_strappend(&table, "</a>\n");
					ppr_strappend(&table, "			</td>\n");
					ppr_strappend(&table, "			<td>");
					ppr_strappend(&table, date);
					ppr_strappend(&table, "</td>\n");
					ppr_strappend(&table, "			<td>");
					ppr_strappend(&table, size);
					ppr_strappend(&table, "</td>\n");
					ppr_strappend(&table, "		</tr>\n");

					free(s);
					if(img != NULL) free(img_esc);

					free(tmp);
				}
				free(namelist[i]);
			}
			free(namelist);
		}

		res->status_code = 200;
		strcpy(res->status_text, "OK");

		context->response_set_header(res, "Content-Type", "text/html");

		/* clang-format off */
		res->body = ppr_strvacat(
			"<html>\n"									/**/
			"	<head>\n",								/**/
			"		<title>Index of ", req->path, "</title>\n" 			/**/
			"	</head>\n"								/**/
			"	<body>\n"								/**/
			"		<h1>Index of ", req->path, "</h1>\n"				/**/
			"		<hr>\n"								/**/
			"		<table border=\"0\">\n", /**/
			table, /**/
			"		</table>\n" /**/
			"		<hr>\n"							/**/
			"		<i>", context->response_get_header(res, "Server"), "</i>\n",	/**/
			"	</body>\n"								/**/
			"</html>\n",									/**/
			NULL										/**/
		);
		/* clang-format on */
		res->body_size = strlen(res->body);

		free(table);

		context->response_set_header(res, "Content-Type", "text/html");

		return FR_MODULE_OK;
	}

	return FR_MODULE_DECLINE;
}

static int directive(fr_context_t* context, int argc, char** argv) {
	if(strcmp(argv[0], "DirectoryIndex") == 0) {
		if(argc >= 2) {
			int i;

			for(i = 1; i < argc; i++) {
				context->stringarraykv_push(context->config_current->arraykv, "DirectoryIndex", argv[i]);
			}
		} else {
			fprintf(stderr, "%s: %s: DirectoryIndex takes 1 argument or more\n", context->argv0, context->config_path);

			return FR_MODULE_ERROR;
		}

		return FR_MODULE_OK;
	} else if(strcmp(argv[0], "AddIcon") == 0) {
		if(argc >= 3) {
			int i;

			for(i = 2; i < argc; i++) {
				if(strchr(argv[i], '*') != NULL || strchr(argv[i], '?') != NULL) {
					context->stringarraykv_push(context->config_current->arraykv, "IconMatch", argv[i]);
					context->stringarraykv_push(context->config_current->arraykv, "IconMatch", argv[1]);
				} else {
					char* name = malloc(16 + strlen(argv[i]) + 1);

					sprintf(name, "Icon_%s", argv[i]);
					context->stringkv_set(&context->config_current->kv, name, argv[1]);
					free(name);
				}
			}
		} else {
			fprintf(stderr, "%s: %s: AddIcon takes 2 arguments or more\n", context->argv0, context->config_path);

			return FR_MODULE_ERROR;
		}
	} else if(strcmp(argv[0], "AddIconByType") == 0) {
		if(argc >= 3) {
			int i;

			for(i = 2; i < argc; i++) {
				context->stringarraykv_push(context->config_current->arraykv, "IconType", argv[i]);
				context->stringarraykv_push(context->config_current->arraykv, "IconType", argv[1]);
			}
		} else {
			fprintf(stderr, "%s: %s: AddIconByType takes 2 arguments or more\n", context->argv0, context->config_path);

			return FR_MODULE_ERROR;
		}
	}

	return FR_MODULE_DECLINE;
}

static void register_stuff(fr_context_t* context) {
	context->register_hook(hook_rewrite, FR_MODULE_HOOK_REWRITE);
	context->register_hook(hook, FR_MODULE_HOOK_LAST);
}

FR_MODULE_DATA fr_module_t dir_module = {
    FR_MODULE_VERSION_00,
    directive,	    /* directive */
    register_stuff, /* register_stuff */
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL};
