#include <fpr.h>
#include <fpr_int.h>

int fpr_gethostname(char* name, int namelen) {
	return gethostname(name, namelen);
}
