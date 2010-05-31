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
	Encrypt data.
*/
#include "cipher_sub.h"
#include "parse.h"
#include "dact.h"
#include "ui.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <fcntl.h>
#include <stdio.h>
#ifndef RANDOM_DEV
#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif
#endif

#if defined(USE_MODULES) && defined(AS_MODULE)
#include "module.h"
uint32_t DC_NUM=1;
uint32_t DC_TYPE=DACT_MOD_TYPE_ENC;
void *DC_ALGO=cipher_sub;
char *DC_NAME="subst (MOD)";
#endif


int cipher_sub(const unsigned char *inblock, unsigned char *outblock, const int blksize, unsigned char *key, const int mode) {
	switch (mode) {
		case (DACT_MODE_CINIT+DACT_MODE_CDEC):
		case (DACT_MODE_CINIT+DACT_MODE_CENC):
		case DACT_MODE_CINIT:
			return(cipher_sub_init(mode,key));
			break;
		case DACT_MODE_CDEC:
			return(cipher_sub_decrypt(inblock, outblock, blksize, key));
			break;
		case DACT_MODE_CENC:
			return(cipher_sub_encrypt(inblock, outblock, blksize, key));
			break;
	}
	return(0);
}


int cipher_sub_init(const int mode, unsigned char *key) {
	return(cipher_sub_init_getkey(mode-DACT_MODE_CINIT,key));
}

int cipher_sub_init_getkey(const int mode, unsigned char *key) {
	char *fname;
	char keybuf[1024], *ptrbuf;
	int fd,x=257;

	fname=dact_ui_getuserinput("Key file: ",128,0);
	fd=open(fname, O_RDONLY);
	if (fd>=0) {
		x=read(fd, &keybuf, sizeof(keybuf));
		if (x==257) {
			memcpy(key,keybuf,257); /* For backward compatability with DACT 0.8.1*/
		} else {
			memcpy(key,ptrbuf=demime64((unsigned char *) keybuf),257);
			free(ptrbuf);
		}
		close(fd);
		return(257);
	}
	if (fd<0 && mode==DACT_MODE_CENC) {
		fd=open(fname, O_WRONLY|O_CREAT|O_TRUNC, 0600);
		if (fd<0) { PERROR("open"); return(0); }
		memcpy(key,generatekey(),257);
		memcpy(keybuf, ptrbuf=mimes64(key,&x), 400);
		write(fd, keybuf, x);
		write(fd, "\n", 1);
		close(fd);
		free(ptrbuf);
		return(257);
	}
	return(-1);
}

int cipher_sub_encrypt(const unsigned char *inblk, unsigned char *outblk, int blksize, unsigned char *key) {
	int i,mod;
	static int keyoffset=0;

	mod=(int) key[0];
	for (i=0;i<blksize;i++) {
		if (!(i%mod)) {
			keyoffset=((keyoffset+1)&0xff);
		}
		outblk[i]=key[((((int) inblk[i])+keyoffset)&0xff)+1];
	}
	return(blksize);
}

int cipher_sub_decrypt(const unsigned char *inblk, unsigned char *outblk, int blksize, unsigned char *key) {
	int i,mod,x;
	char reversekey[256];
	static int keyoffset=0;

	mod=(int) key[0];
	for (i=1;i<257;i++) reversekey[(int) key[i]]=(i-1);
	for (i=0;i<blksize;i++) {
		if (!(i%mod)) {
			keyoffset=((keyoffset+1)&0xff);
			for (x=0;x<256;x++) reversekey[(int) key[((x+keyoffset)&0xff)+1]]=x;
		}
		outblk[i]=reversekey[(int) inblk[i]];
	}
	return(blksize);
}

char *generatekey(void) {
	static char key[257];
	char used[256];
	int i,x;
#ifdef RANDOM_DEV
	unsigned char buff[1];
	int fd;
	fd=open(RANDOM_DEV, O_RDONLY);
	read(fd, buff, 1);
	if (buff[0]==0) buff[0]=3;
	key[0]=buff[0];
#else
	srand(time(NULL)+rand());
	key[0]=1+(int) (255.0*rand()/(RAND_MAX+1.0));
#endif
	for (i=0;i<256;i++) used[i]=0;
	for (i=1;i<257;i++) {
#ifdef RANDOM_DEV
		read(fd, buff, 1);
		x=buff[0];
#else
		srand(time(NULL)+rand());
		x=(int) (256.0*rand()/(RAND_MAX+1.0));
#endif
		if (used[x]==0) { key[i]=x; used[x]=1; } else { i--; }
	}
#ifdef RANDOM_DEV
	close(fd);
#endif
	return(key);
}
