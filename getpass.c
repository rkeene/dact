#include "dact.h"

#ifndef HAVE_GETPASS
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#include "getpass.h"

char *getpass(const char *prompt) {
	static char buf[127];
	FILE *fp=NULL;

	fprintf(stderr, "%s", prompt);
	fp=fopen("/dev/tty", "w");
	if (fp==NULL) fp=stdin;
	fgets(buf, sizeof(buf), fp);
	while (buf[strlen(buf)-1]<' ') buf[strlen(buf)-1]='\0';
	if (fp!=stdin) fclose(fp);
	return(buf);
}
#endif
