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

#ifndef _CIPHERS_H
#define _CIPHERS_H
#include "cipher_chaos.h"
#include "cipher_sub.h"
#include "cipher_serpent.h"
#include "cipher_psub.h"


#ifndef CIPHER_COUNT
#define CIPHER_COUNT 5
#endif

#ifdef __DACT_C
int (*(ciphers[CIPHER_COUNT]))()={
	cipher_chaos,
	cipher_sub,
#ifdef HAVE_LIBMCRYPT
	cipher_serpent,
#else
	NULL,
#endif
	cipher_psub,
	NULL
};

char *ciphers_name[CIPHER_COUNT]={ 		/* These must be lower-cased */
	"chaos",
	"subst",
#ifdef HAVE_LIBMCRYPT
	"serpent",
#else
	NULL,
#endif
	"psubst",
	NULL
};

#else
extern int (*(ciphers[CIPHER_COUNT]))();
extern char *ciphers_name[CIPHER_COUNT];
#endif

#endif
