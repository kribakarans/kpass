#ifndef __KPASS__UTILS_H__
#define __KPASS__UTILS_H__

#include <stdbool.h>

int  kpass_init(void);
void print_usage(void);
void print_version(void);
bool kpass_user_action(void);
char *strbool(const int val);

#endif
