/*
	Help, my atoi() is broken.
*/

#include <sys/utsname.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include "conf.h"
#include "buffer.h"
#include "parse.h"
#include "crc.h"

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

	if (strstr(url,"://")==NULL) {	/* Make sure we have an URL */
		strncpy(file,url,1023);
		return(1);
	};

	local_url_s=local_url=parse_url_subst(url);

	*port=0;
	file[1]=0;

	strncpy(scheme,strsep(&local_url,":"),5);
	local_url+=2;
	strncpy(host,strsep(&local_url,"/"),512);
	if (local_url!=NULL) strncpy(file+1,local_url,1022);
	file[0]='/';

	password[0]=0;
	if (strchr(host,'@')!=NULL) {
		local_url=local_url_s; /* Doh!  This was missing. */
		strcpy(local_url,host);
		strncpy(username,strsep(&local_url,"@:"),128);
		if (strchr(local_url,'@')!=NULL)
			strncpy(password,strsep(&local_url,"@"),128);
		strcpy(host,local_url);
	} else {
		strcpy(username,"");
	}

	if (strchr(host,':')!=NULL) {
		local_url=local_url_s;
		strcpy(local_url,host);
		strcpy(host,strsep(&local_url,":"));
		*port=atoi(local_url);
	} else {
		if (!strcasecmp(scheme,"http")) *port=80;
		if (!strcasecmp(scheme,"ftp")) *port=21;
	}

	free(local_url_s);
	strtolower(scheme);
	return(0);
}


/*
	Replace 
		@@OSNM@@	OS Name (linux, freebsd, sunos, etc)
		@@OSVR@@	OS version (2.2.x, 4.2, 5.8, etc)
		@@OSVS@@	OS version (short) (2.2, 4.2, 5.8, etc)
		@@ARCH@@	Arch (i386, sparc64, sun4u, sun4m, etc)
		@@DIST@@	If OSNM=Linux, distribution of Linux.
		@@ATSN@@	Put an `@'
*/
char *parse_url_subst(const char *src) {
	struct utsname system_info;
	uint32_t cmd=0, x, strsz;
	const char *loc=src, *ploc=loc;
	char *ret, *ret_s, found=0, *smbuf;

	if (!strstr((char *) src,"@@")) return(strdup(src));

	ret_s=ret=calloc(1024,1);

	uname(&system_info);

	strtolower(system_info.sysname);
	strtolower(system_info.machine);

	while (strstr(loc,"@@")) {
		cmd=ELFCRC(0, loc=strstr(loc, "@@")+2, 4);
		loc+=6;

		strsz=(loc-ploc-8);
		memcpy(ret, ploc, strsz);
		ret+=strsz;

		if ((ret-ret_s)>(1024-128)) break;

		switch (cmd) {
			case 288376: /* ARCH-done */
				x=strlen(system_info.machine);
				if (x>127) break;
				memcpy(ret,system_info.machine,x);
				ret+=x;
				break;
			case 346157: /* OSNM-done */
				x=strlen(system_info.sysname);
				if (x>127) break;
				memcpy(ret,system_info.sysname,x);
				ret+=x;
				break;
			case 346290: /* OSVR-done */
				if (strlen(system_info.release)>127) break;
				for (x=0;x<strlen(system_info.release);x++) {
					if (isdigit(system_info.release[x]) || system_info.release[x]=='.') {
						ret[0]=system_info.release[x];
						ret++;
					} else {
						break;
					}
				}
				break;
			case 346291: /* OSVS-done */
				if (strlen(system_info.release)>127) break;
				for (x=0;x<strlen(system_info.release);x++) {
					if (system_info.release[x]=='.' && found) break;
					if (system_info.release[x]=='.') found=1;
					if (isdigit(system_info.release[x]) || found) {
						ret[0]=system_info.release[x];
						ret++;
					}
				}
				break;
			case 298628: /* DIST-done */
				if (!strcmp("linux",system_info.sysname)) {
					smbuf=parse_url_subst_dist();
					if (smbuf==NULL) break;
					x=strlen(smbuf);
					if (x>127) break;
					memcpy(ret, smbuf, x);
					ret+=x;
				}
				break;
			case 289150: /* ATSN-done */
				ret[0]='@';
				ret+=1;
				break;
			default:
				break;
		}

		ploc=loc;
	}

	memcpy(ret, loc, strlen(loc));

	return(ret_s);
}

char *parse_url_subst_dist(void) {
	DIR *dirfd=NULL;
	struct dirent *info=NULL;
	static char retbuf[128]="unknown";
	char *buf;

/*
	Round 1: check for /etc/DISTRIBUTION-version or /etc/DISTRIBUTION-release
*/
	dirfd=opendir("/etc/.");
	while ((info=readdir(dirfd)) != NULL) {
		if ((buf=strstr(info->d_name,"-version"))!=NULL) {
			buf[0]=0;
			strncpy(retbuf, info->d_name, sizeof(retbuf));
			return(retbuf);
		}
		if ((buf=strstr(info->d_name,"_version"))!=NULL) {
			buf[0]=0;
			strncpy(retbuf, info->d_name, sizeof(retbuf));
			return(retbuf);
		}
		if ((buf=strstr(info->d_name,"-release"))!=NULL) {
			buf[0]=0;
			strncpy(retbuf, info->d_name, sizeof(retbuf));
			return(retbuf);
		}
	}

/*
	Round 2: ??? (distinguish between older versions of Slackware
		and Unknown distributions..)
*/

	return(retbuf);
}


void strtolower(char *str) {
	uint32_t x=0;

	while (str[x]) { str[x]=tolower(str[x]); x++; }
}

char *mime64(char *str) {
	char *ret;
	char mimeabet[64]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	int i=0, x=0, m, bit_buf_sze;
	uint32_t bit_buf_sto;

	bit_buf_sze=bit_buffer_size(); /* Save the bit buffer, in case in use */
	bit_buf_sto=bit_buffer_read(bit_buf_sze);

	if ((ret=malloc((int) ((((float)strlen(str))*1.5)+6)))==NULL) return(NULL);

	while (i<strlen(str)) {
		while (bit_buffer_size()>=6) ret[x++]=mimeabet[bit_buffer_read(6)];
		if ((bit_buffer_size()+8)<=32) bit_buffer_write(str[i++],8);
	}
	while (bit_buffer_size()>=6) ret[x++]=mimeabet[bit_buffer_read(6)];
	if ((m=bit_buffer_size())) ret[x++]=mimeabet[bit_buffer_read(m)<<(6-m)];
	while (x&3) ret[x++]='=';

	bit_buffer_write(bit_buf_sto, bit_buf_sze); /* Restore it */

	return(ret);
}


ssize_t read_net(int fd, void *buf, size_t count) {
	ssize_t i=0,x;

	while (i!=count) {
		x=read(fd,(void *) ((buf)+i),count-i);
		if (x==0) break;
		if (x==-1) return(-1);
		i+=x;
	}

	return(i);
}
