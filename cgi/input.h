#ifndef CGI_INPUT_H
#define CGI_INPUT_H

struct cgi_arg {
  struct cgi_arg *next;

  const char *name;
  const char *value;
};

struct cgi_file {
  struct cgi_file *next;

  const char *name;
  const char *filename;
  const char *type;
  const char *data;
  int size;
};

struct cgi_input {
  char *query_data;
  char *post_data;
  char *cookie_data;

  struct cgi_arg *query;
  struct cgi_arg *post;
  struct cgi_file *file;
  struct cgi_arg *cookie;
  struct cgi_arg *compound_cookie;
};

extern struct cgi_input *cgi_input_create(void);
extern void cgi_input_destroy(struct cgi_input *this);
extern void cgi_input_read(struct cgi_input *this);
extern void cgi_input_read_compound(struct cgi_input *this);
extern const char *cgi_input_get_value(struct cgi_input *this,
                                       const char *name);
extern const char *cgi_input_get_value_compound(struct cgi_input *this,
                                                const char *name);
extern const struct cgi_file *cgi_input_get_file(struct cgi_input *this,
                                                 const char *name);

#endif
