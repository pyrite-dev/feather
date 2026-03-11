#include <fpr.h>
#include <fpr_int.h>

int fpr_stat(const char* path, struct fpr_stat* s) {
#if defined(_WIN32)
	FILE_STAT_INFORMATION info;

	if(!GetFileInformationByName(path, FileStatByNameInfo, &info, sizeof(info))) return -1;

	s->st_size    = 0;
	s->st_modtime = (info.QuadPart / 10000000) / 11644473600;
	s->st_mode    = 0;

	if(info.FileAttributes & FILE_ATTRIBUTE_NORMAL) s->st_mode |= FPR_S_IFREG;
	if(info.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) s->st_mode |= FPR_S_IFDIR;

	if(info.FileAttributes & FILE_ATTRIBUTE_NORMAL) {
		FPR_FILE* f = fpr_fopen(path, "r"); /* :))))))) */
		s->st_size  = GetFileSize((HANDLE)f);
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
