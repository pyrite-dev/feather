#include <fpr.h>
#include <fpr_int.h>

#if defined(FPR_IS_WIN32)
typedef struct dir {
	fpr_bool	next;
	HANDLE		hFind;
	WIN32_FIND_DATA ffd;

	struct fpr_dirent dirent;
} dir_t;
#else
typedef struct dir {
	DIR* dir;

	struct fpr_dirent dirent;
} dir_t;
#endif

FPR_DIR* fpr_opendir(const char* path) {
	dir_t* dir = malloc(sizeof(*dir));
#if defined(FPR_IS_WIN32)
	char* p = fpr_strvacat(path, strchr(path, "/") != NULL ? "/" : "\\", "*", NULL);

	if((dir->hFind = FindFirstFile(p, &dir->ffd)) == INVALID_HANDLE_VALUE) {
		free(p);
		free(dir);

		return NULL;
	}
	free(p);

	dir->next = fpr_true;
#else
	if((dir->dir = opendir(path)) == NULL) {
		free(dir);

		return NULL;
	}
#endif

	return (FPR_DIR*)dir;
}

struct fpr_dirent* fpr_readdir(FPR_DIR* handle) {
	dir_t* dir = handle;
#if defined(FPR_IS_WIN32)
	if(!dir->next) return NULL;

	strcpy(dir->dirent.name, dir->ffd.cFileName);
#else
	struct dirent* d;

	if((d = readdir(dir->dir)) == NULL) return NULL;

	strcpy(dir->dirent.d_name, d->d_name);
#endif

	return &dir->dirent;
}

void fpr_closedir(FPR_DIR* handle) {
	dir_t* dir = handle;
#if defined(FPR_IS_WIN32)
	FindClose(dir->hFind);
#else
	closedir(dir->dir);
#endif

	free(dir);
}
