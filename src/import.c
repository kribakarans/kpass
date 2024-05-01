#include "logit.h"
#include "vendor.h"
#include "kpass.h"

#define PASSWD_FIELD_DELIMITER1 ","
#define PASSWD_FIELD_DELIMITER2 ",\""

#define kpass_skip_field_1() (strtok(NULL, PASSWD_FIELD_DELIMITER1))
#define kpass_skip_field_2() (strtok(NULL, PASSWD_FIELD_DELIMITER2))

#define kpass_import_field_1(ent_field, line_ptr) \
	do { \
		char *token = NULL; \
		token = strtok(line_ptr, PASSWD_FIELD_DELIMITER1); \
		if (token != NULL) { \
			strncpy(ent_field, token, sizeof(ent_field)); \
		} \
	} while(0);

#define kpass_import_field_2(ent_field, line_ptr) \
	do { \
		char *token = NULL; \
		token = strtok(line_ptr, PASSWD_FIELD_DELIMITER2); \
		if (token != NULL) { \
			strncpy(ent_field, token, sizeof(ent_field)); \
		} \
	} while(0);


int kpass_import_chrome_passwords(const char *file)
{
	int i = 0;
	int id = -1;
	FILE *fp = NULL;
	int  retval = -1;
	size_t nbytes = 0;
	size_t len = 0;
	char *line = NULL;
	kpass_entry_t *entry = NULL;

	do {
		return_if_null(file);

		fp = fopen(file, "r");
		if (fp == NULL) {
			kpass_error("failed to open '%s': %s", file, strerror(errno));
			break;
		}

		/* Skip header line */
		nbytes = getline(&line, &len, fp);

		while ((nbytes = getline(&line, &len, fp)) != -1) {
			if (line[strlen(line) - 1] == '\n') {
				line[strlen(line) - 1] = '\0';
			}

			//kpass_printf("%s", line);

			entry = kpass_entry_init();
			if (entry == NULL) {
				kpass_error("kpass_entry_init() failed");
				break;
			}

			/* (name,url,username,password,note) */
			kpass_import_field_1(entry->name, line);
			kpass_import_field_1(entry->url, NULL);
			kpass_import_field_1(entry->user, NULL);
			kpass_import_field_1(entry->password, NULL);
			kpass_import_field_1(entry->notes, NULL);

			id = kpass_get_entry_id(entry);
			if (id < 0) {
				/* Insert entry if not exist */
				retval = kpass_insert_entry(entry);
				if (retval != RETSXS) {
					kpass_error("%s", "kpass_insert_entry() failed");
					break;
				}
			} else {
				/* Update entry if already exist */
				retval = kpass_update_entry(id, entry);
				if (retval == RETERR) {
					kpass_error("%s", "kpass_update_entry() failed");
					break;
				}
			}

			kpass_printf("Added new entry: %s", entry->name);
			kpass_entry_free(entry);
			i++;
		}

		fclose(fp);
		free(line);

		kpass_printf("Added %d entries.", i);
	} while(0);

	logit_retval();

	return retval;
}

int kpass_import_firefox_passwords(const char *file)
{
	int i = 0;
	int id = -1;
	FILE *fp = NULL;
	int  retval = -1;
	size_t nbytes = 0;
	size_t len = 0;
	char *line = NULL;
	kpass_entry_t *entry = NULL;


	do {
		return_if_null(file);

		fp = fopen(file, "r");
		if (fp == NULL) {
			kpass_error("failed to open '%s': %s", file, strerror(errno));
			break;
		}

		/* Skip header line */
		nbytes = getline(&line, &len, fp);

		while ((nbytes = getline(&line, &len, fp)) != -1) {
			if (strstr(line, "Firefox Accounts credentials") != NULL) {
				continue;
			}

			if (line[strlen(line) - 1] == '\n') {
				line[strlen(line) - 1] = '\0';
			}

			kpass_printf("%s", line);

			entry = kpass_entry_init();
			if (entry == NULL) {
				kpass_error("kpass_entry_init() failed");
				break;
			}

			/* '"url","username","password","httpRealm","formActionOrigin","guid","timeCreated","timeLastUsed","timePasswordChanged"' */
			kpass_import_field_2(entry->name, line);
			kpass_import_field_2(entry->user, NULL);
			kpass_import_field_2(entry->password, NULL);

			/* For Firefox url and name looks same */
			strncpy(entry->url, entry->name, sizeof(entry->url));

			id = kpass_get_entry_id(entry);
			if (id < 0) {
				/* Insert entry if not exist */
				retval = kpass_insert_entry(entry);
				if (retval != RETSXS) {
					kpass_error("%s", "kpass_insert_entry() failed");
					break;
				}
			} else {
				/* Update entry if already exist */
				retval = kpass_update_entry(id, entry);
				if (retval == RETERR) {
					kpass_error("%s", "kpass_update_entry() failed");
					break;
				}
			}

			kpass_printf("Added new entry: %s", entry->name);
			kpass_entry_free(entry);
			i++;
		}

		fclose(fp);
		free(line);
	} while(0);

	logit_retval();

	return retval;
}

int kpass_import_microsoft_passwords(const char *file)
{
	int retval = -1;

	kpass_printf("Need to implement.");

	return retval;
}

int kpass_import_opera_passwords(const char *file)
{
	int retval = -1;

	kpass_printf("Need to implement.");

	return retval;
}

/**
 * Description:
 * Import passwords from the given file and store it in kpass database.
 */
int kpass_import(const char *file)
{
	int retval = -1;
	int vendor = -1;

	do {
		return_if_null(file);

		if (access(file, F_OK|R_OK|W_OK) != RETSXS) {
			kpass_error("access() failed: %s (%s)", file, strerror(errno));
			retval = RETERR;
			break;
		}

		logit("%s", file);

		vendor = kpass_get_vendor(file);
		logit("vendor=%s", strvendor(vendor));

		switch(vendor) {
			case CHROME:
				retval = kpass_import_chrome_passwords(file);
				break;

			case FIREFOX:
				retval = kpass_import_firefox_passwords(file);
				break;

			case MICROSOFT:
				retval = kpass_import_microsoft_passwords(file);
				break;

			case OPERA:
				retval = kpass_import_opera_passwords(file);
				break;

			default:
				break;

		};
	} while(0);

	return retval;
}

/* EOF */
