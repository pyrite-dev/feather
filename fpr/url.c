#include <fpr.h>
#include <fpr_int.h>

static int hex(const char in) {
	if('0' <= in && in <= '9') return in - '0';
	if('a' <= in && in <= 'f') return in - 'a' + 10;
	if('A' <= in && in <= 'F') return in - 'A' + 10;

	return 0;
}

fpr_bool fpr_url_decode(char* out, const char* input, int len) {
	int i;

	for(i = 0; input[i] != 0; i++) {
		if(strlen(out) == len) {
			return fpr_false;
		} else if(input[i] == '%') {
			if(strlen(input + i) >= 3) {
				int n	   = strlen(out);
				out[n]	   = (hex(input[i + 1]) << 4) | (hex(input[i + 2]));
				out[n + 1] = 0;

				i += 2;
			}
		} else {
			int n	   = strlen(out);
			out[n]	   = input[i];
			out[n + 1] = 0;
		}
	}

	return fpr_true;
}

fpr_bool fpr_url_encode(char* out, const char* input, int len) {
	int  i;
	char hexstr[] = "0123456789ABCDEF";

	for(i = 0; input[i] != 0; i++) {
		if(strlen(out) == len) {
			return fpr_false;
		} else if(input[i] == '%') {
			if(strlen(input + i) >= 3 && strlen(out) <= (len - 3)) {
				int n = strlen(out);

				out[n]	   = '%';
				out[n + 1] = '2';
				out[n + 2] = '5';
				out[n + 3] = 0;
			}
		} else {
			int n	   = strlen(out);
			out[n]	   = input[i];
			out[n + 1] = 0;
		}
	}

	return fpr_true;
}
