#ifndef _LOCAL_UNAME_H
#define _LOCAL_UNAME_H

struct utsname {
	char sysname[64];
	char machine[64];
	char release[64];
};

int uname(struct utsname *buf);

#endif
