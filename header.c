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
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <stdio.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#include "header.h"

char *dact_hdr_ext_val=NULL;
uint32_t dact_hdr_ext_sze=0;
uint32_t dact_hdr_ext_pos=0;



int dact_hdr_ext_alloc(uint32_t size) {
	if (dact_hdr_ext_val==NULL) {
		if (!(dact_hdr_ext_val=malloc(DACT_HEADER_BLKSIZE))) {
			dact_hdr_ext_val=NULL;
			return(0);
		}
		dact_hdr_ext_sze=DACT_HEADER_BLKSIZE;
	}
	if ((dact_hdr_ext_pos+size)>(dact_hdr_ext_sze)) {
		dact_hdr_ext_sze=(((dact_hdr_ext_pos+size-1)/DACT_HEADER_BLKSIZE)+1)*DACT_HEADER_BLKSIZE;
		dact_hdr_ext_val=realloc(dact_hdr_ext_val,dact_hdr_ext_sze);
	}
	return(1);
}

int dact_hdr_ext_regs(const int id, const char *val, const uint32_t size) {
	if (!dact_hdr_ext_alloc(size+3)) return(0);
	dact_hdr_ext_val[dact_hdr_ext_pos]=(id&0xff);
	dact_hdr_ext_val[dact_hdr_ext_pos+1]=((size>>8)&0xff);
	dact_hdr_ext_val[dact_hdr_ext_pos+2]=(size&0xff);
	memcpy(dact_hdr_ext_val+dact_hdr_ext_pos+3,val,size);
	dact_hdr_ext_pos+=(size+3);
	return(1);
}

int dact_hdr_ext_regn(const int id, const uint32_t val, const uint32_t size) {
	int i;

	if (!dact_hdr_ext_alloc(size+3)) return(0);
	dact_hdr_ext_val[dact_hdr_ext_pos]=(id&0xff);
	dact_hdr_ext_val[dact_hdr_ext_pos+1]=((size>>8)&0xff);
	dact_hdr_ext_val[dact_hdr_ext_pos+2]=(size&0xff);
	for (i=0;i<size;i++) {
		dact_hdr_ext_val[dact_hdr_ext_pos+3+i]=((val>>((size-i-1)*8)) &0xff);
	}
	dact_hdr_ext_pos+=(size+3);
	return(1);

}

uint32_t dact_hdr_ext_size(void) {
	return(dact_hdr_ext_pos);
}

char *dact_hdr_ext_data(void) {
	memset(dact_hdr_ext_val+dact_hdr_ext_pos, DACT_HDR_NOP, dact_hdr_ext_sze-dact_hdr_ext_pos);
	return(dact_hdr_ext_val);
}

void dact_hdr_ext_clear(void) {
	if (dact_hdr_ext_val!=NULL) free(dact_hdr_ext_val);
	dact_hdr_ext_pos=0;
	dact_hdr_ext_sze=0;
	dact_hdr_ext_val=NULL;
	return;
}


