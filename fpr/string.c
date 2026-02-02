#include <fpr.h>
#include <fpr_int.h>

char* fpr_strdup(const char* str) {
	char* r = malloc(strlen(str) + 1);

	strcpy(r, str);

	return r;
}
