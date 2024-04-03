#ifndef __KPASS_H__
#define __KPASS_H__

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

typedef struct kpass_st {
	int      id;
	char   mode;
	int   debug;
	sqlite3 *db;
	char *pattern;
	int show_password;
} kpass_t;

extern kpass_t kpass;

enum kpass_err_t {
	RETERR = -1,
	RETSXS =  0,
};

enum kpass_limits_t {
	USERNAME_LEN  = 64,
	PASSWD_LEN    = 64,
	TAG_LEN       = 64,
	TIME_LEN      = 64,
	NAME_LEN      = 128,
	URL_LEN       = 512,
	BUFF_MAX      = 1024
};

typedef struct kpass_entry_st {
	int  id;
	char tag[TAG_LEN];
	char name[NAME_LEN];
	char user[USERNAME_LEN];
	char password[PASSWD_LEN];
	char url[URL_LEN];
	char notes[BUFF_MAX];
	char stamp[TIME_LEN];
} kpass_entry_t;

int kpass_add_entry(void);
int kpass_list_entries(void);
int kpass_list_entry(const int id);
int kpass_edit_entry(const int id);
int kpass_delete_entry(const int id);
int kpass_grep_entries(const char *pattern);

#endif
