#ifndef __KPASS_DB_H__
#define __KPASS_DB_H__

#include <sqlite3.h>

int kpass_db_init(const char *path);
sqlite3 *kpass_db_open(const char *path);
void kpass_db_close(sqlite3 *db);
int kpass_db_commit(char *fmt, ...);
int kpass_db_count(char *fmt, ...);
int kpass_db_exec(int (*callback)(void*, int, char**, char**), void *arg, char *fmt, ...);

#endif
