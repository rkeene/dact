/*
	Help, my atoi() is broken.
*/

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include "parse.h"

uint32_t atoi2(const char *n) {
	uint32_t retval=0;
	int i,m;
	if (n==NULL) return(0);
	m=strcspn(n,".");
	for (i=0;i<m;i++) retval+=(n[i]-48)*pow(10,m-i-1);
	return(retval);
}

/* Parse those stupid netscape-style URLs, icky. 
 * And I know this REALLY crummy looking, but it works
 * (albeit, probably not perfectly) suffciently well.
 *  -- Roy Keene <rkeene@Rkeene.org>
 * `scheme' 	will never exceed 5 bytes
 * `username'	will never exceed 128 bytes
 * `password'	will never exceed 128 bytes
 * `host'		will never exceed 512 bytes
 * `file'		will never exceed 1024 bytes
 */
int parse_url(const char *url, char *scheme, char *username, char *password, char *host, int *port, char *file) {
	char *local_url=NULL, *local_url_s;
	int i=0;

	if (strstr(url,"p:/")==NULL) {	/* Make sure we have an URL */
		strncpy(file,url,1023);
		return(1);
	};

	local_url_s=local_url=strdup(url);
	file[1]=0;
	*port=0;

	strncpy(scheme,strsep(&local_url,":"),5);
	local_url++;
	if (local_url[0]=='/') local_url++;
	strncpy(host,strsep(&local_url,"/"),512);
	if (local_url!=NULL) strncpy(file+1,local_url,1023);
	file[0]='/';

	password[0]=0;
	if (strchr(host,'@')!=NULL) {
		strcpy(local_url,host);
		strncpy(username,strsep(&local_url,"@:"),128);
		if (strchr(local_url,'@')!=NULL)
			strncpy(password,strsep(&local_url,"@"),128);
		strcpy(host,local_url);
	} else {
		strcpy(username,"");
	}

	if (strchr(host,':')!=NULL) {
		strcpy(local_url,host);
		strcpy(host,strsep(&local_url,":"));
		*port=atoi(local_url);
	} else {
		if (!strcasecmp(scheme,"http")) *port=80;
		if (!strcasecmp(scheme,"ftp")) *port=21;
	}

	free(local_url_s);

	while (scheme[i]) scheme[i++]=tolower(scheme[i]);

	return(0);
}

