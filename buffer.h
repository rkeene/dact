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


#ifndef _BUFFER_H
#define _BUFFER_H
#include "dact.h"

int bit_buffer_read(int bits);
void bit_buffer_write(unsigned int val, unsigned int bits);
int bit_buffer_size(void);
void bit_buffer_purge(void);

char *byte_buffer_read(int bytes);
char byte_buffer_read_1(void);
void byte_buffer_write(char *val, unsigned int size);
int byte_buffer_size(void);
void byte_buffer_purge(void);

#ifndef BYTE_BUFF_SIZE
#define BYTE_BUFF_SIZE 16384
#endif
#endif
