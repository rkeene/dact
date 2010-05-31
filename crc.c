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

uint32_t elfcrc(const uint32_t start, const unsigned char *name, const uint32_t n) {
	uint32_t i, h, g=0;

	h = start;
	for (i=0;i<n;i++) {
		h = (h << 4) + (*name++);
		g = h & 0xf0000000;
		if (g) h ^= (g >> ((sizeof(g)*8)-8));
		h &= ~g;
	}

	return(h);
}

/* Adler-32 is composed of two sums accumulated per byte: s1 is the sum
   of all bytes, s2 is the sum of all s1 values. Both sums are done
   modulo 65521. s1 is initialised to 1, s2 to zero. 
*/
uint32_t crc(const uint32_t prev, unsigned char *val, const uint32_t n) {
	uint32_t ret=prev, i;
	uint16_t s1, s2;
	unsigned char *s=val;

	if (ret==0) ret=1;

	s1=ret&0xffff;
	s2=(ret>>16)&0xffff;
	for (i=0; i<n; i++) {
		s1 = (s1 + *s) % 65521;
		s2 = (s1 + s2) % 65521;
		s++;
	}
	ret=(s2<<16)|s1;
	return(ret);
}
