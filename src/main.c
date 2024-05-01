#include <getopt.h>

#include "kpass.h"
#include "logit.h"
#include "utils.h"

kpass_t kpass;

enum cliopts_e {
	ADD_ENTRY    = 'a',
	EDIT_ENTRY   = 'e',
	DELETE_ENTRY = 'r',
	GREP_ENTRY   = 'g',
	LIST_ENTRY   = 'l',
	LIST_ENTRIES = 'L',
	CLEAR_TEXT   = 'c',
	DATABASE     = 'f',
	DEBUG        = 'd',
	IMPORT       = 'i',
	EXPORT       = 'E',
	VERSION      = 'v',
	HELP         = 'h',
};

static char *shortopts = ":acde:E:f:g:hi:l::r:v";

struct option longopts[] = {
	{ "add",       no_argument,        NULL,  ADD_ENTRY     },
	{ "edit",      required_argument,  NULL,  EDIT_ENTRY    },
	{ "delete",    required_argument,  NULL,  DELETE_ENTRY  },
	{ "grep",      required_argument,  NULL,  GREP_ENTRY    },
	{ "list",      optional_argument,  NULL,  LIST_ENTRY    },
	{ "file",      required_argument,  NULL,  DATABASE      },
	{ "debug",     no_argument,        NULL,  DEBUG         },
	{ "import",    required_argument,  NULL,  IMPORT        },
	{ "export",    required_argument,  NULL,  EXPORT        },
	{ "version",   no_argument,        NULL,  VERSION       },
	{ "help",      no_argument,        NULL,  HELP          },
	{  0,          0,                  NULL,  0             }
};

typedef struct kpass_mode_st {
	char  mode;
	char *name;
} kpass_mode_t;

kpass_mode_t kpass_mode[] = {
	{ ADD_ENTRY,    "add-entry"    },
	{ EDIT_ENTRY,   "edit-entry"   },
	{ DELETE_ENTRY, "delete-entry" },
	{ GREP_ENTRY,   "grep-entry"   },
	{ LIST_ENTRY,   "list-entry"   },
	{ LIST_ENTRIES, "list-entries" },
	{ IMPORT,       "import"       },
	{ EXPORT,       "exprt"        },
	{ 0,            "unknown"      }
};

static char *strmode(const char mode)
{
	int i = 0;

	for (i = 0; kpass_mode[i].mode != 0; i++) {
		if (kpass_mode[i].mode == mode) {
			return kpass_mode[i].name;
		}
	}

	return "other";
}

/* Exit if mode already set */
static void exit_if_redundant_mode(const char new_mode)
{
	char ex_mode = kpass.mode;

	if (ex_mode) {
		kpass_printf("Kpass: multiple operations selected (%s & %s)",
		                        strmode(ex_mode), strmode(new_mode));
		exit(RETERR);
	}

	return;
}

int parse_cmdline(int argc, char **argv)
{
	int option = -1;

	while ((option = getopt_long(argc, argv, shortopts, longopts, NULL)) != -1) {
		switch (option) {
			case ADD_ENTRY:
				exit_if_redundant_mode(option);
				kpass.mode = option;
				break;

			case EDIT_ENTRY:
				exit_if_redundant_mode(option);
				kpass.id = atoi(optarg);
				kpass.mode = option;
				break;

			case DELETE_ENTRY:
				exit_if_redundant_mode(option);
				kpass.id = atoi(optarg);
				kpass.mode = option;
				break;

			case GREP_ENTRY:
				exit_if_redundant_mode(option);
				kpass.mode = option;
				kpass.pattern = optarg;
				break;

			case LIST_ENTRY:
				exit_if_redundant_mode(option);
				if (optarg) {
					kpass.id = atoi(optarg);
					kpass.mode = LIST_ENTRY;
				} else {
					kpass.mode = LIST_ENTRIES;
				}
				break;

			case DATABASE:
				exit_if_redundant_mode(option);
				kpass.dbfile = optarg;
				kpass.mode = option;
				break;

			case IMPORT:
				exit_if_redundant_mode(option);
				kpass.passwdfile = optarg;
				kpass.mode = option;
				break;

			case EXPORT:
				exit_if_redundant_mode(option);
				kpass.passwdfile = optarg;
				kpass.mode = option;
				break;

			case DEBUG:
				kpass.debug = 1;
				break;

			case HELP:
				print_usage();
				exit(RETSXS);
				break;

			case VERSION:
				print_version();
				exit(RETSXS);
				break;

			case ':':
				kpass_printf("Kpass: option `-%c' requires an argument.", optopt);
				kpass_printf("Try 'kpass --help' for more information.");
				exit(RETERR);
				break;

			case '?': default:
				kpass_printf("Kpass: invalid option -- '%c'", optopt);
				kpass_printf("Try 'kpass --help' for more information.");
				exit(RETERR);
				break;
		}; /* switch(option) */
	}; /* while(getopt) loop */

	return RETSXS;
}

static int kpass_worker(const char mode)
{
	int retval = -1;

	switch (mode) {
		case ADD_ENTRY:
			retval = kpass_add_entry();
			break;

		case EDIT_ENTRY:
			retval = kpass_edit_entry(kpass.id);
			break;

		case DELETE_ENTRY:
			retval = kpass_delete_entry(kpass.id);
			break;

		case GREP_ENTRY:
			retval = kpass_grep_entries(kpass.pattern);
			break;

		case LIST_ENTRY:
			retval = kpass_list_entry(kpass.id);
			break;

		case LIST_ENTRIES:
			retval = kpass_list_entries();
			break;

		case IMPORT:
			retval = kpass_import(kpass.passwdfile);
			break;

		case EXPORT:
			kpass_printf("Need to implement (%s)", strmode(mode));
			break;

		default:
			kpass_printf("Kpass: invalid mode (%c)", mode);
			kpass_printf("Try 'kpass --help' for more information.");
			retval = RETERR;
			break;
	};

	return retval;
}

void kpass_print_opts(void)
{
	logit("dbcon      : %p", kpass.db);
	logit("mode       : %s", strmode(kpass.mode));
	logit("debug      : %s", strbool(kpass.debug));
	logit("cleartext  : %s", strbool(kpass.cleartext));
	logit("dbfile     : %s", kpass.dbfile);
	logit("passwdfile : %s", kpass.passwdfile);

	return;
}

int main(int argc, char **argv)
{
	int retval = -1;

	do {
		retval = parse_cmdline(argc, argv);
		if (retval != RETSXS) {
			kpass_printf("Kpass: failed to parse command line.");
			retval = RETERR;
			break;
		}

		kpass.debug = true;

		retval = kpass_init();
		if (retval != RETSXS) {
			kpass_error("failed to init kpass");
			break;
		}

		kpass_print_opts();

		if (kpass.mode)
			retval = kpass_worker(kpass.mode);
		else
			print_usage();
	} while(0);

	logit_retval();

	return retval;
}

/* EOF */
