#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cgi/cgi.h"

#include "wiki/draw.h"
#include "wiki/format.h"

static char *script_name;

static void wiki_draw_http_header(const struct wiki_args *args) {
  if (args->command == WIKI_COMMAND_LOGIN) {
    printf("Set-Cookie: wiki=");
    cgi_puts_url_encoded("account=");
    cgi_puts_url_encoded(args->account);
    cgi_puts_url_encoded("&session=");
    cgi_puts_url_encoded(args->session);
    printf("\n");
  } else if (args->command == WIKI_COMMAND_LOGOUT) {
    cgi_delete_cookie("wiki");
  }

  cgi_put_http_header();
}

static void wiki_draw_head(const struct wiki_args *args) {
  printf("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" "
         "\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n");
  printf("<html xmlns=\"http://www.w3.org/1999/xhtml\">\n");
  printf("<head>\n");
#ifdef WIKI_MATH
  printf("<script type=\"text/javascript\" id=\"MathJax-script\" async src=\"https://cdn.jsdelivr.net/npm/mathjax@4/tex-mml-chtml.js\"></script>\n");
#endif
  printf("<meta http-equiv=\"Content-Type\" content=\"text/html; "
         "charset=UTF-8\" />\n");
#ifdef WIKI_CSS
  printf("<link rel=\"stylesheet\" type=\"text/css\" href=\"%s\" />\n",
         WIKI_CSS);
#endif
#ifdef WIKI_ICON
  printf("<link rel=\"shortcut icon\" href=\"%s\" />\n", WIKI_ICON);
#endif

  printf("<title>");
#if 0
	if (search) {
		printf("Search - ");
	}
	if (upload) {
		printf("Upload - ");
	}
	if (edit) {
		printf("Edit - ");
	}
#else
  switch (args->command) {
  default: {
  } break;
  case WIKI_COMMAND_EDIT:
  case WIKI_COMMAND_PREVIEW: {
    printf("Edit - ");
  } break;
  }
#endif
  cgi_puts_html_escaped(args->page);
  if (WIKI_NAME != NULL && strlen(WIKI_NAME) > 0) {
    printf(" - %s", WIKI_NAME);
  }
  printf("</title>\n");

  printf("</head>\n");
  printf("<body>\n");
}

static void wiki_draw_tail() {
  printf("</body>\n");
  printf("</html>\n");
}

static void wiki_draw_navigator(const struct wiki_args *args) {
#ifdef WIKI_LOGO
  printf("<a href=\"%s\"><img src=\"%s\" border=\"0\" alt=\"%s\" /></a>\n",
         script_name, WIKI_LOGO, WIKI_NAME);
#else
  printf("<a href=\"%s\">%s</a>\n", script_name, WIKI_NAME);
#endif

#if 0
	if (args->frame) {
		/* printf("<a href=\"%s?%s&frame=false\">NoFrames</a>\n", script_name, cgi_url_encode(tt)); */
		printf("<a href=\"%s?", script_name);
		cgi_puts_url_encoded(args->page);
		printf("&frame=false\">NoFrames</a>\n");
	} else {
		/* printf("<a href=\"%s?%s&frame=true\">Frames</a>\n", script_name, cgi_url_encode(tt)); */
		printf("<a href=\"%s?", script_name);
		cgi_puts_url_encoded(args->page);
		printf("&frame=true\">Frames</a>\n");
	}
#endif

#if 0
	/* printf("<a href=\"%s?%s&search=true\">Search</a>\n", script_name, cgi_url_encode(tt)); */
	printf("<a href=\"%s?", script_name);
	cgi_puts_url_encoded(args->page);
	printf("&search=true\">Search</a>\n");
#endif

  /* printf("<a href=\"%s?%s&download=true\">Download</a>\n", script_name,
   * cgi_url_encode(tt)); */

#if 0
	if (args->upload) {
		/* printf("<a href=\"%s?%s\">View</a>\n", script_name, cgi_url_encode(tt)); */
		printf("<a href=\"%s?", script_name);
		cgi_puts_url_encoded(args->page);
		printf("\">View</a>\n");
	} else {
		/* printf("<a href=\"%s?%s&upload=true\">Upload</a>\n", script_name, cgi_url_encode(tt)); */
		printf("<a href=\"%s?", script_name);
		cgi_puts_url_encoded(args->page);
		printf("&upload=true\">Upload</a>\n");
	}
#endif

  if (args->loggedin) {
    cgi_puts_html_escaped(args->account);
    printf("\n");
  }

  if (!args->loggedin) {
    printf("<a href=\"%s?", script_name);
    cgi_puts_url_encoded(args->page);
    printf("&amp;command=login_form\">Login</a>\n");
  } else {
    printf("<a href=\"%s?", script_name);
    cgi_puts_url_encoded(args->page);
    printf("&amp;command=logout\">Logout</a>\n");
  }

  if (args->command == WIKI_COMMAND_EDIT ||
      args->command == WIKI_COMMAND_PREVIEW) {
    /* printf("<a href=\"%s?%s\">View</a>\n", script_name, cgi_url_encode(tt));
     */
    printf("<a href=\"%s?", script_name);
    cgi_puts_url_encoded(args->page);
    printf("\">View</a>\n");
  } else {
    /* printf("<a href=\"%s?%s&command=edit\">Edit</a>\n", script_name,
     * cgi_url_encode(tt)); */
    printf("<a href=\"%s?", script_name);
    cgi_puts_url_encoded(args->page);
    printf("&amp;command=edit\">Edit</a>\n");
  }
}

static void wiki_draw_create_account_form(const struct wiki_args *args) {
  printf("<div><b>Create new account</b></div>\n");
  printf("<form action=\"%s\" method=\"post\">\n", script_name);
  printf("<input type=\"hidden\" name=\"page\" value=\"");
  cgi_puts_html_escaped(args->page);
  printf("\" />\n");
  printf("Account: <input type=\"text\" name=\"account\" value=\"");
  cgi_puts_html_escaped(args->account);
  printf("\" /><br />\n");
  printf("Password: <input type=\"password\" name=\"password\" /><br />\n");
  printf("Confirm: <input type=\"password\" name=\"confirm\" /><br />\n");
  printf("<input type=\"submit\" name=\"create_account\" value=\"Create "
         "account\" /><br />\n");
  printf("</form>\n");
}

static void wiki_draw_login_form(const struct wiki_args *args) {
  printf("<div><b>Login</b></div>\n");
  printf("<form action=\"%s\" method=\"post\">\n", script_name);
  printf("<input type=\"hidden\" name=\"page\" value=\"");
  cgi_puts_html_escaped(args->page);
  printf("\" />\n");
  printf("Account: <input type=\"text\" name=\"account\" value=\"");
  cgi_puts_html_escaped(args->account);
  printf("\" /><br />\n");
  printf("Password: <input type=\"password\" name=\"password\" /><br />\n");
  printf("<input type=\"submit\" name=\"login\" value=\"Login\" /><br />\n");
  printf("</form>\n");
}

static void wiki_draw_editor(const char *title, const char *comment,
                             int preview, int loggedin) {
  if (title == NULL) {
    title = "";
  }
  if (comment == NULL) {
    comment = "";
  }

  printf("<form action=\"%s\" method=\"post\">\n", script_name);
  printf("<input type=\"hidden\" name=\"page\" value=\"");
  cgi_puts_html_escaped(title);
  printf("\" />\n");
#if 0
	if (!preview) {
		printf("<textarea name=\"text\" rows=\"%d\" cols=\"%d\">\n", WIKI_ROWS, WIKI_COLS);
	} else {
		printf("<textarea name=\"text\" rows=\"%d\" cols=\"%d\" readonly>\n", WIKI_ROWS, WIKI_COLS);
	}
#else
  printf("<textarea name=\"text\" rows=\"%d\" cols=\"%d\">\n", WIKI_ROWS,
         WIKI_COLS);
#endif
  cgi_puts_html_escaped(comment);
  printf("</textarea><br />\n");
  printf("<input type=\"submit\" name=\"preview\" value=\"Preview\" />\n");
  if (preview) {
    if (!loggedin) {
      printf("<br />\n");
      printf("Account: <input type=\"text\" name=\"account\" /><br />\n");
      printf("Password: <input type=\"password\" name=\"password\" /><br />\n");
    }
    printf("<input type=\"submit\" name=\"save\" value=\"Save\" />\n");
  }
  printf("<br />\n");
  printf("</form>\n");
}

static void wiki_draw_title(const char *title) {
  fprintf(stdout, "<h2><a href=\"%s?", script_name);
  cgi_puts_url_encoded(title);
  fputs("&amp;command=content_search&amp;keyword=", stdout);
  cgi_puts_url_encoded(title);
  fputs("\">", stdout);
  cgi_puts_html_escaped(title);
  fputs("</a></h2>\n", stdout);
}

static void wiki_draw_text(const char *text) {
  if (text == NULL) {
    return;
  }

  wiki_format(text, stdout);
}

#if 0
static void wiki_draw_search_form(const char *title, const char *keyword)
{
	/* null filter */
	if (title == NULL) title = "";
	if (keyword == NULL) keyword = title;

	/* by title */
	printf("Search by title:\n");
	printf("<form action=\"%s\" method=\"post\">\n", script_name);
	printf("<input type=\"hidden\" name=\"page\" value=\"%s\">\n", title);
	printf("<input type=\"hidden\" name=\"search\" value=\"true\">\n");
	printf("<input type=\"text\" name=\"keyword\" value=\"%s\">\n", keyword);
	printf("<input type=\"submit\" name=\"do_title_search\" value=\"Search\">\n");
	printf("</form>\n");

	/* by content */
	printf("Search by content:\n");
	printf("<form action=\"%s\" method=\"post\">\n", script_name);
	printf("<input type=\"hidden\" name=\"page\" value=\"%s\">\n", title);
	printf("<input type=\"hidden\" name=\"search\" value=\"true\">\n");
	printf("<input type=\"text\" name=\"keyword\" value=\"%s\">\n", keyword);
	printf("<input type=\"submit\" name=\"do_content_search\" value=\"Search\">\n");
	printf("</form>\n");

	/* Separator */
	/*printf("<hr />\n");*/
}
#endif

#ifdef WIKI_DEBUG
static double wiki_getmicrotime(void) { return 0; /* TODO */ }
#endif

void wiki_draw(const char *text, const struct wiki_args *args) {
#ifdef WIKI_DEBUG
  double time_start, time_end;
  double time;
#endif

  script_name = getenv("SCRIPT_NAME");
  if (script_name == NULL) {
    script_name = ".";
  }

  wiki_draw_http_header(args);

  wiki_draw_head(args);

  printf("\n");

  if (args->command == WIKI_COMMAND_LOGIN_FORM) {
    wiki_draw_login_form(args);
    printf("\n");
    wiki_draw_create_account_form(args);
    printf("\n");
  } else if (args->command == WIKI_COMMAND_EDIT) {
    wiki_draw_editor(args->page, text, 0, args->loggedin);
    printf("\n");
  } else if (args->command == WIKI_COMMAND_PREVIEW) {
    wiki_draw_editor(args->page, args->text, 1, args->loggedin);
    printf("\n");
  }

  printf("<div>\n");
  wiki_draw_navigator(args);
  printf("</div>\n");

  printf("\n");

  wiki_draw_title(args->page);

  printf("\n");

#ifdef WIKI_DEBUG
  time_start = wiki_getmicrotime();
#endif

  if (args->command == WIKI_COMMAND_PREVIEW) {
    wiki_draw_text(args->text);
  } else {
    wiki_draw_text(text);
  }

  printf("\n");

#ifdef WIKI_DEBUG
  time_end = wiki_getmicrotime();
  time = time_end - time_start;

  printf("<hr />\n");
  printf("<p>\n");
  printf("debug print.<br />\n");
  printf("elapsed time for formatting the text: %f sec<br />\n", time);
  printf("</p>\n");

  printf("\n");
#endif

#if 0
	printf("<pre>\n");
	printf("account: %s\n", args->account);
	printf("session: %s\n", args->session);
	printf("</pre>\n");
#endif

  wiki_draw_tail();
}
