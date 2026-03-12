#include <fpr.h>
#include <fpr_int.h>

int fpr_stat(const char* path, struct fpr_stat* s) {
#if defined(_WIN32)
	WIN32_FILE_ATTRIBUTE_DATA fad;
	ULARGE_INTEGER		  mtime;

	if(!GetFileAttributesEx(path, GetFileExInfoStandard, &fad)) return -1;

	memcpy(&mtime, &fad.ftLastWriteTime, sizeof(mtime));

	s->st_size    = 0;
	s->st_modtime = (mtime.QuadPart / 10000000) / 11644473600;
	s->st_mode    = 0;

	s->st_mode |= FPR_S_IFREG;
	if(fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		s->st_mode &= ~FPR_S_IFREG;
		s->st_mode |= FPR_S_IFDIR;
	}

	if(FPR_S_ISREG(s->st_mode)) {
		FPR_FILE* f = fpr_fopen(path, "r"); /* :))))))) */
		s->st_size  = GetFileSize((HANDLE)f, NULL);
		fpr_fclose(f);
	}

	return 0;
#else
	struct stat st;
	int	    sc;

	sc = stat(path, &st);
	if(sc != 0) return sc;

	s->st_size    = st.st_size;
	s->st_modtime = st.st_mtime;
	s->st_mode    = 0;

	if(S_ISREG(st.st_mode)) s->st_mode |= FPR_S_IFREG;
	if(S_ISDIR(st.st_mode)) s->st_mode |= FPR_S_IFDIR;

	return sc;
#endif
}
