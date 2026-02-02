#include <fhttpd.h>

#include <stb_ds.h>

char** arg_parse(const char* str) {
	char**	 r  = NULL;
	int	 i  = 0;
	char*	 d  = fpr_strdup(str);
	int	 f  = 0;
	fpr_bool dq = fpr_false;

	while(1) {
		if(str[i] == 0 || (!dq && (str[i] == '\t' || str[i] == ' '))) {
			char*	 v;
			char*	 s;
			int	 j, k = 0;
			fpr_bool b = str[i] == 0;

			d[i] = 0;
			v    = fpr_strdup(s = d + f);
			for(j = 0; s[j] != 0; j++) {
				if(s[j] != '"') v[k++] = s[j];
			}
			v[k] = 0;

			arrput(r, v);

			while(str[i] != 0 && (str[i] == '\t' || str[i] == ' ')) i++;

			f = i;

			if(b) break;
			continue;
		} else if(str[i] == '"') {
			dq = !dq;
		}
		i++;
	}

	free(d);

	arrput(r, NULL);

	return r;
}

int arg_len(char** args) {
	return arrlen(args) - 1;
}

void arg_free(char** args) {
	int i;

	for(i = 0; i < arg_len(args); i++) {
		free(args[i]);
	}
	arrfree(args);
}
