#include <stdio.h>
#include <stdbool.h>

#include "kpass.h"
#include "logit.h"
#include "utils.h"
#include "vendor.h"

#define PASSWD_DELIMITER ",\""

typedef struct kpass_vendor_st {
	int     id;
	char *name;
	int nfield;
	char hdr[HDR_FIELD_MAX][NAME_LEN];
} kpass_vendor_t;

kpass_vendor_t kpass_vendor[VENDOR_MAX] = {
	{
		.id   = CHROME,
		.name = "chrome",
		.nfield = CHROME_PASSWD_FIELDS,
		.hdr  = {
			"name",
			"url",
			"username",
			"password",
			"note"
		}
	},
	{
		.id   = FIREFOX,
		.name = "firefox",
		.nfield = FIREFOX_PASSWD_FIELDS,
		.hdr  = {
			"url",
			"username",
			"password",
			"httpRealm",
			"formActionOrigin",
			"guid",
			"timeCreated",
			"timeLastUsed",
			"timePasswordChanged"
		}
	},
	{
		.id   = MICROSOFT,
		.name = "microsoft",
		.nfield = 5,
		.hdr  = {
			"name",
			"url",
			"username",
			"password",
			"note"
		}
	},
	{
		.id   = OPERA,
		.name = "opera",
		.nfield = 5,
		.hdr  = {
			"name",
			"url",
			"username",
			"password",
			"note"
		}
	},
	{ /* End of entries */
		.id   = 0,
		.name = NULL,
	}
};

char *strvendor(const int id)
{
	int i = 0;

	for (i = 0; kpass_vendor[i].id != 0; i++) {
		if (kpass_vendor[i].id == id) {
			return kpass_vendor[i].name;
		}
	}

	return "unknown";
}

static void kpass_print_header(const char hdr[][NAME_LEN], int nfields)
{
	int i = 0;

	for (i = 0; (i < nfields) && (strlen(hdr[i]) > 1); i++) {
		printf("  Col[%d]: '%s'\n", i, hdr[i]);
	}

	return;
}

static void kpass_print_vendor_data(void)
{
	int i = 0;
	int j = 0;

	for (i = 0; kpass_vendor[i].id != 0; i++) {
		printf("    {\n");
		printf("        id=%d\n", kpass_vendor[i].id);
		printf("        name=%s\n", kpass_vendor[i].name);
		printf("        hdr={\n");

		for (j = 0; (j < HDR_FIELD_MAX) && (strlen(kpass_vendor[i].hdr[j]) > 1); j++) {
			printf("            '%s'\n", kpass_vendor[i].hdr[j]);
		}

		printf("        }\n");
		printf("    }\n");
	}

	return;
}

/**
 * Description:
 * Find out the header format by comparing @hdr[][] with @kpass_vendor[x].hdr[][].
 *
 * Retval:
 * Return Vendor id of matched header.
 */
int kpass_lookup_vendor_id(const char hdr[][NAME_LEN], int nfields)
{
	int id = 0;
	int i = 0, j = 0;

	for (i = 0; kpass_vendor[i].id != 0; i++) {
		for (j = 0; (strlen(kpass_vendor[i].hdr[j]) > 1); j++) {
			if (strcmp(hdr[j], kpass_vendor[i].hdr[j]) != 0) {
				break;
			}
		}

		if (j == kpass_vendor[i].nfield) {
			id = i + 1;
			return id;
		}
	}

	return RETERR;
}

/**
 * Description:
 * - Read the column names from the given password file
 * - Construct the 2D array of column names to @hdr[][]
 * - Get the matched vendor id with @kpass_lookup_vendor_id() api
 *
 * Retval:
 * @retval : vendor id of the @file (chrome/firefox)
 */
int kpass_get_vendor(const char *file)
{
	int i = 0;
	FILE *fp = NULL;
	int retval = -1;
	char *token = NULL;
	char line[BUFF_MAX] = {0};
	char *delim = PASSWD_DELIMITER;
	char hdr[HDR_FIELD_MAX][NAME_LEN] = {0};

	do {
		return_if_null(file);
		fp = fopen(file, "r");
		if (fp == NULL) {
			kpass_error("failed to open '%s': %s", file, strerror(errno));
			break;
		}

		if (fgets(line, sizeof(line), fp) == NULL) {
			kpass_error("%s", "fgets() failed: no records found");
			fclose(fp);
			break;
		}

		line[strlen(line) - 1] = '\0';

		logit("'%s'", line);

		token = strtok(line, delim);
		for (i = 0; token != NULL; i++) {
			strncpy(hdr[i], token, sizeof(hdr[i]));
			token = strtok(NULL, delim);
		}

		retval = kpass_lookup_vendor_id(hdr, HDR_FIELD_MAX);
		logit("id=%d", retval);
		fclose(fp);
	} while(0);

	return retval;
}

/* EOF */
