#ifndef __KPASS_VENDOR_H__
#define __KPASS_VENDOR_H__

#define CHROME_HEADER    "name,url,username,password,note"
#define OPERA_HEADER     "name,url,username,password,note"
#define MICROSOFT_HEADER "name,url,username,password,note"
#define FIREFOX_HEADER_1 "url,username,password,httpRealm,formActionOrigin,guid,timeCreated,timeLastUsed,timePasswordChanged"
#define FIREFOX_HEADER_2 "\"url\",\"username\",\"password\",\"httpRealm\",\"formActionOrigin\",\"guid\",\"timeCreated\",\"timeLastUsed\",\"timePasswordChanged\""

enum e_vendors {
	CHROME    =  1,
	FIREFOX   =  2,
	MICROSOFT =  3,
	OPERA     =  4,
};

char *strvendor(const int id);
int kpass_find_vendor(const char *file);

#endif
