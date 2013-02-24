#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#include "cgi/cgi.h"

#include "wiki/config.h"
#include "wiki/data.h"

static int is_valid_page(const char *page)
{
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
	for (s=page; *s; s++) {
		if (*s == '/') {
			return 0;
		}
	}

	return 1;
}

const char *wiki_data_text_read(const char *page)
{
	char path[WIKI_PATH_SIZE + 1];
	struct stat st;
	char *text;
	FILE *fp;

	if (!is_valid_page(page)) {
		cgi_die("invalid page name");
	}

	if (strlen(WIKI_DATA_DIR) + 1 + strlen("pages") + 1 + strlen(page) + 1 + strlen("current") > WIKI_PATH_SIZE) {
		cgi_die("path size exceeded");
	}
	snprintf(path, WIKI_PATH_SIZE, "%s/pages/%s/current", WIKI_DATA_DIR, page);
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

void wiki_data_text_write(const char *page, const char *text)
{
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

	if (strlen(WIKI_DATA_DIR) + 1 + strlen("pages") > WIKI_PATH_SIZE) {
		cgi_die("path size exceeded");
	}
	snprintf(path, WIKI_PATH_SIZE, "%s/pages", WIKI_DATA_DIR);
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

	if (strlen(WIKI_DATA_DIR) + 1 + strlen("pages") + 1 + strlen(page) > WIKI_PATH_SIZE) {
		cgi_die("path size exceeded");
	}
	snprintf(path, WIKI_PATH_SIZE, "%s/pages/%s", WIKI_DATA_DIR, page);
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

	if (strlen(WIKI_DATA_DIR) + 1 + strlen("pages") + 1 + strlen(page) + 1 + strlen("current") > WIKI_PATH_SIZE) {
		cgi_die("path size exceeded");
	}
	snprintf(path, WIKI_PATH_SIZE, "%s/pages/%s/current", WIKI_DATA_DIR, page);
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

void wiki_data_create_account(const char *account, const char *password)
{
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

	if (strlen(WIKI_DATA_DIR) + 1 + strlen("accounts") > WIKI_PATH_SIZE) {
		cgi_die("path size exceeded");
	}
	snprintf(path, WIKI_PATH_SIZE, "%s/accounts", WIKI_DATA_DIR);
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

	if (strlen(WIKI_DATA_DIR) + 1 + strlen("accounts") + 1 + strlen(account) > WIKI_PATH_SIZE) {
		cgi_die("path size exceeded");
	}
	snprintf(path, WIKI_PATH_SIZE, "%s/accounts/%s", WIKI_DATA_DIR, account);
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

	if (strlen(WIKI_DATA_DIR) + 1 + strlen("accounts") + 1 + strlen(account) + 1 + strlen("password") > WIKI_PATH_SIZE) {
		cgi_die("path size exceeded");
	}
	snprintf(path, WIKI_PATH_SIZE, "%s/accounts/%s/password", WIKI_DATA_DIR, account);
	path[WIKI_PATH_SIZE] = '\0';

	fp = fopen(path, "w");
	if (fp == NULL) {
		cgi_die("fopen");
	}
	size = strlen(password);
	if (fwrite(password, 1, size, fp) != size) {
		cgi_die("fwrite");
	}
	if (fclose(fp) == EOF) {
		cgi_die("fclose");
	}
}

const char *wiki_data_read_password(const char *account)
{
	char path[WIKI_PATH_SIZE + 1];
	struct stat st;
	char *password;
	FILE *fp;

	if (!is_valid_page(account)) {
		cgi_die("invalid account name");
	}

	if (strlen(WIKI_DATA_DIR) + 1 + strlen("accounts") + 1 + strlen(account) + 1 + strlen("password") > WIKI_PATH_SIZE) {
		cgi_die("path size exceeded");
	}
	snprintf(path, WIKI_PATH_SIZE, "%s/accounts/%s/password", WIKI_DATA_DIR, account);
	path[WIKI_PATH_SIZE] = '\0';

	if (stat(path, &st) == -1) {
		if (errno == ENOENT) {
			return NULL;
		} else {
			cgi_die("stat");
		}
	}

	password = malloc(st.st_size + 1);
	if (password == NULL) {
		cgi_die("malloc");
	}

	fp = fopen(path, "r");
	if (fp == NULL) {
		cgi_die("fopen");
	}
	if (fread(password, 1, st.st_size, fp) != st.st_size) {
		cgi_die("fread");
	}
	if (fclose(fp) == EOF) {
		cgi_die("fclose");
	}

	password[st.st_size] = '\0';

	return password;
}

const char *wiki_data_begin_session(const char *account)
{
	int length;
	char *secret;
	struct timeval tv;
	int i;
	char *token;
	char path[WIKI_PATH_SIZE + 1];
	FILE *fp;
	int size;

	if (!is_valid_page(account)) {
		cgi_die("invalid account name");
	}

	length = strlen(account);
	secret = malloc(length + 1 + 16 + 1);
	gettimeofday(&tv, NULL);
	srand(tv.tv_usec);
	sprintf(secret, "%s/", account);
	for (i=0; i<16; i++) {
		sprintf(secret + length + 1 + i, "%02x", rand() & 0xff);
	}

	token = malloc(strlen(WIKI_SECRET) + strlen(secret) + 1);
	if (token == NULL) {
		cgi_die("malloc");
	}
	sprintf(token, "%s%s", WIKI_SECRET, secret);

	if (strlen(WIKI_DATA_DIR) + 1 + strlen("accounts") + 1 + strlen(account) + 1 + strlen("session") > WIKI_PATH_SIZE) {
		cgi_die("path size exceeded");
	}
	snprintf(path, WIKI_PATH_SIZE, "%s/accounts/%s/session", WIKI_DATA_DIR, account);
	path[WIKI_PATH_SIZE] = '\0';

	fp = fopen(path, "w");
	if (fp == NULL) {
		cgi_die("fopen");
	}
	size = strlen(token);
	if (fwrite(token, 1, size, fp) != size) {
		cgi_die("fwrite");
	}
	if (fclose(fp) == EOF) {
		cgi_die("fclose");
	}

	return token;
}

void wiki_data_end_session(const char *account)
{
	char path[WIKI_PATH_SIZE + 1];

	if (!is_valid_page(account)) {
		cgi_die("invalid account name");
	}

	if (strlen(WIKI_DATA_DIR) + 1 + strlen("accounts") + 1 + strlen(account) + 1 + strlen("session") > WIKI_PATH_SIZE) {
		cgi_die("path size exceeded");
	}
	snprintf(path, WIKI_PATH_SIZE, "%s/accounts/%s/session", WIKI_DATA_DIR, account);
	path[WIKI_PATH_SIZE] = '\0';

	if (unlink(path) == -1) {
		cgi_die("unlink");
	}
}

const char *wiki_data_read_session(const char *account)
{
	char path[WIKI_PATH_SIZE + 1];
	struct stat st;
	char *session;
	FILE *fp;

	if (!is_valid_page(account)) {
		cgi_die("invalid account name");
	}

	if (strlen(WIKI_DATA_DIR) + 1 + strlen("accounts") + 1 + strlen(account) + 1 + strlen("session") > WIKI_PATH_SIZE) {
		cgi_die("path size exceeded");
	}
	snprintf(path, WIKI_PATH_SIZE, "%s/accounts/%s/session", WIKI_DATA_DIR, account);
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
