#ifndef CGIINC_OUTPUT_H
#define CGIINC_OUTPUT_H

enum {
	CGI_CONTENT_TYPE_TEXT_PLAIN,
	CGI_CONTENT_TYPE_TEXT_HTML,
};

extern int cgi_content_type;

extern void cgi_die(const char *s) __attribute__ ((__noreturn__));
extern void cgi_set_cookie(const char *name, const char *value);
extern void cgi_delete_cookie(const char *name);
extern void cgi_put_http_header(void);
extern void cgi_putc_url_encoded(int c);
extern void cgi_puts_url_encoded(const char *s);
extern void cgi_putc_html_escaped(int c);
extern void cgi_puts_html_escaped(const char *s);

#endif
