#include <fpr.h>

int fpr_stat(const char* path, struct fpr_stat* s) {
#if defined(_WIN32)
	FILE_STAT_INFORMATION info;

	GetFileInformationByName(path, FileStatByNameInfo, &info, sizeof(info));

	s->st_size  = 0;
	s->st_mtime = (info.QuadPart / 10000000) / 11644473600;
	s->st_mode  = 0;

	if(info.FileAttributes & FILE_ATTRIBUTE_NORMAL) s->st_mode |= FPR_S_IFREG;
	if(info.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) s->st_mode |= FPR_S_IFDIR;

	if(info.FileAttributes & FILE_ATTRIBUTE_NORMAL) {
		FPR_FILE* f = fpr_fopen(path, "r"); /* :))))))) */
		s->st_size  = GetFileSize((HANDLE)f);
		fpr_fclose(f);
	}
#else
	struct stat s;
	int	    st;

	st = stat(path, &s);

	s->st_size  = st.st_size;
	s->st_mtime = st.st_mtime;
	s->st_mode  = 0;

	if(S_ISREG(st.st_mode)) s->st_mode |= FPR_S_IFREG;
	if(S_ISDIR(st.st_mode)) s->st_mode |= FPR_S_IFDIR;

	return st;
#endif
}
