/*
Define of Type, Enumerator
*/

/*HTML Object Type-Constant*/
enum protocol {
	HTTP = 1,
	HTTPS
}

/*HTML Object Type-Constant*/
enum htmlobj_type {
	HTML_ROOT,
	HTML_TAG,
	HTML_ATTRIBUTE,
	HTML_STRING
}

enum err {
	UNKNOWN_ERROR = -20,
	URL_TOO_LONG,
	INVALID_SYNTAX
}

/*Structure declaration*/
struct url_obj;
struct html_obj;
struct obj_list;

/*Structure definition*/
typedef struct url_obj {
	char url_info[BUFSIZ];
	unsigned char protocol;
	char *fqdn;
	unsigned short port;
	char *path;
} URLObject;

typedef struct html_obj {
	char type;
	char name[BUFSIZ];
	char value[BUFSIZ];
	struct obj_list *list;
} HTMLObject;

struct obj_list {
	HTMLObject *obj;
	struct obj_list *next;
};

