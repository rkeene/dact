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

#ifndef _CIPHER_CHAOS_H
#define _CIPHER_CHAOS_H
#include "dact.h"
#include "comp_fail.h"

#if defined(USE_MODULES) && !defined(AS_MODULE)
#define cipher_chaos DACT_FAILED_ALGO
#else


#define R(x) 4*(x)
#define NORM(x) { x+=0.000001; while(x<1) x*=10; while(x>=1) x/=10; }
#define BEST(x,y) { y=((int) ((x)*10000))&0xff; }
#define FIX(x) { x=(((double) ((int) ((x)*100000)))/100000); }

int cipher_chaos(const char *inblock, char *outblock, const int blksize, char *key, const int mode);
int cipher_chaos_init(const int mode, char *key);
int cipher_chaos_init_getkey(const int mode, char *key);
int cipher_chaos_encdec(const char *blk, char *outblk, const int blksize, char *key);
unsigned char cipher_chaos_getbyte(double *x, int y);
#endif
#endif
