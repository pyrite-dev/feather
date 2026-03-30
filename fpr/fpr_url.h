#ifndef __FPR_URL_H__
#define __FPR_URL_H__

#include <fpr_machdep.h>

typedef struct fpr_url fpr_url_t;

struct fpr_url {
	char* scheme;
	char* userinfo;
	char* host;
	int   port;
	char* path;
	char* query;
	char* fragment;
};

fpr_bool fpr_url_decode(char* out, const char* input, int len);
fpr_bool fpr_url_encode(char* out, const char* input, int len);

void	 fpr_url_init(fpr_url_t* url);
fpr_bool fpr_url_parse(fpr_url_t* url, const char* str);
void	 fpr_url_deinit(fpr_url_t* url);

#endif
