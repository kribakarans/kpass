#include "logit.h"
#include "vendor.h"

int kpass_import_chrome_passwords()
{
	int retval = -1;

	kpass_printf("Need to implement.");

	return retval;
}

int kpass_import_firefox_passwords()
{
	int retval = -1;

	kpass_printf("Need to implement.");

	return retval;
}

int kpass_import_microsoft_passwords()
{
	int retval = -1;

	kpass_printf("Need to implement.");

	return retval;
}

int kpass_import_opera_passwords()
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

		logit("%s", file);

		vendor = kpass_find_vendor(file);
		logit("vendor=%s", strvendor(vendor));

		switch(vendor) {
			case CHROME:
				retval = kpass_import_chrome_passwords();
				break;

			case FIREFOX:
				retval = kpass_import_firefox_passwords();
				break;

			case MICROSOFT:
				retval = kpass_import_microsoft_passwords();
				break;

			case OPERA:
				retval = kpass_import_opera_passwords();
				break;

			default:
				break;

		};
	} while(0);

	return retval;
}

/* EOF */
