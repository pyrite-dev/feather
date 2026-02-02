#include <fhttpd.h>

char* path_transform(const char* path) {
	if(strlen(path) > 2 && (									  /**/
				strstr(path, ":/") != NULL ||						  /**/
				strstr(path, ":\\") != NULL ||						  /**/
				path[0] == '/' ||							  /**/
				path[0] == '\\' ||							  /**/
				(path[0] == '.' && (path[1] == '/' || path[1] == '\\')) ||		  /**/
				(path[0] == '.' && path[1] == '.' && (path[2] == '/' || path[2] == '\\')) /**/
				)) {
		return fpr_strdup(path);
	}

	return fpr_strvacat_alloc(serverroot, "/", path, NULL);
}
