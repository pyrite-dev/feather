#include <fhttpd.h>

#include <stb_ds.h>

fr_stringkv_t* mime_types = NULL;

void mime_parse(void) {
	char*	  p = path_transform(config_mimefile);
	FPR_FILE* f;
	int	  i;

	mime_close();
	sh_new_strdup(mime_types);
	shdefault(mime_types, NULL);

	if((f = fpr_fopen(p, "r")) != NULL) {
		char buffer[BUFFER_SIZE];
		char line[LINE_SIZE + 1];
		int  s;

		line[0] = 0;

		while((s = fpr_fread(buffer, 1, BUFFER_SIZE, f)) > 0) {
			int i;

			for(i = 0; i < s; i++) {
				if(buffer[i] == '\n') {
					if(line[0] != '#') {
						int j;

						for(j = 0; line[j] != 0 && line[j] != ' ' && line[j] != '\t'; j++);
						if(line[j] != 0) {
							char* s;

							line[j++] = 0;
							for(; line[j] != 0 && (line[j] == ' ' || line[j] == '\t'); j++);

							s = line + j;
							while(1) {
								char* f = s;
								char* m = fpr_strdup(line);

								s = strchr(s, ' ');
								if(s != NULL) s[0] = 0;

								shput(mime_types, f, m);

								if(s == NULL) break;
								s++;
							}
						}
					}

					line[0] = 0;
				} else if(buffer[i] != '\r') {
					if(strlen(line) != LINE_SIZE) {
						line[strlen(line) + 1] = 0;
						line[strlen(line)]     = buffer[i];
					}
				}
			}
		}

		fpr_fclose(f);
	}

	free(p);
}

void mime_close(void) {
	int i;
	for(i = 0; i < shlen(mime_types); i++) {
		free(mime_types[i].value);
	}
	shfree(mime_types);
}
