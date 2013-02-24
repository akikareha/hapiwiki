#include <stdio.h>
#include <stdlib.h>

#include "cgi/output.h"

int cgi_content_type = CGI_CONTENT_TYPE_TEXT_PLAIN;

void cgi_die(const char *s)
{
	cgi_put_http_header();
	printf("error: %s\n", s);
	exit(0);
}

void cgi_set_cookie(const char *name, const char *value)
{
	printf("Set-Cookie: ");
	cgi_puts_url_encoded(name);
	printf("=");
	cgi_puts_url_encoded(value);
	printf("\n");
}

void cgi_delete_cookie(const char *name)
{
	printf("Set-Cookie: ");
	cgi_puts_url_encoded(name);
	printf("=; expires=Thu, 01-Jan-1970 00:00:00 GMT;\n");
}

void cgi_put_http_header(void)
{
	static int done = 0;

	if (done) {
		return;
	}

	switch (cgi_content_type) {
		case CGI_CONTENT_TYPE_TEXT_PLAIN: {
			printf("Content-Type: text/plain; charset=UTF-8\n");
		} break;
		case CGI_CONTENT_TYPE_TEXT_HTML: {
			printf("Content-Type: text/html; charset=UTF-8\n");
		} break;
	}
	printf("\n");

	done = 1;
}

/* XXX missing some rules */
static int is_url_alnum(int c)
{
	if (c >= '0' && c <= '9') {
		return 1;
	}
	if (c >= 'A' && c <= 'Z') {
		return 1;
	}
	if (c >= 'a' && c <= 'z') {
		return 1;
	}
	return 0;
}

void cgi_putc_url_encoded(int c)
{
	if (is_url_alnum(c)) {
		fputc(c, stdout);
	} else if (c == ' ') {
		fputc('+', stdout);
	} else {
		printf("%%%02X", c & 0xff);
	}
}

void cgi_puts_url_encoded(const char *s)
{
	if (s == NULL) {
		return;
	}

	for (; *s!='\0'; s++) {
		cgi_putc_url_encoded(*s);
	}
}

/* XXX may be missing some rules. check the specification */
/* XXX should be macro or inlined */
void cgi_putc_html_escaped(int c)
{
	if (c == '<') {
		fputs("&lt;", stdout);
	} else if (c == '>') {
		fputs("&gt;", stdout);
	} else if (c == '&') {
		fputs("&amp;", stdout);
	} else if (c == '"') {
		fputs("&quot;", stdout);
	} else {
		fputc(c, stdout);
	}
}

void cgi_puts_html_escaped(const char *s)
{
	if (s == NULL) {
		return;
	}

	for (; *s!='\0'; s++) {
		cgi_putc_html_escaped(*s);
	}
}
