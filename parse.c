/*
 * Copyright (C) 2001, 2002, and 2003  Roy Keene
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *      email: dact@rkeene.org
 */

/*
	Help, my atoi() is broken.
*/

#include "dact.h"
#ifdef HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <dirent.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include "buffer.h"
#include "parse.h"
#include "crc.h"
#include "ui.h"

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
	char *local_url=NULL, *local_url_s, *fbuf;
	int i;

	if (strstr(url,"://")==NULL) {	/* Make sure we have an URL */
		strncpy(file,url,1023);
		return(1);
	};

	local_url_s=local_url=strdup(url);

	*port=0;
	file[1]=0;

	strncpy(scheme,strsep(&local_url,":"),5);
	local_url+=2;
	strncpy(host,strsep(&local_url,"/"),512);
	if (local_url!=NULL) strncpy(file+1,local_url,1022);
	file[0]='/';

	fbuf=malloc(1024);
	file[0]='/';
	fbuf[0]='\0';
	for (i=0;i<strlen(file);i++) {
		if ((strlen(fbuf)+4)>=1023) break;
		if (file[i]<33 || file[i]>127) {
			if (file[i]==32) { strcat(fbuf, "+"); } else { sprintf(fbuf, "%s%%%02x", fbuf, file[i]); }
		} else {
			sprintf(fbuf, "%s%c", fbuf, file[i]);
		}
	}
	strncpy(file, fbuf, 1023);
	file[1023]=0;
	free(fbuf);


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
		@@HOME@@	Home directory (getenv("HOME"))
		@@OSNM@@	OS Name (linux, freebsd, sunos, etc)
		@@OSVR@@	OS version (2.2.x, 4.2, 5.8, etc)
		@@OSVS@@	OS version (short) (2.2, 4.2, 5.8, etc)
		@@ARCH@@	Arch (i386, sparc64, sun4u, sun4m, etc)
		@@DIST@@	If OSNM=Linux, distribution of Linux.
		@@FILE@@	Name of compressed file.
		@@DTVR@@	Version of DACT (maj.min.rev)
		@@DTVS@@	Version of DACT (short) (maj.min)
		@@DTID@@	DACT identifier (dact-maj.min.rev-(dev|rel)-(no)modules-(no)debian-(no)network-(no)vercheck)
		@@PASS@@	Prompt for Password
		@@USER@@	Prompt for Username
		@@ATSN@@	Put an `@'
*/
char *parse_url_subst(const char *src, const char *fname) {
	static struct utsname system_info;
	static int sysinfo_init=0;
	uint32_t cmd=0, x, strsz;
	const char *loc=src, *ploc=loc, *eloc;
	char *ret, *ret_s, found=0, *smbuf;

	if (!strstr((char *) src,"@@")) return(strdup(src));

	ret_s=ret=calloc(1024,1);
	eloc=src+strlen(src);

	if (!sysinfo_init) {
		uname(&system_info);
		strtolower(system_info.sysname);
		strtolower(system_info.machine);
		sysinfo_init=1;
	}


	ret[0]='\0';
	while (strstr(loc,"@@")) {
		loc = strstr(loc, "@@") + 2;
		cmd=elfcrc(0, (unsigned char *) loc, 4);
		loc+=6;
		if (loc>eloc) { loc=eloc-1; continue; }

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
					if (isdigit((int) system_info.release[x]) || system_info.release[x]=='.') {
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
					if (isdigit((int) system_info.release[x]) || found) {
						ret[0]=system_info.release[x];
						ret++;
					}
				}
				break;
			case 306693: /* FILE-done */
				x=strlen(fname);
				if (x>127) break;
				memcpy(ret,fname,x);
				ret+=x;
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
			case 301490: /* DTVR-done */
				smbuf=malloc(128);
				sprintf(smbuf, "%i.%i.%i", DACT_VER_MAJOR, DACT_VER_MINOR, DACT_VER_REVISION);
				x=strlen(smbuf);
				memcpy(ret, smbuf, x);
				ret+=x;
				free(smbuf);
				break;
			case 301491: /* DTVS-done */
				smbuf=malloc(128);
				sprintf(smbuf, "%i.%i", DACT_VER_MAJOR, DACT_VER_MINOR);
				x=strlen(smbuf);
				memcpy(ret, smbuf, x);
				ret+=x;
				free(smbuf);
				break;
			case 345731: /* PASS-done */
				smbuf=dact_ui_getuserinput("Enter password: ", 128, 1);
				x=strlen(smbuf);
				if (x>127) break;
				memcpy(ret, smbuf, x);
				ret+=x;
				free(smbuf);
				break;
			case 370594: /* USER-done */
				smbuf=dact_ui_getuserinput("Enter username: ", 128, 0);
				x=strlen(smbuf);
				if (x>127) break;
				memcpy(ret, smbuf, x);
				ret+=x;
				free(smbuf);
				break;
			case 289150: /* ATSN-done */
				ret[0]='@';
				ret+=1;
				break;
			case 316437: /* HOME-done*/
				if (!(smbuf=getenv("HOME"))) break;
				x=strlen(smbuf);
				if (x>127) break;
				memcpy(ret, smbuf, x);
				ret+=x;
				break;
			case 301268: /* DTID */
				smbuf=malloc(128);
				sprintf(smbuf, "dact-%i.%i.%i-%s-%smodules-%sdebian-%snetwork-%svercheck", DACT_VER_MAJOR, DACT_VER_MINOR, DACT_VER_REVISION,
#ifdef DEBUG
					"dev",
#else
					"rel",
#endif
#ifdef USE_MODULES
					"",
#else
					"no",
#endif
#ifdef DACT_DEBIAN_UPGRADE_PROC
					"",
#else
					"no",
#endif
#ifndef NO_NETWORK
					"",
#else
					"no",
#endif
#ifdef CHECK_VERSION
					""
#else
					"no"
#endif
					);
				x=strlen(smbuf);
				if (x>127) break;
				memcpy(ret, smbuf, x);
				ret+=x;
				break;
#ifdef DEBUG
			default:
				PRINT_LINE; fprintf(stderr, "Unknown cmd (%i) [src=%s].\n",cmd,src);
				break;
#endif
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

char *mime64(unsigned char *str) {
	int x=strlen((char *) str);
	return(mimes64(str,&x));
}


char *mimes64(unsigned char *str, int *size) {
	char *ret;
	char mimeabet[64]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	int i=0, x=0, m, bit_buf_sze;
	uint32_t bit_buf_sto;

	bit_buf_sze=bit_buffer_size(); /* Save the bit buffer, in case in use */
	bit_buf_sto=bit_buffer_read(bit_buf_sze);

	if ((ret=malloc((int) ((((float)(*size))*1.5)+7)))==NULL) return(NULL);

	while (i<(*size)) {
		while (bit_buffer_size()>=6) ret[x++]=mimeabet[bit_buffer_read(6)];
		if ((bit_buffer_size()+8)<=32) bit_buffer_write(str[i++],8);
	}
	while (bit_buffer_size()>=6) ret[x++]=mimeabet[bit_buffer_read(6)];
	if ((m=bit_buffer_size())) ret[x++]=mimeabet[bit_buffer_read(m)<<(6-m)];
	while (x&3) ret[x++]='=';

	bit_buffer_write(bit_buf_sto, bit_buf_sze); /* Restore it */

	*size=x;

	ret[x]='\0';

	return(ret);
}

char *demime64(unsigned char *str) {
	char *ret;
	char mimeabet[64]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	int i=0, x=0, m, bit_buf_sze;
	uint32_t bit_buf_sto;

	bit_buf_sze=bit_buffer_size(); /* Save the bit buffer, in case in use */
	bit_buf_sto=bit_buffer_read(bit_buf_sze);

	if ((ret=malloc((int) ((((float)strlen((char *) str))*0.75)+6)))==NULL) return(NULL);

	while (i<strlen((char *) str)) {
		if (str[i]=='=') break;
		while (bit_buffer_size()>=8) ret[x++]=bit_buffer_read(8);
		if ((bit_buffer_size()+6)<=32) bit_buffer_write(strchr(mimeabet,str[i++])-mimeabet,6);
	}
	while (bit_buffer_size()>=8) ret[x++]=bit_buffer_read(8);
	if ((m=bit_buffer_size())) ret[x++]=bit_buffer_read(m)<<(8-m);

	bit_buffer_write(bit_buf_sto, bit_buf_sze); /* Restore it */

	return(ret);
}


int32_t read_f(int fd, void *buf, size_t count) {
	int32_t i=0,x;

	while (i!=count) {
		x=read(fd,(void *) (((char *) buf)+i),count-i);
		if (x==0) break;
/* Damn typos, read_f was broken here because x<0 was typed as x>0 */
		if (x<0) return(x);
		i+=x;
	}

	return(i);
}

uint32_t hash_fourbyte(unsigned char *str, const char term) {
	uint32_t ret=0;
	int i;

	for (i=0;i<4;i++) {
		if (!str[i] || str[i]==term) break;
		ret+=(str[i]<<(i*8));
	}
	return(ret);
}
