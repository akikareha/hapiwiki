#include <stdio.h> /* NULL */
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "argon2/argon2.h"
#include "cgi/cgi.h"
#include "util/util.h"

#include "wiki/config.h"
#include "wiki/data.h"
#include "wiki/draw.h"
#include "wiki/format.h"
#include "wiki/main.h"

static struct cgi_input *input;

static struct wiki_args args;

static void wiki_init(void) {
  input = cgi_input_create();
  cgi_input_read_compound(input);

  cgi_content_type = CGI_CONTENT_TYPE_TEXT_HTML;
}

static void wiki_finish(void) { cgi_input_destroy(input); }

static void wiki_load_args(void) {
  const char *preview;
  const char *save;
  const char *create_account;
  const char *login;
  const char *command;

  { /* XXX temporal implimentation */
    const char *first_name = NULL;
    const char *first_value = NULL;

    if (input->query->next != NULL) {
      first_name = input->query->next->name;
      first_value = input->query->next->value;
    }
    if (first_name != NULL && first_value == NULL) {
      args.page = first_name;
    } else {
      args.page = cgi_input_get_value_compound(input, "page");
      if (args.page == NULL) {
        args.page = WIKI_FRONT_PAGE;
      }
    }
  }

  preview = cgi_input_get_value_compound(input, "preview");
  save = cgi_input_get_value_compound(input, "save");
  create_account = cgi_input_get_value_compound(input, "create_account");
  login = cgi_input_get_value_compound(input, "login");
  if (preview != NULL) {
    args.command = WIKI_COMMAND_PREVIEW;
  } else if (save != NULL) {
    args.command = WIKI_COMMAND_SAVE;
  } else if (create_account != NULL) {
    args.command = WIKI_COMMAND_CREATE_ACCOUNT;
  } else if (login != NULL) {
    args.command = WIKI_COMMAND_LOGIN;
  } else {
    command = cgi_input_get_value_compound(input, "command");
    if (command == NULL) {
      command = "view";
    }
    if (strcmp(command, "view") == 0) {
      args.command = WIKI_COMMAND_VIEW;
    } else if (strcmp(command, "edit") == 0) {
      args.command = WIKI_COMMAND_EDIT;
    } else if (strcmp(command, "login_form") == 0) {
      args.command = WIKI_COMMAND_LOGIN_FORM;
    } else if (strcmp(command, "logout") == 0) {
      args.command = WIKI_COMMAND_LOGOUT;
    } else {
      cgi_die("unknown command");
    }
  }

  args.text = (char *)cgi_input_get_value_compound(input, "text");
  if (args.text == NULL) {
    args.text = "";
  } else {
    unify_line_breaks(args.text);
  }

  args.account = cgi_input_get_value_compound(input, "account");
  args.password = cgi_input_get_value_compound(input, "password");
  args.confirm = cgi_input_get_value_compound(input, "confirm");
  args.session = cgi_input_get_value_compound(input, "session");
}

static int match_passwords(const char *password1, const char *password2) {
  MD5_CTX md5_ctx;
  unsigned char hash[16];
  char hash_string[2 * 16 + 1];

  MD5_Init(&md5_ctx);
  MD5_Update(&md5_ctx, (void *)password2, strlen(password2));
  MD5_Final(hash, &md5_ctx);

  binary_to_hex_string(hash, 16, hash_string);

  return strcmp(password1, hash_string) == 0;
}

#define FILTER_TMP_FILE "/tmp/post.txt"
#define FILTER_BUFFER_SIZE 8192

static char *run_filter(const char *filter, const char *text) {
  FILE *tmp;
  char *command_line;
  FILE *fp;
  char *buffer;
  int length;

  tmp = fopen(FILTER_TMP_FILE, "w");
  if (tmp == NULL) {
    cgi_die("failed to create temporal file");
  }
  fprintf(tmp, "%s", text);
  fclose(tmp);

  command_line = malloc(strlen(filter) + 1 + strlen(FILTER_TMP_FILE) + 1);
  if (command_line == NULL) {
    cgi_die("malloc");
  }
  sprintf(command_line, "%s %s", filter, FILTER_TMP_FILE);

  fp = popen(command_line, "r");
  if (fp == NULL) {
    cgi_die("failed to filter");
  }
  buffer = malloc(FILTER_BUFFER_SIZE);
  if (buffer == NULL) {
    cgi_die("malloc");
  }
  length = fread(buffer, 1, FILTER_BUFFER_SIZE - 1, fp);
  pclose(fp);
  buffer[length] = '\0';

  free(command_line);

  return buffer;
}

int main(int argc, char **argv) {
  const char *data_dir;
  const char *session;
  const char *text;

  /* init */

  umask(0007); /* XXX separate the const */

  wiki_init();
  wiki_load_args();

  data_dir = getenv("WIKI_DATA_DIR");
  if (data_dir == NULL) {
    data_dir = WIKI_DATA_DIR;
  }

  /* operate */

  if (args.command == WIKI_COMMAND_CREATE_ACCOUNT) {
    if (args.password == NULL || args.confirm == NULL) {
      cgi_die("passwords not match");
    }
    if (strcmp(args.password, args.confirm) != 0) {
      cgi_die("passwords not match");
    }
    wiki_data_create_account(data_dir, args.account, args.password);
    args.command = WIKI_COMMAND_LOGIN;
  }

  args.temp_loggedin = 0;
  if (args.command == WIKI_COMMAND_SAVE && args.session == NULL) {
    const char *password;

    password = wiki_data_read_password(data_dir, args.account);
    if (password == NULL || args.password == NULL) {
      cgi_die("password is null");
    }
    if (argon2id_verify(password, args.password, strlen(args.password)) !=
        ARGON2_OK) {
      cgi_die("passwords not match");
    }

    args.temp_loggedin = 1;
  } else if (args.command == WIKI_COMMAND_LOGIN) {
    const char *password;

    password = wiki_data_read_password(data_dir, args.account);
    if (password == NULL || args.password == NULL) {
      cgi_die("password is null");
    }
    if (argon2id_verify(password, args.password, strlen(args.password)) !=
        ARGON2_OK) {
      cgi_die("passwords not match");
    }

    args.session = wiki_data_begin_session(data_dir, args.account);
  }

  args.loggedin = 0;
  if (args.account != NULL) {
    session = wiki_data_read_session(data_dir, args.account);
    if (session != NULL && args.session != NULL) {
      if (match_passwords(session, args.session)) {
        args.loggedin = 1;
      }
    }
  }

  if (args.command == WIKI_COMMAND_LOGOUT) {
    if (args.loggedin) {
      wiki_data_end_session(data_dir, args.account);
      args.loggedin = 0;
    } else {
      cgi_die("not logged in");
    }
  }

  if (args.command == WIKI_COMMAND_SAVE) {
    if (!args.temp_loggedin && !args.loggedin) {
      cgi_die("not logged in");
    }
  }

  if (args.command != WIKI_COMMAND_PREVIEW) {
    if (args.command == WIKI_COMMAND_SAVE) {
      const char *filter;
      char *filtered;

      filter = getenv("WIKI_FILTER");
      if (filter != NULL) {
        filtered = run_filter(filter, args.text);
      } else {
        filtered = args.text;
      }
      wiki_data_text_write(data_dir, args.page, filtered);
      if (filter != NULL) {
        free(filtered);
      }
    }

    text = wiki_data_text_read(data_dir, args.page);
  } else {
    text = NULL;
  }

  /* output */

  wiki_draw(text, &args);

  /* finish */

  free((char *)text);
  wiki_finish();

  return 0;
}
