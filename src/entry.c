
#include "db.h"
#include "kpass.h"
#include "logit.h"

#define return_if_invalid_id(x) \
	do { \
		if (x <= 0) { \
			fprintf(stderr, "Invalid entry index %d.\n", x); \
			return RETERR; \
		} \
	} while(0);

#define return_if_entry_not_exists(x) \
	do { \
		logit("id: %d", x); \
		retval = kpass_db_count("SELECT count(*) FROM entries where id=%d;", x); \
		logit("nitems: %d", retval); \
		if (retval == RETERR) { \
			logit("%s", "kpass_db_count() failed"); \
			return RETERR; \
		} \
		if (retval == 0) { \
			fprintf(stderr, "No such entry with index %d.\n", x); \
			return RETERR; \
		} \
	} while(0);

static kpass_entry_t *kpass_entry_init(void)
{
	kpass_entry_t *entry = NULL;

	entry = (kpass_entry_t *)calloc(1, sizeof(*entry));
	if (entry == NULL) {
		logit("calloc() failed: %s", strerror(errno));
		exit(RETERR);
	}

	return entry;
}

static void kpass_entry_free(kpass_entry_t *entry)
{
	if (entry)
		free(entry);

	return;
}

static void kpass_print_entry(kpass_entry_t *entry)
{
	if (entry == NULL) {
		logit("%s", "entry is NULL");
	} else {
		logit("Id       : %d", entry->id);
		logit("Title    : %s", entry->title);
		logit("Username : %s", entry->user);
		logit("Password : %s", entry->password);
		logit("Url      : %s", entry->url);
		logit("Notes    : %s", entry->notes);
	}

	return;
}

/**
 * Description:
 *   Get and set value for the kpass_entry objects.
 *
 * @name   : Name of the kpass entry object (Title, Username, Password and etc.,)
 * @object : Pointer to the kpass entry object
 * @size   : size of @object
 *
 * Retval:
 * @retval : Return @size on copy, else 0 on no copy.
 */
static int kpass_entry_set(const char *name, char *object, size_t size)
{
	int retval = 0;
	char buff[BUFF_MAX] = {0};

	return_if_null(name);
	return_if_null(object);

	fprintf(stdout, "%s", name);
	fgets(buff, sizeof(buff), stdin);
	buff[strlen(buff) - 1] = '\0';

	if (strlen(buff) > 0) {
		strncpy(object, buff, size);
		retval = size;
	}

	return retval;
}

static int kpass_insert_entry(kpass_entry_t *entry)
{
	int  retval = -1;

	do {
		return_if_null(entry);

		retval = kpass_db_commit("insert into entries(title, user, url, password, notes) values('%q','%q','%q','%q','%q')",
		                          entry->title, entry->user, entry->url, entry->password, entry->notes);
		if (retval < 0) {
			logit("%s", "failed to insert entry");
			break;
		}

		retval = RETSXS;
	} while(0);

	logit_retval();

	return retval;
}

static int cb_get_entry(void *data, int ncolumn, char **column_value, char **column_name)
{
	kpass_entry_t *entry = (kpass_entry_t *)data;

	return_if_null(entry);

	entry->id = atoi(column_value[0]);
	strcpy(entry->title,    column_value[1]);
	strcpy(entry->user,     column_value[2]);
	strcpy(entry->url,      column_value[3]);
	strcpy(entry->password, column_value[4]);
	strcpy(entry->notes,    column_value[5]);
	strcpy(entry->stamp,    column_value[6]);

	return RETSXS;
}

int kpass_edit_entry(const int id)
{
	int  retval = -1;
	kpass_entry_t *entry = NULL;

	do {
		return_if_invalid_id(id);
		return_if_entry_not_exists(id);

		entry = kpass_entry_init();
		retval = kpass_db_exec(cb_get_entry, entry, "select * from entries where id=%d;", id);

		kpass_print_entry(entry);

		fprintf(stdout, "Update entry details:\n");
		fprintf(stdout, " Title [%s]\n", entry->title);
		kpass_entry_set("   New Title    : ", entry->title,    sizeof(entry->title));

		fprintf(stdout, " Username [%s]\n", entry->user);
		kpass_entry_set("   New Username : ", entry->user,     sizeof(entry->user));

		fprintf(stdout, " Password [%s]\n", entry->password);
		kpass_entry_set("   New Password : ", entry->password, sizeof(entry->password));

		fprintf(stdout, " Url [%s]\n", entry->url);
		kpass_entry_set("   New Url      : ", entry->url,      sizeof(entry->url));

		fprintf(stdout, " Notes [%s]\n", entry->notes);
		kpass_entry_set("   New Notes    : ", entry->notes,    sizeof(entry->notes));


		retval = kpass_db_commit("update entries set title='%q', "
		                         "user='%q', "
		                         "url='%q', "
		                         "password='%q', "
		                         "notes='%q', timestamp=datetime('now', 'localtime') where id=%d;",
		                         entry->title,
		                         entry->user,
		                         entry->url,
		                         entry->password,
		                         entry->notes, id);
		if (retval <= 0) {
			logit("%s", "failed to insert entry");
			retval = RETERR;
			break;
		}

		retval = RETSXS;
	} while(0);

	logit_retval();

	return retval;
}

int kpass_delete_entry(const int id)
{
	int  retval = -1;

	do {
		return_if_invalid_id(id);

		logit("id: %d", id);
		retval = kpass_db_count("SELECT count(*) FROM entries where id=%d;", id);
		logit("nitems: %d", retval);

		if (retval == RETERR) {
			logit("%s", "kpass_db_count() failed");
			break;
		}

		if (retval == 0) {
			fprintf(stderr, "No such entry with index %d.\n", id);
			retval = RETERR;
			break;
		}

		retval = kpass_db_commit("delete from entries where id=%d;", id);
		if (retval < 0) {
			logit("%s", "failed to insert entry");
			break;
		}

		fprintf(stdout, "Deleted entry %d.\n", id);

		retval = RETSXS;
	} while(0);

	logit_retval();

	return retval;
}

int kpass_add_entry(void)
{
	int retval = -1;
	kpass_entry_t *entry = NULL;

	do {
		entry = kpass_entry_init();
		if (entry == NULL) {
			logit("%s", "kpass_entry_init() failed");
			break;
		}

		fprintf(stdout, "Enter new entry details:\n");
		kpass_entry_set(" Title    : ", entry->title,    sizeof(entry->title));
		kpass_entry_set(" Username : ", entry->user,     sizeof(entry->user));
		kpass_entry_set(" Password : ", entry->password, sizeof(entry->password));
		kpass_entry_set(" Url      : ", entry->url,      sizeof(entry->url));
		kpass_entry_set(" Notes    : ", entry->notes,    sizeof(entry->notes));

		retval = kpass_insert_entry(entry);
		if (retval != RETSXS) {
			logit("%s", "kpass_insert_entry() failed");
			break;
		}

		fprintf(stdout, "Added new entry.\n");
		kpass_entry_free(entry);
	} while(0);

	logit_retval();

	return retval;
}

static int cb_print_entry(void *data, int ncolumn, char **column_value, char **column_name)
{
	int show_password = *(int *)data;

	fprintf(stdout, "=====================================================================\n");
	fprintf(stdout, "Id       : %s\n", column_value[0]);
	fprintf(stdout, "Title    : %s\n", column_value[1]);
	fprintf(stdout, "User     : %s\n", column_value[2]);
	fprintf(stdout, "Url      : %s\n", column_value[3]);

	if(show_password == 1)
		fprintf(stdout, "Password : %s\n", column_value[4]);
	else
		fprintf(stdout, "Password : **********\n");

	fprintf(stdout, "Notes    : %s\n", column_value[5]);
	fprintf(stdout, "Modified : %s\n", column_value[6]);

	fprintf(stdout, "=====================================================================\n");

	return 0;
}

int kpass_list_entry(const int id)
{
	int  retval = -1;

	do {
		return_if_invalid_id(id);
		return_if_entry_not_exists(id);

		retval = kpass_db_exec(cb_print_entry, &kpass.show_password, "select * from entries where id=%d", id);
	} while(0);

	return retval;
}

int kpass_list_entries(void)
{
	int  retval = -1;

	retval = kpass_db_exec(cb_print_entry, &kpass.show_password, "select * from entries");

	return retval;
}

int kpass_grep_entries(const char *pattern)
{
	int  retval = -1;

	do {
		logit("%s", pattern);

		retval = kpass_db_count("SELECT count(*) FROM entries "
		                           "where title like '%%%q%%' "
		                              "or user  like '%%%q%%' "
		                              "or url   like '%%%q%%' "
		                              "or notes like '%%%q%%';",
		                    pattern, pattern, pattern, pattern);

		logit("nitems: %d", retval);

		if (retval == RETERR) {
			logit("%s", "kpass_db_count() failed");
			retval = RETERR;
			break;
		}

		if (retval == 0) {
			fprintf(stderr, "No match found.\n");
			retval = RETERR;
			break;
		}

		retval = kpass_db_exec(cb_print_entry,
		                       &kpass.show_password,
		                       "select * from entries "
		                       "where title like '%%%q%%' "
		                          "or user  like '%%%q%%' "
		                          "or url   like '%%%q%%' "
		                          "or notes like '%%%q%%';",
		                pattern, pattern, pattern, pattern);
	} while(0);

	return retval;
}

/* EOF */
