#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cgi/input.h"
#include "cgi/output.h"

static struct cgi_arg *cgi_arg_create(void) {
  struct cgi_arg *this;

  this = malloc(sizeof(struct cgi_arg));
  if (this == NULL) {
    cgi_die("can not allocate cgi_arg");
  }

  this->next = NULL;

  this->name = NULL;
  this->value = NULL;

  return this;
}

static void cgi_arg_destroy(struct cgi_arg *this) {
  if (this == NULL) {
    return;
  }
  cgi_arg_destroy(this->next);

  free(this);
}

static const char *cgi_arg_get_value(const struct cgi_arg *this,
                                     const char *name) {
  if (this == NULL) {
    return NULL;
  }

  if (strcmp(this->name, name) == 0) {
    return this->value;
  }

  return cgi_arg_get_value(this->next, name);
}

static struct cgi_file *cgi_file_create(void) {
  struct cgi_file *this;

  this = malloc(sizeof(struct cgi_file));
  if (this == NULL) {
    cgi_die("can not allocate cgi_file");
  }

  this->next = NULL;

  this->name = NULL;
  this->filename = NULL;
  this->type = NULL;
  this->data = NULL;
  this->size = -1;

  return this;
}

static void cgi_file_destroy(struct cgi_file *this) {
  if (this == NULL) {
    return;
  }
  cgi_file_destroy(this->next);

  free(this);
}

static const struct cgi_file *cgi_file_find(const struct cgi_file *this,
                                            const char *name) {
  if (this == NULL) {
    return NULL;
  }

  if (strcmp(this->name, name) == 0) {
    return this;
  }

  return cgi_file_find(this->next, name);
}

struct cgi_input *cgi_input_create(void) {
  struct cgi_input *this;

  this = malloc(sizeof(struct cgi_input));
  if (this == NULL) {
    cgi_die("can not allocate cgi_input");
  }

  this->query_data = NULL;
  this->post_data = NULL;
  this->cookie_data = NULL;

  this->query = cgi_arg_create();
  this->post = cgi_arg_create();
  this->file = cgi_file_create();
  this->cookie = cgi_arg_create();
  this->compound_cookie = cgi_arg_create();

  return this;
}

void cgi_input_destroy(struct cgi_input *this) {
  if (this == NULL) {
    return;
  }

  free(this->query_data);
  free(this->post_data);
  free(this->cookie_data);

  cgi_arg_destroy(this->query);
  cgi_arg_destroy(this->post);
  cgi_file_destroy(this->file);
  cgi_arg_destroy(this->cookie);
  cgi_arg_destroy(this->compound_cookie);

  free(this);
}

static const char *url_decode(const char *src, char *dst) {
  if (*src == '+') {
    *dst = ' ';
    src++;
  } else if (*src == '%') {
    char buf[3];
    int c;

    if (*(src + 1) == '\0' || *(src + 2) == '\0') {
      cgi_die("url_decode: unexpected termination");
    }

    buf[0] = *(src + 1);
    buf[1] = *(src + 2);
    buf[2] = '\0';
    if (sscanf(buf, "%02x", &c) != 1) { /* XXX do yourself for performance */
      cgi_die("url_decode failed");
    }
    *dst = c;

    src += 3;
  } else {
    *dst = *src;
    src++;
  }

  return src;
}

static void split_query(const char *src, char *dst, struct cgi_arg *arg) {
  char c;

  while (*src != '\0') {
    arg->next = cgi_arg_create();
    arg = arg->next;
    arg->name = dst;

    /* parse key */
    for (;;) {
      if (*src == '=' || *src == '&' || *src == '\0') {
        c = *src;
        *dst = '\0';
        dst++;
        break;
      } else {
        src = url_decode(src, dst);
        dst++;
      }
    }

    /* parse value */
    if (c != '=') {
      arg->value = NULL;
      if (c != '\0') {
        src++;
      }
    } else {
      src++;

      arg->value = dst;
      for (;;) {
        if (*src == '&' || *src == '\0') {
          if (*src != '\0') {
            src++;
          }

          *dst = '\0';
          dst++;

          break;
        } else {
          src = url_decode(src, dst);
          dst++;
        }
      }
    }
  }
}

static void split_cookie(const char *src, char *dst, struct cgi_arg *arg) {
  char c;

  while (*src != '\0') {
    arg->next = cgi_arg_create();
    arg = arg->next;
    arg->name = dst;

    /* parse key */
    for (;;) {
      if (*src == '=' || *src == ';' || *src == '\0') {
        c = *src;
        *dst = '\0';
        dst++;
        if (*src == ';' && *(src + 1) == ' ') {
          src++;
        }
        break;
      } else {
        src = url_decode(src, dst);
        dst++;
      }
    }

    /* parse value */
    if (c != '=') {
      arg->value = NULL;
      if (c != '\0') {
        src++;
        if (*src == ' ') {
          src++;
        }
      }
    } else {
      src++;

      arg->value = dst;
      for (;;) {
        if (*src == ';' || *src == '\0') {
          if (*src != '\0') {
            src++;
            if (*src == ' ') {
              src++;
            }
          }

          *dst = '\0';
          dst++;

          break;
        } else {
          src = url_decode(src, dst);
          dst++;
        }
      }
    }
  }
}

static char *find(const char *s, int slen, const char *t, int tlen) {
  int i;

  for (i = 0; i < slen - tlen; i++) {
    int match;
    int j;

    match = 1;
    for (j = 0; j < tlen; j++) {
      if (*(s + i + j) != *(t + i + j)) {
        match = 0;
        break;
      }
    }

    if (match) {
      return (char *)(s + i);
    }
  }

  return NULL;
}

static void split_multipart(char *s, int length, const char *boundary,
                            struct cgi_arg *arg, struct cgi_file *file) {
  char *origin;
  int boundary_length;
  char *b;

  origin = s;
  boundary_length = strlen(boundary);

  b = find(s, length - (s - origin), boundary, boundary_length);
  if (b == NULL) {
    cgi_die("multipart format error");
  }
  s = b + boundary_length;

  for (;;) {
    char *head, *begin, *end;
    char *name, *e;

    if (s - origin + 2 > length) {
      cgi_die("multipart format error");
    }
    if (*s == '-' && *(s + 1) == '-') {
      break;
    }
    if (*s != '\r' || *(s + 1) != '\n') {
      cgi_die("multipart format error");
    }
    head = s + 2;

    begin = find(head, length - (head - origin), "\r\n\r\n", 4);
    if (begin == NULL) {
      cgi_die("multipart format error");
    }

    end = find(begin, length - (begin - origin), boundary, boundary_length);
    if (end == NULL) {
      cgi_die("multipart format error");
    }
    if (*(end - 4) != '\r' || *(end - 3) != '\n' || *(end - 2) != '-' ||
        *(end - 1) != '-') {
      cgi_die("multipart format error");
    }
    end -= 4;

    if (head - origin + strlen("Content-Disposition: form-data; name=\"") >
        length) {
      cgi_die("multipart format error");
    }
    if (strncmp(head, "Content-Disposition: form-data; name=\"",
                strlen("Content-Disposition: form-data; name=\"")) != 0) {
      cgi_die("multipart format error");
    }
    name = head + strlen("Content-Disposition: form-data; name=\"");
    for (e = name; *e != '"' && e - origin < length; e++)
      ;
    *e = '\0';

    if (e - origin + 2 > length) {
      cgi_die("multipart format error");
    }
    if (*(e + 1) == '\r' && *(e + 2) == '\n') {
      arg->next = cgi_arg_create();
      arg = arg->next;

      arg->name = name;

      *end = '\0';
      if (e - origin + 5 > length) {
        cgi_die("multipart format error");
      }
      arg->value = e + 5;
    } else {
      file->next = cgi_file_create();
      file = file->next;

      file->name = name;

      e++;
      if (e - origin + strlen("; filename=\"") > length) {
        cgi_die("multipart format error");
      }
      if (strncmp(e, "; filename=\"", strlen("; filename=\"")) != 0) {
        cgi_die("multipart format error");
      }
      e += strlen("; filename=\"");
      file->filename = e;
      for (; *e != '"' && e - origin < length; e++)
        ;
      *e = '\0';

      e++;
      if (e - origin + strlen("\r\nContent-Type: ") > length) {
        cgi_die("multipart format error");
      }
      if (strncmp(e, "\r\nContent-Type: ", strlen("\r\nContent-Type: ")) != 0) {
        cgi_die("multipart format error");
      }
      e += strlen("\r\nContent-Type: ");
      file->type = e;
      for (; *e != '\r' && e - origin < length; e++)
        ;
      *e = '\0';

      e++;
      if (e - origin + 3 > length) {
        cgi_die("multipart format error");
      }
      if (strncmp(e, "\n\r\n", 3) != 0) {
        cgi_die("multipart format error");
      }
      e += 3;
      file->data = e;

      file->size = end - e;
    }

    s = end + 4 + boundary_length;
  }
}

static void read_query(struct cgi_input *this) {
  char *query_string;

  query_string = getenv("QUERY_STRING");
  if (query_string == NULL) {
    return;
  }

  this->query_data = malloc(strlen(query_string) + 1);
  if (this->query_data == NULL) {
    cgi_die("can not allocate query_data");
  }

  split_query(query_string, this->query_data, this->query);
}

static void read_post(struct cgi_input *input) {
  const char *request_method;
  const char *content_type;
  int multipart;
  const char *content_length;
  int length;

  request_method = getenv("REQUEST_METHOD");
  if (request_method == NULL) {
    cgi_die("REQUEST_METHOD is NULL");
  }
  if (strcmp("POST", request_method) != 0) {
    return; /* may be GET or other, it's successful return */
  }

  content_type = getenv("CONTENT_TYPE");
  if (content_type == NULL) {
    cgi_die("CONTENT_TYPE is NULL");
  }
  if (strcmp("application/x-www-form-urlencoded", content_type) == 0) {
    multipart = 0;
  } else if (strncmp("multipart/form-data;", content_type,
                     strlen("multipart/form-data;")) == 0) {
    multipart = 1;
  } else {
    cgi_die("unknown CONTENT_TYPE");
  }

  content_length = getenv("CONTENT_LENGTH");
  if (content_length == NULL) {
    cgi_die("CONTENT_LENGTH is NULL");
  }
  if (sscanf(content_length, "%d", &length) != 1) {
    cgi_die("invalid CONTENT_LENGTH");
  }
  if (length < 0) {
    cgi_die("invalid CONTENT_LENGTH");
  }

  input->post_data = malloc(length + 1);
  if (input->post_data == NULL) {
    cgi_die("can not allocate post_data");
  }

  if (fread(input->post_data, 1, length, stdin) != length) {
    cgi_die("failed reading from stdin");
  }
  *(input->post_data + length) = '\0';

  if (!multipart) {
    split_query(input->post_data, input->post_data, input->post);
  } else {
    const char *boundary;

    boundary = strstr(content_type, "boundary=");
    if (boundary == NULL) {
      cgi_die("can not find a boundary");
    }
    boundary += strlen("boundary=");

    split_multipart(input->post_data, length, boundary, input->post,
                    input->file);
  }
}

static void read_cookie(struct cgi_input *this) {
  char *http_cookie;

  http_cookie = getenv("HTTP_COOKIE");
  if (http_cookie == NULL) {
    return;
  }

  this->cookie_data = malloc(strlen(http_cookie) + 1);
  if (this->cookie_data == NULL) {
    cgi_die("can not allocate cookie_data");
  }

  split_cookie(http_cookie, this->cookie_data, this->cookie);
}

void cgi_input_read(struct cgi_input *this) {
  read_query(this);
  read_post(this);
  read_cookie(this);
}

void cgi_input_read_compound(struct cgi_input *this) {
  struct cgi_arg *arg;

  read_query(this);
  read_post(this);
  read_cookie(this);

  for (arg = this->cookie->next; arg != NULL; arg = arg->next) {
    if (arg->value != NULL) {
      struct cgi_arg *last;

      for (last = this->compound_cookie; last->next != NULL; last = last->next)
        ;

      split_query(arg->value, (char *)arg->value, last);
    }
  }
}

const char *cgi_input_get_value(struct cgi_input *this, const char *name) {
  const char *value;

  value = cgi_arg_get_value(this->query->next, name);

  if (value == NULL) {
    value = cgi_arg_get_value(this->post->next, name);
  }

  if (value == NULL) {
    value = cgi_arg_get_value(this->cookie->next, name);
  }

  return value;
}

const char *cgi_input_get_value_compound(struct cgi_input *this,
                                         const char *name) {
  const char *value;

  value = cgi_arg_get_value(this->query->next, name);

  if (value == NULL) {
    value = cgi_arg_get_value(this->post->next, name);
  }

  if (value == NULL) {
    value = cgi_arg_get_value(this->compound_cookie->next, name);
  }

  return value;
}

const struct cgi_file *cgi_input_get_file(struct cgi_input *this,
                                          const char *name) {
  return cgi_file_find(this->file->next, name);
}
