#include <fhttpd.h>

fpr_bool http_got(client_t* c, void* buffer, int size){
	int i;
	char* buf = buffer;

	for(i = 0; i < size; i++){
		if(c->state == CS_CONNECTED){
			if(buf[i] == ' '){
				c->state = CS_GOT_METHOD;
			}else{
				if(strlen(c->method) == MAX_METHOD_LENGTH){
					return fpr_false;
				}else{
					c->method[strlen(c->method)] = buf[i];
				}
			}
		}
	}

	return fpr_true;
}
