/*
 * Copyright (C) 2001  Roy Keene
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

#ifndef _COMP_RANGE_H
#define _COMP_RANGE_H
#include "dact.h"
#include "config.h"

#if defined(USE_MODULES) && !defined(AS_MODULE)
#define comp_range_algo DACT_FAILED_ALGO
#else
int comp_range_algo(int mode, unsigned char *prev_block, unsigned char *curr_block, char *out_block, int blk_size);
int comp_range_compress(unsigned char *prev_block, unsigned char *curr_block, char *out_block, int blk_size);
int comp_range_decompress(unsigned char *prev_block, unsigned char *curr_block, char *out_block, int blk_size);
#endif

void incoming_data(unsigned char value);
#endif
