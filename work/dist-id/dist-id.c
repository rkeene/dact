/*
 * Identify OS by md5 of /bin/date, /bin/ls, and /bin/ps
 *   Return code:
 *     0 = Unknown
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include "dist-id.h"
#include "md5.h"

int findOS(char *digest_ps) {
	char *md5_ps[][2]=MD5PS;
	int i;

	for (i=0;i<sizeof(md5_ps)/sizeof(char*);i++) {
		if (!strcmp(md5_ps[i][0],digest_ps)) { return(atoi(md5_ps[i][1])); }
	}

	return(0);
}

int main(int argc, char **argv) {
	struct MD5Context md5crap;
	unsigned char digest[16], buf[1024];
	char digeststr[40];
	int fd, i, x, os, dist, arch, vmaj, vmin;

	if ((fd=open("/bin/ps", O_RDONLY))<0) {
		perror("open");
		return(-1);
	}

	MD5Init(&md5crap);
	while ((x=read(fd, buf, sizeof(buf)))>0) {
		MD5Update(&md5crap, buf, x);
	}
	MD5Final(digest, &md5crap);

	digeststr[0]='\0';
	for (i=0;i<16;i++) sprintf(digeststr, "%s%02x", digeststr, digest[i]);

	if (argc>1) {
		dist=arch=0;
		for (i=0;i<(sizeof(distid_os_name)/sizeof(char *));i++) {
			if (!strncasecmp(distid_os_name[i],argv[1],6)) dist=i;
		}
		for (i=0;i<(sizeof(distid_arch)/sizeof(char *));i++) {
			if (!strncasecmp(distid_arch[i],argv[2],6)) arch=i;
		}
		vmaj=atoi(argv[3]);
		vmin=atoi(argv[4]);
		os=(arch<<28)|(vmaj<<22)|(vmin<<14)|dist;
		printf("\t{\"%s\", \"%08i\"}, /* ",digeststr, os);
	} else {
		os=findOS(digeststr);
	}

	if (os>0) {
		dist=os&0x3fff;
		vmaj=(os&0x7e00000)>>22;
		vmin=(os&0x3fc000)>>14;
		arch=(os&0xf0000000)>>28;
		printf("%s %i.%i (%s)", distid_os_name[dist], vmaj, vmin, distid_arch[arch]);
	} else {
		printf("Unknown OS");
	}

	if (argc>1) {
		printf(" */ \\");
	}
	printf("\n");

	return(0);
}
