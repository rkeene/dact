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
#include "comp_delta.h"
#include "buffer.h"
#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif



/*
	mode 		- DACT_MODE_COMPR or DACT_MODE_DECMP
			    Determine whether to compress or decompress.
	prev_block	- Previous (uncompressed) block.
	curr_block	- The data to be compressed.
	out_block	- Where to put data after compression.
	blk_size	- Size of prev_block and curr_block.
*/

#if defined(USE_MODULES) && defined(AS_MODULE)
#include "module.h"
uint32_t DC_NUM=2;
uint32_t DC_TYPE=DACT_MOD_TYPE_COMP;
void *DC_ALGO=comp_delta_algo;
char *DC_NAME="Delta Compression (MOD)";
#endif


int comp_delta_algo(int mode, unsigned char *prev_block, unsigned char *curr_block, char *out_block, int blk_size, int bufsize) {
	switch(mode) {
		case DACT_MODE_COMPR:
			return(comp_delta_compress(prev_block, curr_block, out_block, blk_size, bufsize));
			break; /* Heh */
		case DACT_MODE_DECMP:
			return(comp_delta_decompress(prev_block, curr_block, out_block, blk_size, bufsize));
			break;
		default:
			printf("Unsupported mode: %i\n", mode);
			return(-1);
	}
}

int comp_delta_compress(unsigned char *prev_block, unsigned char *curr_block, char *out_block, int blk_size, int bufsize) {
	int i,x=0,y=0;
	char Val;
	unsigned char CurrByte, PrevByte;
	signed char DeltaByte;

	bit_buffer_purge();

	CurrByte=curr_block[0];
	out_block[x]=CurrByte;
	for (i=1;i<blk_size;i++) {
		PrevByte=CurrByte;
		CurrByte=curr_block[i];
		DeltaByte=(CurrByte-PrevByte);
		if (abs(DeltaByte)<32) {
			Val=(((1<<6) | (( ( DeltaByte<0 ))<<5) | (abs(DeltaByte)&33)));
			bit_buffer_write(Val,7);
		} else {
			bit_buffer_write(CurrByte,9);
		}
		y=bit_buffer_size();
		while (y>=8 && y!=16) {
			out_block[++x]=bit_buffer_read(8);
			if (x>=blk_size*2) return(-1);
			y=bit_buffer_size();
		}
		
	}
	y=bit_buffer_size();
	if (y!=0) {
		out_block[++x]=(bit_buffer_read(y)<<(8-y));
	}
	return(x+1);
}

int comp_delta_decompress(unsigned char *prev_block, unsigned char *curr_block, char *out_block, int blk_size, int bufsize) {
	int i=0,x=0;
	unsigned char CurrByte;
	unsigned char DeltaByte;
	char CompBit;
	char Neg;
	char Val;

	CurrByte=curr_block[0];
	bit_buffer_purge();
	while (1) {
		if ((bit_buffer_size()<=8) && i==blk_size) break;
		if ((bit_buffer_size()<=8) && i!=blk_size)
			bit_buffer_write((unsigned char) curr_block[++i],8);
		CompBit=bit_buffer_read(1);
		if ((bit_buffer_size()<=8) && i!=blk_size)
			bit_buffer_write((unsigned char) curr_block[++i],8);

		if (CompBit==1) {
			Neg=bit_buffer_read(1);
			DeltaByte=bit_buffer_read(5);
			Val=CurrByte+(((-2*Neg)+1)*DeltaByte);
		} else {
			Val=bit_buffer_read(8);
		}
		out_block[x++]=CurrByte;
		CurrByte=Val;
	}
	return(x);
}


