#include <fpr.h>
#include <fpr_int.h>

fpr_uint16_t fpr_htons(fpr_uint16_t host16) {
	return htons(host16);
}

static int sort_v6(const void* _a, const void* _b) {
	int* a = (int*)_a;
	int* b = (int*)_b;

	if(a[1] > b[1]) return -1;
	if(a[1] < b[1]) return 1;

	return 0;
}

const char* fpr_inet_ntop(struct fpr_sockaddr* src, char* dst) {
	if(src->sa_family == FPR_AF_INET) {
		struct fpr_sockaddr_in* addr = (struct fpr_sockaddr_in*)src;
		int			i;

		dst[0] = 0;

		for(i = 0; i < 4; i++) {
			if(i > 0) strcat(dst, ".");
			sprintf(dst + strlen(dst), "%d", (int)addr->sin_addr.u.addr8[i]);
		}
		return dst;
	} else if(src->sa_family == FPR_AF_INET6) {
		struct fpr_sockaddr_in6* addr = (struct fpr_sockaddr_in6*)src;
		int			 i;
		char			 v6[8][5];
		int			 l[8][2]; /* index, count */
		int			 c   = -1;
		int			 z   = 0;
		int			 max = 8;

		dst[0] = 0;

		for(i = 0; i < 16; i += 2) {
			v6[i / 2][0] = 0;

			if(addr->sin6_addr.u.addr8[i + 0] > 0) {
				sprintf(v6[i / 2], "%x", (int)addr->sin6_addr.u.addr8[i + 0]);
			}
			if(addr->sin6_addr.u.addr8[i + 0] > 0 && !(addr->sin6_addr.u.addr8[i + 1] & 0xf0)) strcat(v6[i / 2], "0");
			sprintf(v6[i / 2] + strlen(v6[i / 2]), "%x", (int)addr->sin6_addr.u.addr8[i + 1]);
		}

		for(i = 0; i < 8; i++) l[i][0] = l[i][1] = 0;

		for(i = 7; i >= 0; i--) {
			if(z && strcmp(v6[i], "0") == 0) {
				l[c][0] = i;
				l[c][1]++;
				continue;
			} else if(z) {
				z = 0;
			}

			if(strcmp(v6[i], "0") == 0) {
				z = 1;

				c++;

				l[c][0] = i;
				l[c][1] = 1;
			} else {
			}
		}

		qsort(l, 8, sizeof(int) * 2, sort_v6);

		dst[0] = 0;
		for(i = 0; i < max; i++) {
			if(i > 0) strcat(dst, ":");
			if(i == l[0][0] && l[0][1] > 0) {
				if(i == 0) strcat(dst, ":");

				i += l[0][1] - 1;
			} else {
				strcat(dst, v6[i]);
			}
		}

		if(memcmp(dst, "::ffff:", 7) == 0) {
			struct fpr_sockaddr_in in;

			dst[7] = 0;

			in.sin_family = FPR_AF_INET;
			memcpy(&in.sin_addr, &addr->sin6_addr.u.addr8[12], 4);

			fpr_inet_ntop((struct fpr_sockaddr*)&in, dst + 7);
		}

		return dst;
	}

	return NULL;
}
