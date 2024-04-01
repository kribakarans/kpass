#ifndef __KPASS_LOGIT_H__
#define __KPASS_LOGIT_H__

#include <stdio.h>
#include <unistd.h>

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

#define logit(fmt, ...) \
	do { \
		if (kpass.debug) \
			fprintf(stderr, "\r%d %17s:%-4d %25s() :: " fmt "\n", \
			        getpid(), __FILE__, __LINE__, __func__, __VA_ARGS__); \
	} while(0);

#endif
