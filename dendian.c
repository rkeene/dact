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
#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#include "dendian.h"

int write_de(const int dst, const uint32_t num, const int sze) {
	unsigned char buf[4]={0, 0, 0, 0};
	int i,x=0,v;

	for (i=0;i<sze;i++) {
		buf[sze-i-1]=((num&(0xff<<(i*8)))>>(i*8));
	}

	for (i=0;i<sze;i++) {
		if ((v=write(dst, &buf[i], 1))<=0) {
			PERROR("write");
			return(-1);
		}
		x+=v;
	}

	return(x);
}

int read_de(const int src, void *dest, const int sze, const int out_sze) {
	unsigned char ch;
	uint32_t ret=0;
	int i;

	for (i=0;i<sze;i++) {
		if (read(src, &ch, 1)<=0) {
			PERROR("read");
			return(-1);
		}
		ret|=ch<<(8*(sze-i-1));
	}
	memcpy(dest, &ret, out_sze);

	return(sze);
}
