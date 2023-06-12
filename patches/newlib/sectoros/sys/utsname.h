#ifndef __UTSNAME_H__
#define __UTSNAME_H__

#define _UTSNAME_LENGTH 256

struct utsname {
	char  sysname[_UTSNAME_LENGTH];
	char nodename[_UTSNAME_LENGTH];
	char  release[_UTSNAME_LENGTH];
	char  version[_UTSNAME_LENGTH];
	char  machine[_UTSNAME_LENGTH];
	char domainname[_UTSNAME_LENGTH];
};

int uname(struct utsname *);

#endif /*__UTSNAME_H__*/