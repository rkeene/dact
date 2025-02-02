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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 *      email: dact@rkeene.org
 */

#ifndef _CIPHER_PSUB_H
#define _CIPHER_PSUB_H
#include "dact.h"

#if defined(USE_MODULES) && !defined(AS_MODULE)
#define cipher_psub DACT_FAILED_ALGO
#else


int cipher_psub(const unsigned char *inblock, unsigned char *outblock, const int blksize, unsigned char *key, const int mode);
int cipher_psub_init(const int mode, unsigned char *key);
int cipher_psub_init_getkey(const int mode, unsigned char *key);
char *cipher_psub_generatekey(const char *passphrase);
int cipher_psub_encrypt(const unsigned char *inblk, unsigned char *outblk, int blksize, unsigned char *key);
int cipher_psub_decrypt(const unsigned char *inblk, unsigned char *outblk, int blksize, unsigned char *key);
#endif
#endif
