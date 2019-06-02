/**
 *	Crawler-Module for C/C++ on linux.
 *	
 *	This module not supported to Mluti-byte character.
 */

#ifndef __HTML_GET_H_INCLUDED__
#define __HTML_GET_H_INCLUDED__

#include <stdbool.h>
#include "web_type.h"

/*Function declaration*/
extern _Bool parseurl(const char *url_str, URLObject *url_obj);
extern _Bool getonlinedata(const URLObject url_obj, char **data);
/*extern HTMLObject *converthtmlobj(const char *html_str);*/
/*extern void free_htmlobj(HTMLObject *html_obj);*/

#endif
