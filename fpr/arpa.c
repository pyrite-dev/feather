#include <fpr.h>
#include <fpr_int.h>

fpr_uint16_t fpr_htons(fpr_uint16_t host16) {
	return htons(host16);
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

		dst[0] = 0;

		for(i = 0; i < 16; i += 2) {
			if(i > 0) strcat(dst, ":");

			if(!(addr->sin6_addr.u_addr8[i + 0] & 0xf0)) strcat(dst, "0");
			sprintf(dst + strlen(dst), "%x", (int)addr->sin6_addr.u.addr8[i + 0]);
			if(!(addr->sin6_addr.u_addr8[i + 1] & 0xf0)) strcat(dst, "0");
			sprintf(dst + strlen(dst), "%x", (int)addr->sin6_addr.u.addr8[i + 1]);
		}
		return dst;
	}

	return NULL;
}
