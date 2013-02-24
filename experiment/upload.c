#include <stdio.h>
#include <stdlib.h>

#include "cgi/cgi.h"

int main(void)
{
	struct cgi_input *input;
	const struct cgi_file *file;
	const char *button;

	input = cgi_input_create();
	cgi_input_read(input);

	file = cgi_input_get_file(input, "file");
	button = cgi_input_get_value(input, "button");

	cgi_content_type = CGI_CONTENT_TYPE_TEXT_HTML;
	cgi_put_http_header();

	printf("<html>\n");
	printf("<head>\n");
	printf("<title>File Upload test</title>\n");
	printf("</head>\n");
	printf("<body>\n");
	printf("\n");

	printf("<h2>File Upload test</h2>\n");
	printf("\n");

	printf("<form action=\"%s\" method=\"post\" enctype=\"multipart/form-data\">\n", getenv("SCRIPT_NAME"));
	printf("<input type=\"file\" name=\"file\" />\n");
	printf("<input type=\"submit\" name=\"button\" value=\"upload\" />\n");
	printf("</form>\n");
	printf("\n");

	if (file != NULL) {
		int i;

		printf("<hr />\n");
		printf("<div>file:</div>\n");
		printf("<pre>\n");
		printf("filename: ");
		cgi_puts_html_escaped(file->filename);
		printf("\n");
		printf("type: ");
		cgi_puts_html_escaped(file->type);
		printf("\n");
		printf("size: %d\n", file->size);
		printf("data:\n");
		for (i=0; i<file->size; i++) {
			cgi_putc_html_escaped(*(file->data + i));
		}
		printf("</pre>\n");
		printf("\n");
	}

	if (button != NULL) {
		printf("<hr />\n");
		printf("<div>button:</div>\n");
		printf("<pre>\n");
		printf("button = ");
		cgi_puts_html_escaped(button);
		printf("\n");
		printf("</pre>\n");
		printf("\n");
	}

	printf("</body>\n");
	printf("</html>\n");

	cgi_input_destroy(input);

	return 0;
}
