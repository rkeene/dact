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

#ifndef _CIPHER_SUB_H
#define _CIPHER_SUB_H
#include "dact.h"
#include "comp_fail.h"

#if defined(USE_MODULES) && !defined(AS_MODULE)
#define cipher_sub DACT_FAILED_ALGO
#else


int cipher_sub(const unsigned char *inblock, unsigned char *outblock, const int blksize, unsigned char *key, const int mode);
int cipher_sub_init(const int mode, unsigned char *key);
int cipher_sub_init_getkey(const int mode, unsigned char *key);
int cipher_sub_encrypt(const unsigned char *inblk, unsigned char *outblk, int blksize, unsigned char *key);
int cipher_sub_decrypt(const unsigned char *inblk, unsigned char *outblk, int blksize, unsigned char *key);
char *generatekey(void);

#endif
#endif
