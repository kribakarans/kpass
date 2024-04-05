#include <stdlib.h>

#include "db.h"
#include "kpass.h"
#include "logit.h"
#include "utils.h"
#include "macros.h"

void kpass_exit(void)
{
	kpass_db_close(kpass.db);

	logit("%s", "bye");

	return;
}

int kpass_init(void)
{
	int retval = -1;
	const char *dbpath = "kpass.db";

	do {
		logit("%s", "hello");

		atexit(&kpass_exit);

		retval = kpass_db_init(dbpath);
		if (retval != RETSXS) {
			kpass_error("db_init() failed");
			break;
		}

		kpass.db = kpass_db_open(dbpath);
		if (kpass.db == NULL) {
			kpass_error("db_open() failed");
			retval = RETERR;
			break;
		}

		retval = RETSXS;
	} while(0);

	return retval;
}

void print_usage(void)
{
	kpass_printf("Kpass: command line password manager");

	kpass_printf("\nkpass [flags] [options] ...\n"
	                "\nOptions:\n"
	                "    -a  --add                 Add new password entry\n"
	                "    -e  --edit    <id>        Edit password entry of @id\n"
	                "    -r  --delete  <id>        Delete password entry of @id\n"
	                "    -g  --grep    <pattern>   Grep @pattern in the database\n"
	                "    -l  --list    <id>        List password entry of @id\n"
	                "    -f  --file    <database>  Use @file as current database\n"
	                "    -d  --debug               Enable debug logging\n"
	                "    -i  --import  <database>  Import entries from the given file\n"
	                "    -E  --export  <csv-file>  Export entries to csv formatted file\n"
	                "    -v  --version             Print package version\n"
	                "    -h  --help                Print this help menun");

	kpass_printf("\nUsage:\n"
	                "    kpass --add               Added new entry\n"
	                "    kpass --edit   1          Edit entry of @id 1\n"
	                "    kpass --delete 2          Delete entry 0f @id 2\n"
	                "    kpass --list              List all password entries\n"
	                "    kpass --list   3          List entry of @id 3\n"
	                "    kpass --grep   hello      Grep the string 'hello' in the database");

	return;
}

void print_version(void)
{
	kpass_printf("%s: Version %s - Password manager.", __prog__, __version__);

	return;
}

bool kpass_user_action(void)
{
	char useropts[16] = {0};

	printf("Are you sure to continue? (Y/N) ");
	fgets(useropts, sizeof(useropts), stdin);
	useropts[strlen(useropts) - 1] = '\0';

	if ((useropts[0] == 'Y') || (useropts[0] == 'y')) {
		return true;
	}

	kpass_printf("Aborted.");

	return false;
}

/* EOF */
