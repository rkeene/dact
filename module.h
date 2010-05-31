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

#ifndef _MODULE_H
#define _MODULE_H

#include "dact.h"

int init_modules (void);
int unload_modules (void);
int load_module (char *modulename, const unsigned char *options);
int load_modules_all(const unsigned char *options);

#define DACT_MOD_OK 0
#define DACT_MOD_FAIL (-1)

#define DACT_MOD_TYPE_COMP 0
#define DACT_MOD_TYPE_ENC 1

#define DACT_MOD_REQ_ATLEAST	(0x01000000)
#define DACT_MOD_REQ_EXACTLY	(0x02000000)
#define DACT_MOD_REQ_ATMOST	(0x03000000)

extern char moduledirectory[2048];
extern void *modules[256];
extern int modules_initialized;
extern int modules_count;

#endif/*_MODULE_H*/
