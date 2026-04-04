#include <fpr.h>
#include <fpr_int.h>

#if defined(FPR_IS_WIN32)
typedef struct dir {
	fpr_bool	next;
	HANDLE		hFind;
	WIN32_FIND_DATA ffd;

	char*		  path;
	struct fpr_dirent dirent;
} dir_t;
#else
typedef struct dir {
	DIR* dir;

	char*		  path;
	struct fpr_dirent dirent;
} dir_t;
#endif

FPR_DIR* fpr_opendir(const char* path) {
	dir_t* dir = malloc(sizeof(*dir));
#if defined(FPR_IS_WIN32)
	char* p = fpr_strvacat(path, strchr(path, '/') != NULL ? "/" : "\\", "*", NULL);

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

	dir->path = fpr_strdup(path);

	return (FPR_DIR*)dir;
}

struct fpr_dirent* fpr_readdir(FPR_DIR* handle) {
	dir_t* dir = handle;
	char*  d_fullname;
#if defined(FPR_IS_WIN32)
	if(!dir->next) return NULL;

	strcpy(dir->dirent.d_name, dir->ffd.cFileName);
#else
	struct dirent* d;

	if((d = readdir(dir->dir)) == NULL) return NULL;

	strcpy(dir->dirent.d_name, d->d_name);
#endif

	d_fullname = malloc(strlen(dir->path) + 1 + strlen(dir->dirent.d_name) + 1);
	strcpy(d_fullname, dir->path);
	strcat(d_fullname, strchr(dir->path, '/') != NULL ? "/" : "\\");
	strcat(d_fullname, dir->dirent.d_name);

	memset(&dir->dirent.d_stat, 0, sizeof(dir->dirent.d_stat));
	fpr_stat(d_fullname, &dir->dirent.d_stat);

	free(d_fullname);

	return &dir->dirent;
}

void fpr_closedir(FPR_DIR* handle) {
	dir_t* dir = handle;
#if defined(FPR_IS_WIN32)
	FindClose(dir->hFind);
#else
	closedir(dir->dir);
#endif

	free(dir->path);

	free(dir);
}

int fpr_scandir(const char* dirname, struct fpr_dirent*** namelist, int (*selectfn)(const struct fpr_dirent* d), int (*compar)(const struct fpr_dirent** d1, const struct fpr_dirent** d2)) {
	FPR_DIR*	   dir = fpr_opendir(dirname);
	struct fpr_dirent* d;
	int		   n  = 0;
	int		   n2 = 0;
	if(dir == NULL) return -1;

	while((d = fpr_readdir(dir)) != NULL) n++;

	fpr_closedir(dir);

	if((dir = fpr_opendir(dirname)) == NULL) return -1;

	*namelist = malloc(sizeof(**namelist) * n);

	while(n2 < n && (d = fpr_readdir(dir)) != NULL) {
		if(selectfn != NULL && !selectfn(d)) continue;

		(*namelist)[n2] = malloc(sizeof(***namelist));
		memcpy((*namelist)[n2], d, sizeof(*d));

		n2++;
	}

	fpr_closedir(dir);

	qsort(*namelist, n2, sizeof(**namelist), (int (*)(const void* d1, const void* d2))compar);

	return n2;
}

int fpr_alphasort(const struct fpr_dirent** d1, const struct fpr_dirent** d2) {
	return strcmp((*d1)->d_name, (*d2)->d_name);
}
