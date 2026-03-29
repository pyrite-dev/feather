#ifndef __FPR_URL_H__
#define __FPR_URL_H__

#include <fpr_machdep.h>

fpr_bool fpr_url_decode(char* out, const char* input, int len);
fpr_bool fpr_url_encode(char* out, const char* input, int len);

#endif
