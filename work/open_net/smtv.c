
/*
 *	Network related functions
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <netdb.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include "parse.h"
#include "net.h"


#define REAL_LIBC RTLD_NEXT


int __open(const char *pathname, int flags, ...) {
	static int (*old_open)(const char *, int, mode_t) = NULL;
	uint32_t scheme_val=0;
	va_list args;
	mode_t mode;
	char scheme[5], host[512], file[1024];
	char username[128], password[128];
	char *read_buf, *read_buf_s=NULL;
	int port, fd, x;

	if (!old_open) old_open=(int (*)(const char *, int, mode_t)) dlsym(REAL_LIBC, "__open");

	va_start(args, flags);
	mode = va_arg(args, mode_t);
	va_end(args);

	if (!strcmp(pathname,"/dev/audio")) {
		return(old_open("/etc/showmetv/audio",flags,mode));
	} else {
		return(old_open(pathname,flags,mode));
	}

	return(-1);
}

int open(const char *pathname, int flags, ...) {
	va_list args;
	mode_t mode;

	va_start(args, flags);
	mode = va_arg(args, mode_t);
	va_end(args);

	return(__open(pathname,flags,mode));
}

