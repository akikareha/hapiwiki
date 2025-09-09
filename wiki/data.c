#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include "argon2/argon2.h"
#include "cgi/cgi.h"
#include "util/util.h"

#include "wiki/config.h"
#include "wiki/data.h"

static int is_valid_page(const char *page) {
  const char *s;

  if (page == NULL) {
    return 0;
  }

  if (strcmp(".", page) == 0) {
    return 0;
  }
  if (strcmp("..", page) == 0) {
    return 0;
  }
  for (s = page; *s; s++) {
    if (*s == '/') {
      return 0;
    }
  }

  return 1;
}

const char *wiki_data_text_read(const char *data_dir, const char *page) {
  char path[WIKI_PATH_SIZE + 1];
  struct stat st;
  char *text;
  FILE *fp;

  if (!is_valid_page(page)) {
    cgi_die("invalid page name");
  }

  if (strlen(data_dir) + 1 + strlen("pages") + 1 + strlen(page) + 1 +
          strlen("current") >
      WIKI_PATH_SIZE) {
    cgi_die("path size exceeded");
  }
  snprintf(path, WIKI_PATH_SIZE, "%s/pages/%s/current", data_dir, page);
  path[WIKI_PATH_SIZE] = '\0';

  if (stat(path, &st) == -1) {
    if (errno == ENOENT) {
      return NULL;
    } else {
      cgi_die("stat");
    }
  }

  text = malloc(st.st_size + 1);
  if (text == NULL) {
    cgi_die("malloc");
  }

  fp = fopen(path, "r");
  if (fp == NULL) {
    cgi_die("fopen");
  }
  if (fread(text, 1, st.st_size, fp) != st.st_size) {
    cgi_die("fread");
  }
  if (fclose(fp) == EOF) {
    cgi_die("fclose");
  }

  text[st.st_size] = '\0';

  return text;
}

void wiki_data_text_write(const char *data_dir, const char *page,
                          const char *text) {
  char path[WIKI_PATH_SIZE + 1];
  struct stat st;
  FILE *fp;
  int size;

  if (!is_valid_page(page)) {
    cgi_die("invalid page name");
  }

  if (text == NULL) {
    cgi_die("invalid text");
  }

  if (strlen(data_dir) + 1 + strlen("pages") + 1 > WIKI_PATH_SIZE) {
    cgi_die("path size exceeded");
  }
  snprintf(path, WIKI_PATH_SIZE, "%s/pages", data_dir);
  path[WIKI_PATH_SIZE] = '\0';

  if (stat(path, &st) == -1) {
    if (errno == ENOENT) {
      if (mkdir(path, 02770) == -1) {
        cgi_die("mkdir");
      }
      if (chmod(path, 02770) == -1) {
        cgi_die("chmod");
      }
    } else {
      cgi_die("stat");
    }
  }

  if (strlen(data_dir) + 1 + strlen("pages") + 1 + strlen(page) >
      WIKI_PATH_SIZE) {
    cgi_die("path size exceeded");
  }
  snprintf(path, WIKI_PATH_SIZE, "%s/pages/%s", data_dir, page);
  path[WIKI_PATH_SIZE] = '\0';

  if (stat(path, &st) == -1) {
    if (errno == ENOENT) {
      if (mkdir(path, 02770) == -1) {
        cgi_die("mkdir");
      }
      if (chmod(path, 02770) == -1) {
        cgi_die("chmod");
      }
    } else {
      cgi_die("stat");
    }
  }

  if (strlen(data_dir) + 1 + strlen("pages") + 1 + strlen(page) + 1 +
          strlen("current") >
      WIKI_PATH_SIZE) {
    cgi_die("path size exceeded");
  }
  snprintf(path, WIKI_PATH_SIZE, "%s/pages/%s/current", data_dir, page);
  path[WIKI_PATH_SIZE] = '\0';

  fp = fopen(path, "w");
  if (fp == NULL) {
    cgi_die("fopen");
  }
  size = strlen(text);
  if (fwrite(text, 1, size, fp) != size) {
    cgi_die("fwrite");
  }
  if (fclose(fp) == EOF) {
    cgi_die("fclose");
  }
}

#define SALT_LEN 16

static int get_random_salt(unsigned char *salt, size_t saltlen) {
  FILE *urandom = fopen("/dev/urandom", "rb");
  if (!urandom) {
    return -1;
  }
  if (fread(salt, 1, saltlen, urandom) != saltlen) {
    fclose(urandom);
    return -1;
  }
  fclose(urandom);
  return 0;
}

void wiki_data_create_account(const char *data_dir, const char *account,
                              const char *password) {
  int result;
  uint32_t t_cost = 2;       /* time cost */
  uint32_t m_cost = 1 << 16; /* memory cost */
  uint32_t parallelism = 1;  /* threads */
  unsigned char salt[SALT_LEN];
  char encoded[512];

  char path[WIKI_PATH_SIZE + 1];
  struct stat st;
  FILE *fp;
  int size;

  if (!is_valid_page(account)) {
    cgi_die("invalid account name");
  }

  if (password == NULL) {
    cgi_die("invalid password");
  }

  if (get_random_salt(salt, SALT_LEN) != 0) {
    cgi_die("failed to generate salt");
  }

  result = argon2id_hash_encoded(t_cost, m_cost, parallelism, password,
                                 strlen(password), salt, SALT_LEN, 32, encoded,
                                 sizeof(encoded)); /* NULL: random salt */

  if (result != ARGON2_OK) {
    cgi_die("hash failed");
  }

  if (strlen(data_dir) + 1 + strlen("accounts") + 1 > WIKI_PATH_SIZE) {
    cgi_die("path size exceeded");
  }
  snprintf(path, WIKI_PATH_SIZE, "%s/accounts", data_dir);
  path[WIKI_PATH_SIZE] = '\0';

  if (stat(path, &st) == -1) {
    if (errno == ENOENT) {
      if (mkdir(path, 02770) == -1) {
        cgi_die("mkdir");
      }
      if (chmod(path, 02770) == -1) {
        cgi_die("chmod");
      }
    } else {
      cgi_die("stat");
    }
  }

  if (strlen(data_dir) + 1 + strlen("accounts") + 1 + strlen(account) >
      WIKI_PATH_SIZE) {
    cgi_die("path size exceeded");
  }
  snprintf(path, WIKI_PATH_SIZE, "%s/accounts/%s", data_dir, account);
  path[WIKI_PATH_SIZE] = '\0';

  if (stat(path, &st) == -1) {
    if (errno == ENOENT) {
      if (mkdir(path, 02770) == -1) {
        cgi_die("mkdir");
      }
      if (chmod(path, 02770) == -1) {
        cgi_die("chmod");
      }
    } else {
      cgi_die("stat");
    }
  } else {
    cgi_die("the account already exists");
  }

  if (strlen(data_dir) + 1 + strlen("accounts") + 1 + strlen(account) + 1 +
          strlen("password") >
      WIKI_PATH_SIZE) {
    cgi_die("path size exceeded");
  }
  snprintf(path, WIKI_PATH_SIZE, "%s/accounts/%s/password", data_dir, account);
  path[WIKI_PATH_SIZE] = '\0';

  fp = fopen(path, "w");
  if (fp == NULL) {
    cgi_die("fopen");
  }
  size = strlen(encoded);
  if (fwrite(encoded, 1, size, fp) != size) {
    cgi_die("fwrite");
  }
  if (fclose(fp) == EOF) {
    cgi_die("fclose");
  }
}

const char *wiki_data_read_password(const char *data_dir, const char *account) {
  char path[WIKI_PATH_SIZE + 1];
  struct stat st;
  char *hash;
  FILE *fp;

  if (!is_valid_page(account)) {
    cgi_die("invalid account name");
  }

  if (strlen(data_dir) + 1 + strlen("accounts") + 1 + strlen(account) + 1 +
          strlen("password") >
      WIKI_PATH_SIZE) {
    cgi_die("path size exceeded");
  }
  snprintf(path, WIKI_PATH_SIZE, "%s/accounts/%s/password", data_dir, account);
  path[WIKI_PATH_SIZE] = '\0';

  if (stat(path, &st) == -1) {
    if (errno == ENOENT) {
      return NULL;
    } else {
      cgi_die("stat");
    }
  }

  hash = malloc(st.st_size + 1);
  if (hash == NULL) {
    cgi_die("malloc");
  }

  fp = fopen(path, "r");
  if (fp == NULL) {
    cgi_die("fopen");
  }
  if (fread(hash, 1, st.st_size, fp) != st.st_size) {
    cgi_die("fread");
  }
  if (fclose(fp) == EOF) {
    cgi_die("fclose");
  }

  hash[st.st_size] = '\0';

  return hash;
}

const char *wiki_data_begin_session(const char *data_dir, const char *account) {
  int length;
  int secret_length;
  char *secret;
  struct timeval tv;
  int i;
  int token_length;
  char *token;
  MD5_CTX md5_ctx;
  unsigned char hash[16];
  char *hash_string;
  char hash_string2[2 * 16 + 1];
  char path[WIKI_PATH_SIZE + 1];
  FILE *fp;
  int size;

  if (!is_valid_page(account)) {
    cgi_die("invalid account name");
  }

  length = strlen(account);
  secret_length = length + 1 + 32;
  secret = malloc(secret_length + 1);
  gettimeofday(&tv, NULL);
  srand(tv.tv_usec);
  sprintf(secret, "%s/", account);
  for (i = 0; i < 16; i++) {
    sprintf(secret + length + 1 + i * 2, "%02x", rand() & 0xff);
  }

  token_length = strlen(WIKI_SECRET) + secret_length;
  token = malloc(token_length + 1);
  if (token == NULL) {
    cgi_die("malloc");
  }
  sprintf(token, "%s%s", WIKI_SECRET, secret);

  MD5_Init(&md5_ctx);
  MD5_Update(&md5_ctx, token, strlen(token));
  MD5_Final(hash, &md5_ctx);

  free(token);

  hash_string = malloc(2 * 16 + 1);
  if (hash_string == NULL) {
    cgi_die("malloc");
  }
  binary_to_hex_string(hash, 16, hash_string);

  MD5_Init(&md5_ctx);
  MD5_Update(&md5_ctx, hash_string, strlen(hash_string));
  MD5_Final(hash, &md5_ctx);

  binary_to_hex_string(hash, 16, hash_string2);

  if (strlen(data_dir) + 1 + strlen("accounts") + 1 + strlen(account) + 1 +
          strlen("session") >
      WIKI_PATH_SIZE) {
    cgi_die("path size exceeded");
  }
  snprintf(path, WIKI_PATH_SIZE, "%s/accounts/%s/session", data_dir, account);
  path[WIKI_PATH_SIZE] = '\0';

  fp = fopen(path, "w");
  if (fp == NULL) {
    cgi_die("fopen");
  }
  size = strlen(hash_string2);
  if (fwrite(hash_string2, 1, size, fp) != size) {
    cgi_die("fwrite");
  }
  if (fclose(fp) == EOF) {
    cgi_die("fclose");
  }

  return hash_string;
}

void wiki_data_end_session(const char *data_dir, const char *account) {
  char path[WIKI_PATH_SIZE + 1];

  if (!is_valid_page(account)) {
    cgi_die("invalid account name");
  }

  if (strlen(data_dir) + 1 + strlen("accounts") + 1 + strlen(account) + 1 +
          strlen("session") >
      WIKI_PATH_SIZE) {
    cgi_die("path size exceeded");
  }
  snprintf(path, WIKI_PATH_SIZE, "%s/accounts/%s/session", data_dir, account);
  path[WIKI_PATH_SIZE] = '\0';

  if (unlink(path) == -1) {
    cgi_die("unlink");
  }
}

const char *wiki_data_read_session(const char *data_dir, const char *account) {
  char path[WIKI_PATH_SIZE + 1];
  struct stat st;
  char *session;
  FILE *fp;

  if (!is_valid_page(account)) {
    cgi_die("invalid account name");
  }

  if (strlen(data_dir) + 1 + strlen("accounts") + 1 + strlen(account) + 1 +
          strlen("session") >
      WIKI_PATH_SIZE) {
    cgi_die("path size exceeded");
  }
  snprintf(path, WIKI_PATH_SIZE, "%s/accounts/%s/session", data_dir, account);
  path[WIKI_PATH_SIZE] = '\0';

  if (stat(path, &st) == -1) {
    if (errno == ENOENT) {
      return NULL;
    } else {
      cgi_die("stat");
    }
  }

  session = malloc(st.st_size + 1);
  if (session == NULL) {
    cgi_die("malloc");
  }

  fp = fopen(path, "r");
  if (fp == NULL) {
    cgi_die("fopen");
  }
  if (fread(session, 1, st.st_size, fp) != st.st_size) {
    cgi_die("fread");
  }
  if (fclose(fp) == EOF) {
    cgi_die("fclose");
  }

  session[st.st_size] = '\0';

  return session;
}
