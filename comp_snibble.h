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

#ifndef _COMP_SNIBBLE_H
#define _COMP_SNIBBLE_H
#include "dact.h"
#include "config.h"

#if defined(USE_MODULES) && !defined(AS_MODULE)
#define comp_snibble_algo DACT_FAILED_ALGO
#else
int comp_snibble_algo(int mode, unsigned char *prev_block, unsigned char *curr_block, char *out_block, int blk_size, int bufsize);
int comp_snibble_compress(unsigned char *prev_block, unsigned char *curr_block, char *out_block, int blk_size, int bufsize);
int comp_snibble_decompress(unsigned char *prev_block, unsigned char *curr_block, char *out_block, int blk_size, int bufsize);
#endif
#endif
