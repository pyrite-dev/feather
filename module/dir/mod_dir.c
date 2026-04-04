#define _FHTTPD
#include <fhttpd.h>

#include <string.h>
#include <stdlib.h>

static int hook_rewrite(fr_context_t* context, fr_request_t* req, fr_response_t* res) {
	char**		arr = NULL;
	int		len;
	struct fpr_stat st;
	int		i;

	(void)res;

	if(req->path[0] != '/') return FR_MODULE_DECLINE;

	if(fpr_stat(req->path_translated, &st) == 0 && FPR_S_ISDIR(st.st_mode) && req->path[strlen(req->path) - 1] != '/') {
		char* p = fpr_strvacat(req->path, "/", NULL);

		res->status_code = 301;
		strcpy(res->status_text, "Moved Permanently");

		context->response_set_header(res, "Location", p);

		free(p);

		return FR_MODULE_DECLINE;
	}

	arr = context->config_lookup_array(context, "DirectoryIndex", &len);

	for(i = 0; i < len; i++) {
		char* p = fpr_strvacat(req->path_translated, req->path_translated[strlen(req->path_translated) - 1] == '/' ? "" : "/", arr[i], NULL);

		if(fpr_stat(p, &st) == 0 && !FPR_S_ISDIR(st.st_mode) && (strlen(req->path_virtual) + strlen(arr[i])) <= MAX_PATH_LENGTH) {
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

static int sort_dir(const struct fpr_dirent** d1, const struct fpr_dirent** d2) {
	if(FPR_S_ISDIR((*d1)->d_stat.st_mode) && !FPR_S_ISDIR((*d2)->d_stat.st_mode)) return -1;
	if(!FPR_S_ISDIR((*d1)->d_stat.st_mode) && FPR_S_ISDIR((*d2)->d_stat.st_mode)) return 1;

	return fpr_alphasort(d1, d2);
}

static int hook(fr_context_t* context, fr_request_t* req, fr_response_t* res) {
	struct fpr_stat st;

	if(fpr_stat(req->path_translated, &st) == 0 && FPR_S_ISDIR(st.st_mode) && req->path[strlen(req->path) - 1] != '/') return FR_MODULE_DECLINE;

	if(fpr_stat(req->path_translated, &st) == 0 && FPR_S_ISDIR(st.st_mode)) {
		char*		    table = malloc(1);
		struct fpr_dirent** namelist;
		int		    n;

		table[0] = 0;

		fpr_strappend(&table, "		<tr>\n");
		fpr_strappend(&table, "			<th width=\"24\"></th>\n");
		fpr_strappend(&table, "			<th>Name</th>\n");
		fpr_strappend(&table, "			<th>Last modified</th>\n");
		fpr_strappend(&table, "			<th>Size</th>\n");
		fpr_strappend(&table, "		</tr>\n");

		if((n = fpr_scandir(req->path_translated, &namelist, NULL, sort_dir)) >= 0) {
			int i;
			for(i = 0; i < n; i++) {
				if(strcmp(namelist[i]->d_name, ".") != 0) {
					char*	      name;
					char*	      s;
					char	      date[64];
					char	      size[16];
					struct fpr_tm tm;
					fpr_size_t    sz = namelist[i]->d_stat.st_size;

					name = fpr_strvacat(namelist[i]->d_name, FPR_S_ISDIR(namelist[i]->d_stat.st_mode) ? "/" : "", NULL);
					s    = fpr_strsafehtml(name);
					free(name);

					fpr_gmtime(&tm, namelist[i]->d_stat.st_modtime);

					date[0] = 0;
					sprintf(date, "%d-%02d-%02d %02d:%02d UTC", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min);

					size[0] = 0;
					if(FPR_S_ISDIR(namelist[i]->d_stat.st_mode)) {
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

					fpr_strappend(&table, "		<tr>\n");
					fpr_strappend(&table, "			<td width=\"24\">\n");
					fpr_strappend(&table, "			</td>\n");
					fpr_strappend(&table, "			<td>\n");
					fpr_strappend(&table, "				<a href=\"");
					fpr_strappend(&table, s);
					fpr_strappend(&table, "\">");
					fpr_strappend(&table, strcmp(namelist[i]->d_name, "..") == 0 ? "Parent Directory" : s);
					fpr_strappend(&table, "</a>\n");
					fpr_strappend(&table, "			</td>\n");
					fpr_strappend(&table, "			<td>");
					fpr_strappend(&table, date);
					fpr_strappend(&table, "</td>\n");
					fpr_strappend(&table, "			<td>");
					fpr_strappend(&table, size);
					fpr_strappend(&table, "</td>\n");
					fpr_strappend(&table, "		</tr>\n");

					free(s);
				}
				free(namelist[i]);
			}
			free(namelist);
		}

		res->status_code = 200;
		strcpy(res->status_text, "OK");

		context->response_set_header(res, "Content-Type", "text/html");

		/* clang-format off */
		res->body = fpr_strvacat(
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
