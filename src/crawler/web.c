#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "web_type.h"
#include "crawl_conf.h"

#ifdef ENABLE_HTTP_SECURE
# include <openssl/ssl.h>
# include <openssl/err.h>
#endif

/**
 *	Convert URL-string to URL-object.
 *	URL-object is defined in "htmlget.h".
 *
 *	@arg url_str	- [in]URL-String(Max 511(BUFSIZ)byte)
 *	@arg url_obj	- [out]URL-Object
 *	@return			- Success: true  Failure: false
 */
_Bool parseurl(const char *url_str, URLObject *url_obj){
	if(strlen(url_str) > URLSIZE-2) return false;

	if(((strstr(url_str, "http://") == url_str) && sscanf(url_str, "http://%s", url_obj->url_info) && strcmp(url_str, "http://") && (url_obj->protocol = HTTP)) ||
	   ((strstr(url_str, "https://") == url_str) && sscanf(url_str, "https://%s", url_obj->url_info) && strcmp(url_str, "https://") && (url_obj->protocol = HTTPS)))	{		
		char *sp;
		url_obj->port = (url_obj->protocol == HTTP ? "80" : "443");
		url_obj->fqdn = url_obj->url_info;
		sp = strchr(url_obj->url_info, '/');
		if(sp != NULL){
			url_obj->path = sp+1;
			*sp = '\0';
		}else{
			url_obj->path = NULL;
		}
		sp = strchr(url_obj->url_info, ':');
		if(sp != NULL){
			url_obj->port = sp+1;
			*sp = '\0';
		}
		return true;
	}
	return false;
}

/**
 *	Acquire data from the Internet.
 *
 *	@arg url_obj	- [in]URL-Object
 *	@arg data		- [out]acquired data
 *	@return			- Success: true  Failure: false
 */
_Bool getonlinedata(const URLObject url_obj, char **data){
#ifndef ENABLE_HTTP_SECURE
	if(url_obj.protocol == HTTPS){
		return false;
	}
#endif
	struct addrinfo hints;
	struct addrinfo *result;
	struct addrinfo *rp;
	
	int s;
	unsigned int rindex;
	unsigned int rblockindex;
	unsigned int retry;
	char sbuf[BUFSIZE];
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
		return false;
	}
	freeaddrinfo(result);

#ifdef ENABLE_HTTP_SECURE
	int err;
	SSL *ssl;
	SSL_CTX *ctx;
	if(url_obj.protocol == HTTPS){
		SSL_load_error_strings();
		SSL_library_init();

		ctx = SSL_CTX_new(SSLv23_client_method());
		ssl = SSL_new(ctx);
		err = SSL_set_fd(ssl, s);
		SSL_connect(ssl);
	}
#endif

	sprintf(sbuf, "GET /%s HTTP/1.0\r\nHost: %s:%s\r\n\r\n", (url_obj.path == NULL ? "" : url_obj.path), url_obj.fqdn, url_obj.port);
#ifdef ENABLE_HTTP_SECURE
	if(url_obj.protocol == HTTPS){
		SSL_write(ssl, sbuf, strlen(sbuf));
	}else{
#endif
	write(s, sbuf, strlen(sbuf));
#ifdef ENABLE_HTTP_SECURE
	}
#endif

	if((rbuf = (char*)malloc(RECSIZE)) == NULL){
		close(s);
		return false;
	}
	rindex = 0;
	rblockindex = 0;
	retry = 0;
#ifdef ENABLE_HTTP_SECURE
	if(url_obj.protocol == HTTPS){
		while(1){
			if(SSL_read(ssl, rbuf+(RECSIZE*rblockindex)+rindex, 1) > 0){
				if(retry != 0) retry = 0;
				++rindex;
				if(rindex == RECSIZE){
					if((t_rbuf = (char*)realloc(rbuf, RECSIZE*(rblockindex+2))) == NULL){
						free(rbuf);
						SSL_shutdown(ssl);
						SSL_free(ssl);
						SSL_CTX_free(ctx);
						ERR_free_strings();
						close(s);
						return false;
					}
					rbuf = t_rbuf;
					rindex = 0;
					++rblockindex;
				}
			}else{
				//if read-byte is 0, retry.
				//("read-byte is 0" means "Download Completed Successfully" or "Download Failure". So, try again and re-jadge.)
				if(retry != RETRY_COUNT){
					++retry;
					continue;
				}
				break;
			}
		}
	}else{
#endif
	while(1){
		if(read(s, rbuf+(RECSIZE*rblockindex)+rindex, 1) > 0){
			if(retry != 0) retry = 0;
			++rindex;
			if(rindex == RECSIZE){
				if((t_rbuf = (char*)realloc(rbuf, RECSIZE*(rblockindex+2))) == NULL){
					free(rbuf);
					close(s);
					return false;
				}
				rbuf = t_rbuf;
				rindex = 0;
				++rblockindex;
			}
		}else{
			//if read-byte is 0, retry.
			//("read-byte is 0" means "Download Completed Successfully" or "Download Failure". So, try again and re-jadge.)
			if(retry != RETRY_COUNT){
				++retry;
				continue;
			}
			break;
		}
	}
#ifdef ENABLE_HTTP_SECURE
	}
#endif

#ifdef ENABLE_HTTP_SECURE
	if(url_obj.protocol == HTTPS){
		SSL_shutdown(ssl);
		SSL_free(ssl);
		SSL_CTX_free(ctx);
		ERR_free_strings();
	}
#endif
	close(s);
	*data = rbuf;
	return true;
}
