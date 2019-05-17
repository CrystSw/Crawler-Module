/*
Crawl-Helper Module for C/C++ on linux.

This module not supported to Mluti-byte character.
*/

#ifndef __HTML_GET_H_INCLUDED__
#define __HTML_GET_H_INCLUDED__

#include "crawl_type.h"

/*Function declaration*/
extern int parseurl(const char *url_str, URLObject *url_obj);
extern char *getonlinedata(URLObject *url_obj)
extern HTMLObject *converthtmlobj(const char *html_str);
extern void free_htmlobj(HTMLObject *html_obj);

#endif
