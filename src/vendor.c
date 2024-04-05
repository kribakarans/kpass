#include <stdio.h>

#include "kpass.h"
#include "logit.h"
#include "vendor.h"

typedef struct kpass_vendor_st {
	int     id;
	char *name;
} kpass_vendor_t;

kpass_vendor_t kpass_vendor[] = {
	{ CHROME,    "chrome"    },
	{ FIREFOX,   "firefox"   },
	{ MICROSOFT, "microsoft" },
	{ OPERA,     "opera"     },
	{ 0,         "unknown"   }
};

/* Return vendor name string */
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

int kpass_find_vendor(const char *file)
{
	FILE *fp = NULL;
	int retval = -1;
	char header[BUFF_MAX] = {0};

	do {
		return_if_null(file);
		fp = fopen(file, "r");
		if (fp == NULL) {
			kpass_error("failed to open '%s': %s", file, strerror(errno));
			break;
		}

		if (fgets(header, sizeof(header), fp) == NULL) {
			kpass_error("%s", "fgets() failed: no records found");
			fclose(fp);
			break;
		}

		header[strlen(header) - 1] = '\0';
		logit("'%s'", header);

		if (strcmp(header, CHROME_HEADER) == 0) {
			retval = CHROME;
		} else if ((strcmp(header, FIREFOX_HEADER_2) == 0) ||
		           (strcmp(header, FIREFOX_HEADER_2) == 0)) {
			retval = FIREFOX;
		} else if (strcmp(header, MICROSOFT_HEADER) == 0) {
			retval = MICROSOFT;
		} else if (strcmp(header, OPERA_HEADER) == 0) {
			retval = OPERA;
		} else {
			kpass_error("%s", "unknown password file format");
			retval = RETERR;
		}
	} while(0);

	return retval;
}

/* EOF */
