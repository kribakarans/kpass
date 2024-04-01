#include <libgen.h>
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
	{ 0,             NULL          }
};

static char *kpass_get_modename(const char mode)
{
	int i = 0;

	for (i = 0; kpass_mode[i].mode != 0; i++) {
		if (kpass_mode[i].mode == mode) {
			return kpass_mode[i].name;
		}
	}

	return "unknown";
}

/* Exit if mode already set */
static void exit_if_redundant_mode(const char new_mode)
{
	char ex_mode = kpass.mode;

	if (ex_mode) {
		fprintf(stderr, "Kpass: multiple operations selected (%s & %s)\n",
		                 kpass_get_modename(ex_mode),
		                 kpass_get_modename(new_mode));
		exit(RETERR);
	}

	return;
}

int parse_cmdline(int argc, char **argv)
{
	int option = -1;
	char *prog = NULL;

	if (argv[0])
		prog = basename(argv[0]);

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

			case IMPORT:
				exit_if_redundant_mode(option);
				kpass.mode = option;
				break;

			case EXPORT:
				exit_if_redundant_mode(option);
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
				fprintf(stderr, "%s: option `-%c' requires an argument.\n", prog, optopt);
				fprintf(stderr, "Try '%s --help' for more information.\n", prog);
				exit(RETERR);
				break;

			case '?': default:
				fprintf(stderr, "%s: invalid option -- '%c'\n", prog, optopt);
				fprintf(stderr, "Try '%s --help' for more information.\n", prog);
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
			printf("Need to implement (%s)\n", kpass_get_modename(mode));
			break;

		case EXPORT:
			printf("Need to implement (%s)\n", kpass_get_modename(mode));
			break;

		default:
			fprintf(stderr, "Kpass: invalid mode (%c)\n", mode);
			fprintf(stderr, "Try 'kpass --help' for more information.\n");
			retval = RETERR;
			break;
	};

	return retval;
}

int main(int argc, char **argv)
{
	int retval = -1;

	do {
		retval = parse_cmdline(argc, argv);
		if (retval != RETSXS) {
			fprintf(stderr, "Kpass: failed to parse command line.\n");
			retval = RETERR;
			break;
		}

		retval = kpass_init();
		if (retval != RETSXS) {
			logit("%s", "kpass_init() failed");
			break;
		}

		if (kpass.mode)
			retval = kpass_worker(kpass.mode);
		else
			print_usage();
	} while(0);

	logit_retval();

	return retval;
}

/* EOF */