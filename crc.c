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
#include "crc.h"

uint32_t ELFCRC(const uint32_t start, const unsigned char *name, const uint32_t n) {
	uint32_t i,h,g;

	h=start;
	for (i=0;i<n;i++) {
		h = (h << 4) + (*name++);
		if ((g = (h & 0xf0000000)))
			h ^= (g >> 24);
		h &= ~g;
	}

	return(h);
}

