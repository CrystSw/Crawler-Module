#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "crawl_type.h"

/**
 *	Convert URL-string to URL-object.
 *	URL-object is defined in "htmlget.h".
 *
 *	@arg url_str	- [in]URL-String(Max 511(BUFSIZ)byte)
 *	@arg url_obj	- [out]URL-Object
 *	@return			- Success: 1  Failure: 0
 */
int parse_url(const char *url_str, URLObject *url_obj){
	if(strlen(url_str) > BUFSIZ-2) return 0;

	if(((strstr(url_str, "http://") == url_str) && (sscanf("http://%s", url_obj->url_info) && strcmp(url_str, "http://") && (url_obj->protocol = HTTP)) ||
	   ((strstr(url_str, "https://") == url_str) && (sscanf("https://%s", url_obj->url_info) && strcmp(url_str, "https://") && (url_obj->protocol = HTTPS))){		
		char *sp;
		url_obj->port = (url_obj->protocol == HTTP ? 80 : 443);
		url_obj->fqdn = url_obj->url_info;
		sp = strchr(url_obj->url_info, '/');
		if(sp != NULL){
			url_obj->path = sp+1;
			*sp = '\0';
		}
		sp = strchr(url_obj->url_info, ':');
		if(sp != NULL){
			url_obj->port = (unsigned short)atoi(sp+1);
			*sp = '\0';
		}
		return 1;
	}
	return 0;
}

/**
 *	Acquire data from the Internet.
 *
 *	@arg url_obj	- [in]URL-Object
 *	@arg data		- [out]acquired data
 *	@return			- Success: 1  Failure: 0
 */
int getonlinedata(const URLObject *url_obj, char **data){	
	struct addrinfo hints;
	struct addrinfo *result;
	struct addrinfo *rp;
	
	int s;
	unsigned int r_index;
	char sbuf[BUFSIZ];
	char *rbuf;
	char *t_rbuf;
	
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	
	if(getaddrinfo(url_obj.fqdn, url_obj.port, &hints, &result) != 0) return 0;
	
	for(rp = result; rp != NULL; rp = rp->ai_next){
		if((s = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == -1){
			continue;
		}
		if(connect(s, rp->ai_addr, rp->ai_addrlen) != -1){
			break;
		}
		close(s);
	}
	if(rp == NULL){
		return 0;
	}
	
	freeaddrinfo(result);
	
	sprintf(sbuf, "GET %s HTTP/1.0\r\n", url_obj.path);
	write(s, sbuf, strlen(sbuf));
	sprintf(sbuf, "Host: %s:%d\r\n", url_obj.host, url_obj.port);
    write(s, sbuf, strlen(sbuf));
    sprintf(sbuf, "\r\n");
    write(s, sbuf, strlen(sbuf));
	
	if((rbuf = (char*)malloc(BUFSIZ)) == NULL){
		close(s);
		return 0;
	}
	r_index = 0;
	while(1){
		if(read(s, rbuf+(BUFSIZ*r_index), BUFSIZ) > 0){
			if((t_rbuf = (char*)realloc(rbuf, BUFSIZ*(r_index+2))) == NULL){
				free(rbuf);
				close(s);
				return 0;
			}
			rbuf = t_rbuf;
			r_index++;
		}else{
			break;
		}
	}
	close(s);
	
	*data = rbuf;
	return 1;
}