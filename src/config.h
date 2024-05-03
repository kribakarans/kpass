#ifndef __KPASS_CONFIG_H__
#define __KPASS_CONFIG_H__

#define PROGRAM_NAME    "Kpass"
#define PROGRAM_VERSION "1.0"

#define KPASSROOT ".kpass"
#define KPASSDB   "kpass.db"

#ifndef TERMUX
	#define PREFIX "/tmp/"
#else
	#define PREFIX "/data/data/com.termux/files/usr/tmp/"
#endif

#define LOGFILE PREFIX "kpass.log"

#endif
