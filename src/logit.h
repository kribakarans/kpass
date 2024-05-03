#ifndef __KPASS_LOGIT_H__
#define __KPASS_LOGIT_H__

#include <stdio.h>
#include <unistd.h>

#include "kpass.h"

FILE *logfp;

#define logit_init() \
	do { \
		logfp = fopen(LOGFILE, "w+"); \
		if (logfp == NULL) { \
			fprintf(stderr, "ERROR: failed to init logger: fopen() failed for %s (%s)\n", \
			                                                   LOGFILE, strerror(errno)); \
			abort(); \
		} \
	} while(0);

#define logit_close() \
	do { \
		if (logfp != NULL) \
			fclose(logfp); \
	} while(0);

#define return_if_null(x) \
	do { \
		if (x == NULL) { \
			logit("@%s", #x " is NULL"); \
			return RETERR; \
		} \
	} while(0);

#define logit_retval() \
	do { \
		if (retval != RETSXS) \
			logit("retval: %d", retval); \
	} while(0);

#define logit(...) \
	do { \
		if (kpass.debug) { \
			fprintf(logfp, "DEBUG %d %17s:%-4d %25s() :: ", \
			        getpid(), __FILE__, __LINE__, __func__); \
			fprintf(logfp, __VA_ARGS__); \
			fprintf(logfp, "\n"); \
		} \
	} while(0);

#define dblog(...) \
	do { \
		if (debug_database) { \
			fprintf(logfp, "DEBUG %d %17s:%-4d %25s() :: ", \
			        getpid(), __FILE__, __LINE__, __func__); \
			fprintf(logfp, __VA_ARGS__); \
			fprintf(logfp, "\n"); \
		} \
	} while(0);

#define kpass_error(...) \
	do { \
		fprintf(stderr, "ERROR %d %17s:%-4d %25s() :: ", \
		        getpid(), __FILE__, __LINE__, __func__); \
		fprintf(stderr, __VA_ARGS__); \
		fprintf(stderr, "\n"); \
	} while(0);

#define kpass_printf(...) \
	do { \
		fprintf(stdout, __VA_ARGS__); \
		fprintf(stdout, "\n"); \
	} while(0);

#endif
