#include <stdio.h>
#include <stdlib.h>
#include "crawler/web.h"

int main(void){
	URLObject url;
	if(!parseurl("https://crystarks.net/readme.html", &url)){
		fprintf(stderr, "URL Parse Error.\n");
		exit(1);
	}

	char *html;
	if(!getonlinedata(url, &html)){
		fprintf(stderr, "HTML Get Error.\n");
		exit(1);
	}

	printf("%s\n", html);

	return 0;
}
