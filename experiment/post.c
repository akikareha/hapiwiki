#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cgi/cgi.h"

static void put_args(const struct cgi_arg *arg) {
  for (; arg != NULL; arg = arg->next) {
    cgi_puts_html_escaped(arg->name);
    printf(" = ");
    if (arg->value == NULL) {
      printf("<i>(null)</i>");
    } else {
      cgi_puts_html_escaped(arg->value);
    }
    printf("\n");
  }
}

int main(void) {
  struct cgi_input *input;
  const char *arg_textfield, *arg_textarea, *arg_name, *arg_value, *arg_delete;

  input = cgi_input_create();
#ifndef COMPOUND
  cgi_input_read(input);
#else
  cgi_input_read_compound(input);
#endif

  arg_textfield = cgi_input_get_value(input, "textfield");
  arg_textarea = cgi_input_get_value(input, "textarea");
  arg_name = cgi_input_get_value(input, "name");
  arg_value = cgi_input_get_value(input, "value");
  arg_delete = cgi_input_get_value(input, "delete");

  if (arg_name != NULL) {
    if (arg_delete != NULL) {
      cgi_delete_cookie(arg_name);
    } else if (arg_value != NULL) {
      cgi_set_cookie(arg_name, arg_value);
    }
  }

  cgi_content_type = CGI_CONTENT_TYPE_TEXT_HTML;
  cgi_put_http_header();

  printf("<html>\n");
  printf("<head>\n");
#ifndef COMPOUND
  printf("<title>POST test</title>\n");
#else
  printf("<title>POST test (compound cookie version)</title>\n");
#endif
  printf("</head>\n");
  printf("<body>\n");
  printf("\n");

#ifndef COMPOUND
  printf("<h2>POST test</h2>\n");
#else
  printf("<h2>POST test (compound cookie version)</h2>\n");
#endif
  printf("\n");

  printf("<form action=\"%s\" method=\"post\">\n", getenv("SCRIPT_NAME"));

  printf("<div>textfield:</div>\n");
  printf("<input type=\"text\" name=\"textfield\" value=\"");
  cgi_puts_html_escaped(arg_textfield);
  printf("\" /><br />\n");

  printf("<div>textarea:</div>\n");
  printf("<textarea name=\"textarea\">\n");
  cgi_puts_html_escaped(arg_textarea);
  printf("</textarea><br />\n");

  printf("<div>cookie:</div>\n");
  printf("<blockquote>\n");
  printf("name: <input type=\"text\" name=\"name\" /><br />\n");
  printf("value: <input type=\"text\" name=\"value\" /><br />\n");
  printf("<input type=\"checkbox\" name=\"delete\" value=\"true\" /> delete<br "
         "/>\n");
  printf("</blockquote>\n");

  printf("<div>button:</div>\n");
  printf("<input type=\"submit\" name=\"button\" value=\"send\" /><br />\n");

  printf("</form>\n");
  printf("\n");

  printf("<hr />\n");
  printf("\n");

  printf("<div>query:</div>\n");
  printf("<pre>\n");
  if (input->query == NULL) {
    printf("<i>query is NULL</i>\n");
  } else {
    put_args(input->query->next);
  }
  printf("</pre>\n");
  printf("\n");

  printf("<hr />\n");
  printf("\n");

  printf("<div>post:</div>\n");
  printf("<pre>\n");
  if (input->post == NULL) {
    printf("<i>post is NULL</i>\n");
  } else {
    put_args(input->post->next);
  }
  printf("</pre>\n");
  printf("\n");

  printf("<hr />\n");
  printf("\n");

#ifndef COMPOUND
  printf("<div>cookie:</div>\n");
  printf("<pre>\n");
  if (input->cookie == NULL) {
    printf("<i>cookie is NULL</i>\n");
  } else {
    put_args(input->cookie->next);
  }
  printf("</pre>\n");
  printf("\n");
#else
  printf("<div>compound cookie:</div>\n");
  printf("<pre>\n");
  if (input->compound_cookie == NULL) {
    printf("<i>compound cookie is NULL</i>\n");
  } else {
    put_args(input->compound_cookie->next);
  }
  printf("</pre>\n");
  printf("\n");
#endif

  printf("</body>\n");
  printf("</html>\n");

  cgi_input_destroy(input);

  return 0;
}
