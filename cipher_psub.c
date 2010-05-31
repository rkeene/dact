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
#include "dact.h"
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
#include <math.h>
#include "cipher_psub.h"
#include "parse.h"
#include "ui.h"

#if defined(USE_MODULES) && defined(AS_MODULE)
#include "module.h"
uint32_t DC_NUM=3;
uint32_t DC_TYPE=DACT_MOD_TYPE_ENC;
void *DC_ALGO=cipher_psub;
char *DC_NAME="psubst (MOD)";
#endif


int cipher_psub(const unsigned char *inblock, unsigned char *outblock, const int blksize, unsigned char *key, const int mode) {
	switch (mode) {
		case (DACT_MODE_CINIT+DACT_MODE_CDEC):
		case (DACT_MODE_CINIT+DACT_MODE_CENC):
		case DACT_MODE_CINIT:
			return(cipher_psub_init(mode,key));
			break;
		case DACT_MODE_CDEC:
			return(cipher_psub_decrypt(inblock, outblock, blksize, key));
			break;
		case DACT_MODE_CENC:
			return(cipher_psub_encrypt(inblock, outblock, blksize, key));
			break;
	}
	return(0);
}


int cipher_psub_init(const int mode, unsigned char *key) {
	return(cipher_psub_init_getkey(mode-DACT_MODE_CINIT,key));
}

int cipher_psub_init_getkey(const int mode, unsigned char *key) {
	char *phrase;
	char *keyreg;

	phrase=dact_ui_getuserinput("Passphrase: ",128,1);

	memcpy(key,keyreg=cipher_psub_generatekey(phrase),257);
	free(keyreg);
	return(257);
}

char *cipher_psub_generatekey(const char *passphrase) {
	char *keybuf, used[256], hbuf[4];
	int i,m,x,loc=0,num;
	double d;

	keybuf=malloc(1024);

	for (i=0;i<256;i++) used[i]=0;

	if (strlen(passphrase)<3) { num=257; } else { num=((259/((int) (strlen(passphrase)/3)))+1); }

	for (m=0;m<strlen(passphrase);m+=3) {
		memcpy(hbuf,passphrase+m,3);
		hbuf[3]='\0';
		d=hash_fourbyte((unsigned char *) hbuf, '\0');
		for (i=0;i<num;i++) {
			d=(sin(tan(d))*(255*5));
			x=((abs((int) d)&0x3ff)-255);
			if (x<0 || x>255 || used[x]) { i--; continue; }
			used[x]=1;
			if (loc==0) used[x]=0;
			keybuf[loc++]=x;
			if (loc==257) break;
		}
		if (loc==257) break;
	}
	return(keybuf);
}

int cipher_psub_encrypt(const unsigned char *inblk, unsigned char *outblk, int blksize, unsigned char *key) {
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

int cipher_psub_decrypt(const unsigned char *inblk, unsigned char *outblk, int blksize, unsigned char *key) {
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
