#include "dact.h"
#include "uname.h"
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

int uname(struct utsname *buf) {
#ifdef _USE_WIN32_
	uint32_t winver;
	SYSTEM_INFO winsysinfo;
#endif

	if (buf==NULL) return(-1);

#ifdef _USE_WIN32_
	GetSystemInfo(&winsysinfo);
	winver=GetVersion();
                        
	snprintf(buf->release, 64, "%i.%i", winver&0xff, (winver&0xff00)>>8);
	snprintf(buf->machine, 64, "%lu", (unsigned long) winsysinfo.dwProcessorType); 
	strcpy(buf->sysname, "windows");
#else
	strcpy(buf->sysname, "(unknown)");
	strcpy(buf->machine, "(unknown)");
	strcpy(buf->release, "0.0.0");
#endif
	return(0);
}
