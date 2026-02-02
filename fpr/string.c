#include <fpr.h>
#include <fpr_int.h>

char* fpr_strdup(const char* str) {
	char* r = malloc(strlen(str) + 1);

	strcpy(r, str);

	return r;
}

char* fpr_strvacat_alloc(const char* a, ...) {
	int	l = 1 + strlen(a);
	va_list va;
	char*	s;
	char*	r;

	va_start(va, a);
	while((s = va_arg(va, char*)) != NULL) {
		l += strlen(s);
	}
	va_end(va);

	r = malloc(l);

	strcpy(r, a);

	va_start(va, a);
	while((s = va_arg(va, char*)) != NULL) {
		strcat(r, s);
	}
	va_end(va);

	return r;
}
