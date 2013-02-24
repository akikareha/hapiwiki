#ifndef WIKI_MAIN_H
#define WIKI_MAIN_H

enum {
	WIKI_COMMAND_VIEW,
	WIKI_COMMAND_EDIT,
	WIKI_COMMAND_PREVIEW,
	WIKI_COMMAND_SAVE,

	WIKI_COMMAND_CREATE_ACCOUNT,
	WIKI_COMMAND_LOGIN_FORM,
	WIKI_COMMAND_LOGIN,
	WIKI_COMMAND_LOGOUT,
};

struct wiki_args {
	int command;
	int loggedin;
	int temp_loggedin;

	const char *page;
	char *text;

	const char *account;
	const char *password;
	const char *confirm;
	const char *session;
};

#endif
