#ifndef __KPASS_MACROS_H__
#define __KPASS_MACROS_H__

#include "config.h"

#define ASSERT(x) assert(x)

#define __file__    __FILE__
#define __line__    __LINE__
#define __proc__    (getenv("_"))
#define __prog__    (PROGRAM_NAME)
#define __version__ (PROGRAM_VERSION)

#endif
