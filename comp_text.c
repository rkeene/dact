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
#include "comp_text.h"
#include "buffer.h"
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
uint32_t DC_NUM=3;
uint32_t DC_TYPE=DACT_MOD_TYPE_COMP;
void *DC_ALGO=comp_text_algo;
char *DC_NAME="Text Compression (MOD)";
#endif

int comp_text_algo(int mode, unsigned char *prev_block, unsigned char *curr_block, char *out_block, int blk_size, int bufsize) {
	switch(mode) {
		case DACT_MODE_COMPR:
			return(comp_text_compress(prev_block, curr_block, out_block, blk_size, bufsize));
			break; /* Heh */
		case DACT_MODE_DECMP:
			return(comp_text_decompress(prev_block, curr_block, out_block, blk_size, bufsize));
			break;
		default:
			printf("Unsupported mode: %i\n", mode);
			return(-1);
	}
}

int comp_text_compress(unsigned char *prev_block, unsigned char *curr_block, char *out_block, int blk_size, int bufsize) {
	unsigned char low_byte=255, high_byte=0;
	unsigned char byte_buf;
	unsigned int range;
	int i,x=0,y;


	bit_buffer_purge();

	for (i=0;i<blk_size;i++) {
		if (((unsigned char) curr_block[i])<low_byte) low_byte=curr_block[i];
		if (((unsigned char) curr_block[i])>high_byte) high_byte=curr_block[i];
	}

	range=(unsigned int) (high_byte-low_byte);

	out_block[x++]=low_byte;
	out_block[x++]=high_byte;

	if (range==0)
		return(2);

	for (y=1;y<=8;y++)
		if ((range>>y)==0) break;
	if (y==8) return(-1);

	for (i=0;i<blk_size;i++) {
		byte_buf=(unsigned char) (curr_block[i]-low_byte);
/*
		if ((byte_buf>>y)==1) {
			return(-1);
		}
*/
		bit_buffer_write(byte_buf,y);

		while (bit_buffer_size()>=8) {
			out_block[x++]=bit_buffer_read(8);
		}
	}
/* 		Remove any less-than-one-byte data */
	if ((i=bit_buffer_size())!=0) {
		out_block[x++]=(bit_buffer_read(i)<<(8-i));
	}

	return(x);
}

int comp_text_decompress(unsigned char *prev_block, unsigned char *curr_block, char *out_block, int blk_size, int bufsize) {
	unsigned char high_byte, low_byte;
	unsigned int range;
	int i=0,x=0,y;
	
	low_byte=curr_block[x++];
	high_byte=curr_block[x++];
	range=(unsigned int) (high_byte-low_byte);

	if (range==0) {
		memset(out_block, low_byte, bufsize);
		return(bufsize);
	}
        for (y=1;y<=8;y++)
                if ((range>>y)==0) break;

	bit_buffer_purge();

	while (1) {
		if (bit_buffer_size()<y) bit_buffer_write((unsigned char) curr_block[x++],8);
		out_block[i++]=(bit_buffer_read(y)+low_byte);
		if ((x==blk_size) && (bit_buffer_size()<y)) break;
		if (i>=(bufsize)) break;
	}

	return(i);
}
