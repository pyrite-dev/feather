#include <fhttpd.h>

#include <stb_ds.h>

char* util_stringkv_lookup(fr_stringkv_t* kv, const char* key) {
	return shget(kv, key);
}

void util_stringkv_set(fr_stringkv_t** kv, const char* key, const char* value) {
	int   ind;
	char* v = fpr_strdup(value);

	if((ind = shgeti(*kv, value)) != -1) free((*kv)[ind].value);
	shdel(*kv, value);
	shput(*kv, key, v);
}

char** util_stringarraykv_lookup(fr_stringarraykv_t* arraykv, const char* key) {
	int ind;

	if((ind = shgeti(arraykv, key)) == -1) return NULL;

	return arraykv[ind].value;
}

void util_stringarraykv_push(fr_stringarraykv_t* arraykv, const char* key, const char* value) {
	int    ind;
	char** v   = NULL;
	char*  val = fpr_strdup(value);

	if((ind = shgeti(arraykv, key)) != -1) v = arraykv[ind].value;

	arrput(v, val);

	if(ind == -1) {
		shput(arraykv, key, v);
	} else {
		arraykv[ind].value = v;
	}
}

int util_stringarraykv_length(fr_stringarraykv_t* arraykv, const char* key) {
	int ind;

	if((ind = shgeti(arraykv, key)) == -1) return 0;

	return arrlen(arraykv[ind].value);
}
