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

#ifndef _HEADER_H
#define _HEADER_H
#include "dact.h"

#ifndef DACT_HEADER_BLKSIZE
#define DACT_HEADER_BLKSIZE	1024
#endif

int dact_hdr_ext_regs(const int id, const char *val, const uint32_t size);
int dact_hdr_ext_regn(const int id, const uint32_t val, const uint32_t size);
uint32_t dact_hdr_ext_size(void);
char *dact_hdr_ext_data(void);
int dact_hdr_ext_alloc(uint32_t size);
void dact_hdr_ext_clear(void);
#endif
