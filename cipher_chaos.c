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

#include "dact.h"
#include <math.h>
#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
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
#include "ui.h"
#include "cipher_chaos.h"


#if defined(USE_MODULES) && defined(AS_MODULE)
#include "module.h"
uint32_t DC_NUM=0;
uint32_t DC_TYPE=DACT_MOD_TYPE_ENC;
void *DC_ALGO=cipher_chaos;
char *DC_NAME="chaos (MOD)";
#endif


int cipher_chaos(const char *inblock, char *outblock, const int blksize, char *key, const int mode) {
	switch (mode) {
		case (DACT_MODE_CINIT+DACT_MODE_CDEC):
		case (DACT_MODE_CINIT+DACT_MODE_CENC):
		case DACT_MODE_CINIT:
			return(cipher_chaos_init(mode,key));
			break;
		case DACT_MODE_CDEC:
			return(cipher_chaos_encdec(inblock, outblock, blksize, key));
			break;
		case DACT_MODE_CENC:
#ifdef DACT_CIPHER_CHAOS
			return(cipher_chaos_encdec(inblock, outblock, blksize, key));
#else
			dact_ui_status(DACT_UI_LVL_GEN, "The chaos cipher is no longer supported.");
			return(-1);
#endif
			break;
	}
	return(0);
}

int cipher_chaos_init(const int mode, char *key) {
	return(cipher_chaos_init_getkey(mode-DACT_MODE_CINIT,key));
}

int cipher_chaos_init_getkey(const int mode, char *key) {
	strcpy(key,dact_ui_getuserinput("File Identification Number: ", 128, 1));
	return(1);
}

int cipher_chaos_encdec(const char *blk, char *outblk, const int blksize, char *key) {
	double fkey;
	int y=0,i;

	fkey=(double) atoi(key);
	for (i=0;i<blksize;i++) {
		outblk[i]=blk[i]^cipher_chaos_getbyte(&fkey,y);
	}
	return(blksize);
}

unsigned char cipher_chaos_getbyte(double *x, int y) {
	static int i=0;

	NORM(*x);
	FIX(*x);
	BEST(*x,y);
	*x=(double) (R(*x)*(1-*x));
	i++;
SPOTVAR_NUM(y);
	return((unsigned char) y);
}
