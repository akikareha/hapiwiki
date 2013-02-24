#ifndef WIKI_DATA_H
#define WIKI_DATA_H

extern const char *wiki_data_text_read(const char *page);
extern void wiki_data_text_write(const char *page, const char *text);
extern void wiki_data_create_account(const char *account, const char *password);
extern const char *wiki_data_read_password(const char *account);
extern const char *wiki_data_begin_session(const char *account);
extern void wiki_data_end_session(const char *account);
extern const char *wiki_data_read_session(const char *account);

#endif
