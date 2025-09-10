#include <stdio.h>
#include <stdlib.h> /* getenv, XXX which will be not required later */
#include <string.h>

#include "cgi/cgi.h"

#include "wiki/format.h"

static FILE *out;

enum {
  WIKI_BLOCK_NONE,
  WIKI_BLOCK_P,
  WIKI_BLOCK_PRE,
  WIKI_BLOCK_UL,
  WIKI_BLOCK_DL,
};

enum {
  WIKI_MATH_NONE,
  WIKI_MATH_PARENTHES,
  WIKI_MATH_BRACKET,
  WIKI_MATH_DOLLARS,
};

void unify_line_breaks(char *s) {
  char *d;

  d = s;
  while (*s != '\0') {
    if (*s == '\r') {
      if (*(s + 1) == '\n') {
        *d = '\n';
        s += 2;
        d++;
      } else {
        *d = '\n';
        s++;
        d++;
      }
    } else {
      *d = *s;
      s++;
      d++;
    }
  }
  *d = '\0';
}

static int wiki_format_open(int block, int next) {
  if (block == next) {
    return block;
  }

  switch (next) {
  default: {
    cgi_die("invalid wiki_format block type\n");
  }
  case WIKI_BLOCK_NONE: {
    /*printf("<!-- none -->\n");*/
  } break;
  case WIKI_BLOCK_P: {
    printf("<p>\n");
  } break;
  case WIKI_BLOCK_PRE: {
    printf("<pre>\n");
  } break;
  case WIKI_BLOCK_UL: {
    /* <ul> will be printed in set_ul_level() */
  } break;
  case WIKI_BLOCK_DL: {
    printf("<dl>\n");
  } break;
  }

  return next;
}

static int wiki_format_close(int block, int except) {
  if (block == except)
    return block;

  switch (block) {
  default: {
    cgi_die("invalid wiki_format block type\n");
  }
  case WIKI_BLOCK_NONE: {
    /*printf("<!-- /none -->\n");*/
  } break;
  case WIKI_BLOCK_P: {
    printf("</p>\n");
  } break;
  case WIKI_BLOCK_PRE: {
    printf("</pre>\n");
  } break;
  case WIKI_BLOCK_UL: {
    /* </ul> will be printed in set_ul_level() */
  } break;
  case WIKI_BLOCK_DL: {
    printf("</dl>\n");
  } break;
  }

  return WIKI_BLOCK_NONE;
}

static int wiki_format_set_ul_level(int lv, int new_lv) {
  if (new_lv == lv) {
    return lv;
  }

  while (lv < new_lv) {
    int i;

    lv++;
    for (i = 0; i < lv - 1; i++) {
      fputc(' ', stdout);
    }
    printf("<ul>\n");
  }
  while (lv > new_lv) {
    int i;

    lv--;
    for (i = 0; i < lv; i++) {
      fputc(' ', stdout);
    }
    printf("</ul>\n");
  }

  return new_lv;
}

static int attribute_stack[2];
static int attribute_depth;

static void attribute_init(void) { attribute_depth = 0; }

/* *** attribute stack *** */

#define WIKI_ATTRIBUTE_STACK_SIZE 2

#define WIKI_ATTRIBUTE_STRONG 1
#define WIKI_ATTRIBUTE_EM 2

static void attribute_push(int attribute) {
  if (attribute_depth >= WIKI_ATTRIBUTE_STACK_SIZE) {
    cgi_die("attribute_stack overflow");
  }

  switch (attribute) {
  default: {
    cgi_die("attribute_push: invalid attribute type");
  }
  case WIKI_ATTRIBUTE_STRONG: {
    fputs("<strong>", stdout);
  } break;
  case WIKI_ATTRIBUTE_EM: {
    fputs("<em>", stdout);
  } break;
  }

  attribute_stack[attribute_depth++] = attribute;
}

static int attribute_pop(void) {
  int attribute;

  if (attribute_depth <= 0)
    cgi_die("attribute_stack underflow");

  attribute = attribute_stack[--attribute_depth];

  switch (attribute) {
  default: {
    cgi_die("attribute_pop: invalid attribute type");
  }
  case WIKI_ATTRIBUTE_STRONG: {
    fputs("</strong>", stdout);
  } break;
  case WIKI_ATTRIBUTE_EM: {
    fputs("</em>", stdout);
  } break;
  }

  return attribute;
}

static int attribute_exist(int attribute) {
  int i;

  for (i = attribute_depth - 1; i >= 0; i--) {
    if (attribute_stack[i] == attribute) {
      return 1;
    }
  }

  return 0;
}

/* *** main formatting routine *** */

enum {
  WN_INIT,
  WN_NO,
  WN_FIRST_UPPER,
  WN_FIRST_LOWER,
  WN_SECOND_UPPER,
  WN_SECOND_LOWER,
};

static const char *script_name; /* XXX should be in module's scope, initialized
                                   at main() or somewhere else. */

static void wiki_format_draw_wikiname(const char *start, const char *end) {
  const char *s;

  /* example: <a href="wiki.cgi?WikiName">WikiName</a> */
  /* XXX some more args will be included */
  printf("<a href=\"%s?", script_name);
  for (s = start; s < end; s++) {
    cgi_putc_url_encoded(*s);
  }
  fputs("\">", stdout);
  for (s = start; s < end; s++) {
    cgi_putc_html_escaped(*s);
  }
  fputs("</a>", stdout);
}

void wiki_format(const char *comment, FILE *output_stream) {
  int block;
  int lv;
  int seek_dd;
  int math;

  const char *outend;
  int wn;

  out = output_stream;

  script_name = getenv("SCRIPT_NAME");
  if (script_name == NULL) {
    script_name = ".";
  }

  lv = 0;
  seek_dd = 0;
  math = WIKI_MATH_NONE;

  attribute_init();
  block = wiki_format_open(~WIKI_BLOCK_NONE, WIKI_BLOCK_NONE);
  for (;;) {

    if (math == WIKI_MATH_NONE) {
      /* decide the block type */
      if (*comment == '\n' || *comment == '\0') {
        lv = wiki_format_set_ul_level(lv, 0);
        block = wiki_format_close(block, WIKI_BLOCK_NONE);
        if (*comment == '\0') {
          break; /* exit the loop */
        } else {
          comment++;
        }
        continue;
      } else if (strncmp("----", comment, 4) == 0) { /* <hr /> */
        lv = wiki_format_set_ul_level(lv, 0);
        block = wiki_format_close(block, WIKI_BLOCK_P);
        printf("<hr />\n");
        while (*comment == '-') {
          comment++;
        }
        if (*comment == '\n') {
          comment++;
        }
        continue;
      } else if (*comment == ' ') { /* <pre> */
        lv = wiki_format_set_ul_level(lv, 0);
        block = wiki_format_close(block, WIKI_BLOCK_PRE);
        block = wiki_format_open(block, WIKI_BLOCK_PRE);
        /*fputc(*comment, stdout);*/
        comment++;
      } else if (*comment == '*') { /* <ul> */
        const char *end;
        int i;

        block = wiki_format_close(block, WIKI_BLOCK_UL);
        for (end = comment; *end == '*'; end++)
          ;
        lv = wiki_format_set_ul_level(lv, end - comment);
        block = wiki_format_open(block, WIKI_BLOCK_UL);
        for (i = 0; i < lv - 1; i++) {
          fputc(' ', stdout);
        }
        printf("<li>");
        comment = end;
      } else if (*comment == ':') { /* <dl> */
        lv = wiki_format_set_ul_level(lv, 0);
        block = wiki_format_close(block, WIKI_BLOCK_DL);
        block = wiki_format_open(block, WIKI_BLOCK_DL);
        printf("<dt>");
        seek_dd = 1;
        comment++;
      } else { /* an implicit <p> begins */
        lv = wiki_format_set_ul_level(lv, 0);
        block = wiki_format_close(block, WIKI_BLOCK_P);
        block = wiki_format_open(block, WIKI_BLOCK_P);
      }
    } else {
      if (*comment == '\0') {
        lv = wiki_format_set_ul_level(lv, 0);
        block = wiki_format_close(block, WIKI_BLOCK_NONE);
        break;
      }
    }

    /* draw the line */
    outend = comment;
    wn = WN_INIT;
    for (;;) {
      if (*comment == '\n' || *comment == '\0') { /* terminate the line */
        if (math == WIKI_MATH_NONE && block != WIKI_BLOCK_PRE &&
            (wn == WN_SECOND_UPPER || wn == WN_SECOND_LOWER)) {
          wiki_format_draw_wikiname(outend, comment);
        } else {
          for (; outend < comment; outend++) {
            cgi_putc_html_escaped(*outend);
          }
        }
        if (math == WIKI_MATH_NONE) {
          while (attribute_depth > 0) {
            attribute_pop();
          }
          if (block == WIKI_BLOCK_UL) {
            printf("</li>");
          } else if (block == WIKI_BLOCK_DL) {
            printf("</dd>");
          } else {
            if (block != WIKI_BLOCK_PRE) {
              printf("<br />");
            }
          }
        }
        fputc('\n', stdout);
        if (*comment == '\0') {
        } else {
          comment++;
        }
        break;
      } else if (math == WIKI_MATH_NONE && block == WIKI_BLOCK_DL && seek_dd &&
                 *comment == ':') { /* <dd> found */
        if (wn == WN_SECOND_UPPER || wn == WN_SECOND_LOWER) {
          wiki_format_draw_wikiname(outend, comment);
        } else {
          for (; outend < comment; outend++) {
            cgi_putc_html_escaped(*outend);
          }
        }
        wn = WN_INIT;

        printf("</dt>\n<dd>");
        seek_dd = 0;
        comment++;
        outend = comment; /* skip the colon ':' */
      }

      /* https: phrase */
      /* XXX WikiName may be overridden */
      else if (math == WIKI_MATH_NONE && block != WIKI_BLOCK_PRE &&
               *comment == 'h' && strncmp("https:", comment, 6) == 0) {
        const char *end;
        const char *s;

        for (; outend < comment; outend++) {
          cgi_putc_html_escaped(*outend);
        }
        wn = WN_INIT;

        end = comment + 5;
        while (*end) {
          if (*end == ' ' || *end == '\t' || *end == '\n') {
            break;
          }
          end++;
        }
        printf("<a href=\"");
        for (s = comment; s < end; s++) {
          cgi_putc_html_escaped(*s);
        }
        printf("\">");
        for (s = comment; s < end; s++) {
          cgi_putc_html_escaped(*s);
        }
        printf("</a>");

        comment = end;
        outend = comment;
      }

      /* user@domain phrase */
      /* XXX WikiName may be overridden */
      else if (math == WIKI_MATH_NONE && block != WIKI_BLOCK_PRE &&
               *comment == '@') { /* XXX user@ (domain omitted) */
        const char *end;
        const char *s;

        end = comment + 1;
        while (*end) {
          if (*end == ' ' || *end == '\t' || *end == '\n') {
            break;
          }
          end++;
        }
        /* XXX more email specific escapes should be used */
        printf("<a href=\"mailto:");
        for (s = outend; s < end; s++) {
          cgi_putc_html_escaped(*s);
        }
        printf("\">");
        for (s = outend; s < end; s++) {
          cgi_putc_html_escaped(*s);
        }
        printf("</a>");

        wn = WN_INIT;
        comment = end;
        outend = comment;
      }

      /* XXX there may be some misunderstandings around multiple primes.. */
      /* '''''' */
      else if (math == WIKI_MATH_NONE && block != WIKI_BLOCK_PRE &&
               *comment == '\'' && strncmp("''''''", comment, 6) == 0) {
        if (wn == WN_SECOND_UPPER || wn == WN_SECOND_LOWER) {
          wiki_format_draw_wikiname(outend, comment);
        } else {
          for (; outend < comment; outend++) {
            cgi_putc_html_escaped(*outend);
          }
        }
        wn = WN_INIT;

        comment += 6;
        outend = comment;
      }
      /* TODO ''''' <strong><em> */
      else if (math == WIKI_MATH_NONE && block != WIKI_BLOCK_PRE &&
               *comment == '\'' && strncmp("'''''", comment, 5) == 0) {
        if (wn == WN_SECOND_UPPER || wn == WN_SECOND_LOWER) {
          wiki_format_draw_wikiname(outend, comment);
        } else {
          for (; outend < comment; outend++) {
            cgi_putc_html_escaped(*outend);
          }
        }
        wn = WN_INIT;
        if (!attribute_exist(WIKI_ATTRIBUTE_EM) &&
            !attribute_exist(WIKI_ATTRIBUTE_STRONG)) {
          attribute_push(WIKI_ATTRIBUTE_EM);
          attribute_push(WIKI_ATTRIBUTE_STRONG);
        } else if (!attribute_exist(WIKI_ATTRIBUTE_EM)) {
          while (attribute_pop() != WIKI_ATTRIBUTE_STRONG)
            ;
          attribute_push(WIKI_ATTRIBUTE_EM);
        } else if (!attribute_exist(WIKI_ATTRIBUTE_STRONG)) {
          while (attribute_pop() != WIKI_ATTRIBUTE_EM)
            ;
          attribute_push(WIKI_ATTRIBUTE_STRONG);
        } else {
          /* XXX ambiguity exists */
          while (attribute_pop() != WIKI_ATTRIBUTE_STRONG)
            ;
          while (attribute_pop() != WIKI_ATTRIBUTE_EM)
            ;
        }

        comment += 5;
        outend = comment;
      }
      /* ''' <strong> */
      else if (math == WIKI_MATH_NONE && block != WIKI_BLOCK_PRE &&
               *comment == '\'' && strncmp("'''", comment, 3) == 0) {
        if (wn == WN_SECOND_UPPER || wn == WN_SECOND_LOWER) {
          wiki_format_draw_wikiname(outend, comment);
        } else {
          for (; outend < comment; outend++) {
            cgi_putc_html_escaped(*outend);
          }
        }
        wn = WN_INIT;
        if (!attribute_exist(WIKI_ATTRIBUTE_STRONG)) {
          attribute_push(WIKI_ATTRIBUTE_STRONG);
        } else {
          while (attribute_pop() != WIKI_ATTRIBUTE_STRONG)
            ;
        }

        comment += 3;
        outend = comment;
      }
      /* '' <em> */
      else if (math == WIKI_MATH_NONE && block != WIKI_BLOCK_PRE &&
               *comment == '\'' && strncmp("''", comment, 2) == 0) {
        if (wn == WN_SECOND_UPPER || wn == WN_SECOND_LOWER) {
          wiki_format_draw_wikiname(outend, comment);
        } else {
          for (; outend < comment; outend++) {
            cgi_putc_html_escaped(*outend);
          }
        }
        wn = WN_INIT;
        if (!attribute_exist(WIKI_ATTRIBUTE_EM)) {
          attribute_push(WIKI_ATTRIBUTE_EM);
        } else {
          while (attribute_pop() != WIKI_ATTRIBUTE_EM)
            ;
        }

        comment += 2;
        outend = comment;
      }

      /* [[ ]] bracket PageName */
      else if (math == WIKI_MATH_NONE && block != WIKI_BLOCK_PRE &&
               *comment == '[' && *(comment + 1) == '[') {
        const char *start, *end;
        const char *s;

        if (wn == WN_SECOND_UPPER || wn == WN_SECOND_LOWER) {
          wiki_format_draw_wikiname(outend, comment);
        } else {
          for (; outend < comment; outend++) {
            cgi_putc_html_escaped(*outend);
          }
        }
        wn = WN_INIT;

        start = comment + 2;
        end = start;
        while (1) {
          if (!*end || *end == '\n') {
            comment = end;
            break;
          }
          if (*end == ']' && *(end + 1) == ']') {
            comment = end + 2;
            break;
          }
          end++;
        }

        /* XXX some more args will be included */
        printf("<a href=\"%s?", script_name);
        for (s = start; s < end; s++) {
          cgi_putc_url_encoded(*s);
        }
        fputs("\">", stdout);
        for (s = start; s < end; s++) {
          cgi_putc_html_escaped(*s);
        }
        fputs("</a>", stdout);

        outend = comment;
      }

      /* TODO embedded image <img> ... https:img.png etc. */
      /* TODO Wiki:WikiName InterWikiName */
      /* TODO uploaded image */

      /* math parenthes open */
      else if (math == WIKI_MATH_NONE && *comment == '\\' &&
               strncmp("\\(", comment, 2) == 0) {
        if (wn == WN_SECOND_UPPER || wn == WN_SECOND_LOWER) {
          wiki_format_draw_wikiname(outend, comment);
        } else {
          for (; outend < comment; outend++) {
            cgi_putc_html_escaped(*outend);
          }
        }
        wn = WN_INIT;

        fputs("\\(", stdout);

        comment += 2;
        outend = comment;

        math = WIKI_MATH_PARENTHES;
      }

      /* math parenthes close */
      else if (math == WIKI_MATH_PARENTHES && *comment == '\\' &&
               strncmp("\\)", comment, 2) == 0) {
        for (; outend < comment; outend++) {
          cgi_putc_html_escaped(*outend);
        }

        fputs("\\)", stdout);

        comment += 2;
        outend = comment;

        math = WIKI_MATH_NONE;
        wn = WN_INIT;
      }

      /* math bracket open */
      else if (math == WIKI_MATH_NONE && *comment == '\\' &&
               strncmp("\\[", comment, 2) == 0) {
        if (wn == WN_SECOND_UPPER || wn == WN_SECOND_LOWER) {
          wiki_format_draw_wikiname(outend, comment);
        } else {
          for (; outend < comment; outend++) {
            cgi_putc_html_escaped(*outend);
          }
        }
        wn = WN_INIT;

        fputs("\\[", stdout);

        comment += 2;
        outend = comment;

        math = WIKI_MATH_BRACKET;
      }

      /* math bracket close */
      else if (math == WIKI_MATH_BRACKET && *comment == '\\' &&
               strncmp("\\]", comment, 2) == 0) {
        for (; outend < comment; outend++) {
          cgi_putc_html_escaped(*outend);
        }

        fputs("\\]", stdout);

        comment += 2;
        outend = comment;

        math = WIKI_MATH_NONE;
        wn = WN_INIT;
      }

      /* math dollers open */
      else if (math == WIKI_MATH_NONE && *comment == '$' &&
               strncmp("$$", comment, 2) == 0) {
        if (wn == WN_SECOND_UPPER || wn == WN_SECOND_LOWER) {
          wiki_format_draw_wikiname(outend, comment);
        } else {
          for (; outend < comment; outend++) {
            cgi_putc_html_escaped(*outend);
          }
        }
        wn = WN_INIT;

        fputs("$$", stdout);

        comment += 2;
        outend = comment;

        math = WIKI_MATH_DOLLARS;
      }

      /* math dollers close */
      else if (math == WIKI_MATH_DOLLARS && *comment == '$' &&
               strncmp("$$", comment, 2) == 0) {
        for (; outend < comment; outend++) {
          cgi_putc_html_escaped(*outend);
        }

        fputs("$$", stdout);

        comment += 2;
        outend = comment;

        math = WIKI_MATH_NONE;
        wn = WN_INIT;
      }

      /* print a word */
      else if (*comment < 'A' || (*comment > 'Z' && *comment < 'a') ||
               *comment > 'z') {
        if (math == WIKI_MATH_NONE && block != WIKI_BLOCK_PRE &&
            (wn == WN_SECOND_UPPER || wn == WN_SECOND_LOWER)) {
          wiki_format_draw_wikiname(outend, comment);
        } else {
          for (; outend < comment; outend++) {
            cgi_putc_html_escaped(*outend);
          }
        }
        wn = WN_INIT;
        cgi_putc_html_escaped(*comment);
        comment++;
        outend = comment;
      }

      else {
        /* determine WikiName parsing phase */
        switch (wn) {
        default: {
          cgi_die("invalid wikiname_phase");
        }
        case WN_INIT: {
          if (*comment >= 'A' && *comment <= 'Z') {
            wn = WN_FIRST_UPPER;
          } else {
            wn = WN_NO;
          }
        } break;
        case WN_NO: {
        } break;
        case WN_FIRST_UPPER: {
          if (*comment >= 'a' && *comment <= 'z') {
            wn = WN_FIRST_LOWER;
          } else {
            wn = WN_NO;
          }
        } break;
        case WN_FIRST_LOWER: {
          if (*comment >= 'A' && *comment <= 'Z') {
            wn = WN_SECOND_UPPER;
          } else if (*comment >= 'a' && *comment <= 'z') {
            ; /* wn remains WN_FIRST_LOWER */
          } else {
            wn = WN_NO;
          }
        } break;
        case WN_SECOND_UPPER: {
          if (*comment >= 'A' && *comment <= 'Z') {
            wn = WN_NO;
          } else if (*comment >= 'a' && *comment <= 'z') {
            wn = WN_SECOND_LOWER;
          } else {
            wn = WN_NO;
          }
        } break;
        case WN_SECOND_LOWER: {
          if (*comment >= 'A' && *comment <= 'Z') {
            wn = WN_SECOND_UPPER;
          } else if (*comment >= 'a' && *comment <= 'z') {
            ; /* wn remains WN_SECOND_LOWER */
          } else {
            wn = WN_NO;
          }
        } break;
        }

        /* buffering in the space from 'outend' to 'comment' */
        comment++;
      }
    }
  }
  block = wiki_format_close(block, WIKI_BLOCK_NONE);
}
