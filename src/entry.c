
#include "db.h"
#include "kpass.h"
#include "logit.h"
#include "utils.h"

#define return_if_invalid_id(x) \
	do { \
		if (x <= 0) { \
			kpass_printf("Invalid entry index %d.", x); \
			return RETERR; \
		} \
	} while(0);

#define return_if_entry_not_exists(x) \
	do { \
		logit("id: %d", x); \
		retval = kpass_db_count("SELECT count(*) from KPASS_ENTRIES where ID = %d", x); \
		logit("nitems: %d", retval); \
		if (retval == RETERR) { \
			kpass_error("%s", "kpass_db_count() failed"); \
			return RETERR; \
		} \
		if (retval == 0) { \
			kpass_printf("No such entry with index %d.", x); \
			return RETERR; \
		} \
	} while(0);

static kpass_entry_t *kpass_entry_init(void)
{
	kpass_entry_t *entry = NULL;

	entry = (kpass_entry_t *)calloc(1, sizeof(*entry));
	if (entry == NULL) {
		kpass_error("calloc() failed: %s", strerror(errno));
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
		kpass_error("%s", "entry is NULL");
	} else {
		kpass_printf("Id       : %d", entry->id);
		kpass_printf("Tag      : %s", entry->tag);
		kpass_printf("Name     : %s", entry->name);
		kpass_printf("Username : %s", entry->user);
		kpass_printf("Password : %s", entry->password);
		kpass_printf("Url      : %s", entry->url);
		kpass_printf("Notes    : %s", entry->notes);
	}

	return;
}

/**
 * Description:
 *   Get and set value for the kpass_entry objects.
 *
 * @name   : Name of the kpass entry object (Name, Username, Password and etc.,)
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

		retval = kpass_db_commit("INSERT into KPASS_ENTRIES(TAG, NAME, USERNAME, PASSWORD, URL, NOTES) VALUES ('%q','%q','%q','%q','%q','%q')",
		                          entry->tag, entry->name, entry->user, entry->password, entry->url, entry->notes);
		if (retval < 0) {
			kpass_error("%s", "failed to insert entry");
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
	strcpy(entry->tag,      column_value[1]);
	strcpy(entry->name,     column_value[2]);
	strcpy(entry->user,     column_value[3]);
	strcpy(entry->password, column_value[4]);
	strcpy(entry->url,      column_value[5]);
	strcpy(entry->notes,    column_value[6]);
	strcpy(entry->stamp,    column_value[7]);

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
		retval = kpass_db_exec(cb_get_entry, entry, "SELECT * from KPASS_ENTRIES where ID = %d", id);

		kpass_printf("Update entry details:");

		kpass_printf(" Tag [%s]", entry->tag);
		kpass_entry_set("   New Tag    : ", entry->tag, sizeof(entry->tag));

		kpass_printf(" Name [%s]", entry->name);
		kpass_entry_set("   New Name    : ", entry->name, sizeof(entry->name));

		kpass_printf(" Username [%s]", entry->user);
		kpass_entry_set("   New Username : ", entry->user, sizeof(entry->user));

		kpass_printf(" Password [%s]", entry->password);
		kpass_entry_set("   New Password : ", entry->password, sizeof(entry->password));

		kpass_printf(" Url [%s]", entry->url);
		kpass_entry_set("   New Url      : ", entry->url, sizeof(entry->url));

		kpass_printf(" Notes [%s]", entry->notes);
		kpass_entry_set("   New Notes    : ", entry->notes, sizeof(entry->notes));

		if (true != kpass_user_action()) {
			retval = RETERR;
			break;
		}

		retval = kpass_db_commit("UPDATE KPASS_ENTRIES set "
		                         "TAG = '%q', "
		                         "NAME = '%q', "
		                         "USERNAME ='%q', "
		                         "PASSWORD = '%q', "
		                         "URL = '%q', "
		                         "NOTES = '%q', "
		                         "TIMESTAMP = datetime('now', 'localtime') where ID = %d",
		                         entry->tag,
		                         entry->name,
		                         entry->user,
		                         entry->password,
		                         entry->url,
		                         entry->notes, id);
		if (retval <= 0) {
			kpass_error("%s", "failed to insert entry");
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
		retval = kpass_db_count("SELECT count(*) from KPASS_ENTRIES where ID = %d", id);
		logit("nitems: %d", retval);

		if (retval == RETERR) {
			kpass_error("%s", "kpass_db_count() failed");
			break;
		}

		if (retval == 0) {
			kpass_printf("No such entry with index %d.", id);
			retval = RETERR;
			break;
		}

		if (true != kpass_user_action()) {
			retval = RETERR;
			break;
		}

		retval = kpass_db_commit("DELETE from KPASS_ENTRIES where ID = %d", id);
		if (retval < 0) {
			kpass_error("%s", "failed to insert entry");
			break;
		}

		kpass_printf("Deleted entry %d.", id);

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
			kpass_error("%s", "kpass_entry_init() failed");
			break;
		}

		kpass_printf("Enter new entry details:");
		kpass_entry_set(" Tag      : ", entry->tag,      sizeof(entry->tag));
		kpass_entry_set(" Name     : ", entry->name,     sizeof(entry->name));
		kpass_entry_set(" Username : ", entry->user,     sizeof(entry->user));
		kpass_entry_set(" Password : ", entry->password, sizeof(entry->password));
		kpass_entry_set(" Url      : ", entry->url,      sizeof(entry->url));
		kpass_entry_set(" Notes    : ", entry->notes,    sizeof(entry->notes));

		retval = kpass_insert_entry(entry);
		if (retval != RETSXS) {
			kpass_error("%s", "kpass_insert_entry() failed");
			break;
		}

		kpass_printf("Added new entry.");
		kpass_entry_free(entry);
	} while(0);

	logit_retval();

	return retval;
}

static int cb_print_entry(void *data, int ncolumn, char **column_value, char **column_name)
{
	int show_password = *(int *)data;

	kpass_printf("=====================================================================");
	kpass_printf("Id       : %s", column_value[0]);
	kpass_printf("Tag      : %s", column_value[1]);
	kpass_printf("Name     : %s", column_value[2]);
	kpass_printf("User     : %s", column_value[3]);

	if (show_password == 1) {
		kpass_printf("Password : %s", column_value[4]);
	} else {
		kpass_printf("Password : **********");
	}

	kpass_printf("Url      : %s", column_value[5]);
	kpass_printf("Notes    : %s", column_value[6]);
	kpass_printf("Modified : %s", column_value[7]);

	kpass_printf("=====================================================================");

	return 0;
}

int kpass_list_entry(const int id)
{
	int  retval = -1;

	do {
		return_if_invalid_id(id);
		return_if_entry_not_exists(id);

		retval = kpass_db_exec(cb_print_entry, &kpass.show_password, "SELECT * from KPASS_ENTRIES where ID = %d", id);
	} while(0);

	return retval;
}

int kpass_list_entries(void)
{
	int  retval = -1;

	retval = kpass_db_exec(cb_print_entry, &kpass.show_password, "SELECT * from KPASS_ENTRIES");

	return retval;
}

int kpass_grep_entries(const char *pattern)
{
	int  retval = -1;

	do {
		logit("%s", pattern);

		retval = kpass_db_count("SELECT count(*) from KPASS_ENTRIES where"
		                                              " TAG like '%%%q%%'"
		                                            " or ID like '%%%q%%'"
		                                          " or NAME like '%%%q%%'"
		                                      " or USERNAME like '%%%q%%'"
		                                           " or URL like '%%%q%%'"
		                                         " or NOTES like '%%%q%%'",
		             pattern, pattern, pattern, pattern, pattern, pattern);

		logit("nitems: %d", retval);
		if (retval == RETERR) {
			kpass_error("%s", "kpass_db_count() failed");
			retval = RETERR;
			break;
		}

		if (retval == 0) {
			kpass_printf("No match found.");
			retval = RETERR;
			break;
		}

		retval = kpass_db_exec(cb_print_entry,
		                       &kpass.show_password,
		                       "SELECT * from KPASS_ENTRIES where"
		                                      " TAG like '%%%q%%'"
		                                    " or ID like '%%%q%%'"
		                                  " or NAME like '%%%q%%'"
		                              " or USERNAME like '%%%q%%'"
		                                   " or URL like '%%%q%%'"
		                                 " or NOTES like '%%%q%%'",
		     pattern, pattern, pattern, pattern, pattern, pattern);
	} while(0);

	return retval;
}

/* EOF */
